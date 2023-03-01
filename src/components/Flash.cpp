#include "Flash.h"

#include <cstring>
#include <string>
#include <fstream>

#include "StringUtils.h"
#include "StreamUtils.h"

#include "../ATmega32u4.h"
#include "InstHandler.h"

#define MCU_MODULE "Flash"

A32u4::Flash::Flash(ATmega32u4* mcu):
#if MCU_USE_HEAP
	data(new uint8_t[sizeMax])
#if MCU_USE_INSTCACHE
	, instCache(new uint8_t[sizeMax / 2])
#endif
#endif
{
	
}

A32u4::Flash::~Flash() {
#if MCU_USE_HEAP
	delete[] data;
	delete[] instCache;
#endif
}

A32u4::Flash::Flash(const Flash& src):
#if MCU_USE_HEAP
	data(new uint8_t[sizeMax])
#if MCU_USE_INSTCACHE
	, instCache(new uint8_t[sizeMax / 2])
#endif
#endif
{
	operator=(src);
}
A32u4::Flash& A32u4::Flash::operator=(const Flash& src){
	std::memcpy(data, src.data, sizeMax);
#if MCU_USE_INSTCACHE
	std::memcpy(instCache, src.instCache, sizeMax/2);
#endif
	size_ = src.size_;
	hasProgram = src.hasProgram;
	return *this;
}

uint8_t A32u4::Flash::getByte(addrmcu_t addr) const {
	A32U4_ASSERT_INRANGE2(addr, 0, sizeMax, return 0, "Flash getByte Address to Big: " MCU_ADDR_FORMAT);

	return data[addr];
}
uint16_t A32u4::Flash::getWord(addrmcu_t addr) const {
	A32U4_ASSERT_INRANGE2(addr, 0, sizeMax, return 0, "Flash getWord Address to Big: " MCU_ADDR_FORMAT);

	return ((uint16_t)data[addr + 1] << 8) | data[addr];
}

// get Inst Word at PC
uint16_t A32u4::Flash::getInst(pc_t pc) const {
	return getWord(pc * 2);
}

uint8_t A32u4::Flash::getInstIndCache(pc_t pc) const {
	A32U4_ASSERT_INRANGE2(pc, 0, sizeMax, return 0xEE, "Flash getInstIndCache Address to Big: " MCU_ADDR_FORMAT);
	return instCache[pc];
}
uint8_t A32u4::Flash::getInstInd(pc_t pc) const{
#if FLASH_USE_INSTIND_CACHE
	return getInstIndCache(pc);
#else
	return InstHandler::getInstInd(getInst(pc));
#endif
}

const uint8_t* A32u4::Flash::getData() {
	return data;
}

void A32u4::Flash::setByte(addrmcu_t addr, uint8_t val){
	A32U4_ASSERT_INRANGE2(addr, 0, sizeMax, return, "Flash setByte Address too Big: " MCU_ADDR_FORMAT);
	data[addr] = val;
	populateInstIndCacheEntry(addr/2);
}
void A32u4::Flash::setInst(pc_t pc, uint16_t val){
	A32U4_ASSERT_INRANGE2(pc, 0, sizeMax/2, return, "Flash setWord pc too Big: " MCU_ADDR_FORMAT);
	data[pc*2] = val&0xFF;
	data[pc*2+1] = (val>>8)&0xFF;
	populateInstIndCacheEntry(pc);
}

sizemcu_t A32u4::Flash::size() const {
	return size_;
}
sizemcu_t A32u4::Flash::sizeWords() const{
	return size_/2;
}


void A32u4::Flash::clear() {
	for (uint16_t i = 0; i < sizeMax; i++) {
		data[i] = 0;
	}
	hasProgram = false;
}

bool A32u4::Flash::loadFromMemory(const uint8_t* data_, size_t dataLen) {
	if (dataLen >= sizeMax) {
		MCU_LOGF_(ATmega32u4::LogLevel_Warning,"%" MCU_PRIuSIZE " bytes is more than fits into the Flash, max is %" MCU_PRIuSIZEMCU " bytes", dataLen, sizeMax);
		// return; // should we return here?
	}

	clear();

	std::memcpy(data, data_, std::min(dataLen,(size_t)sizeMax));
	size_ = (sizemcu_t)std::min(dataLen,(size_t)sizeMax);
	hasProgram = true;

#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
	return true;
}

bool A32u4::Flash::loadFromHexString(const char* str, const char* str_end) {
	clear();

	if (str_end == nullptr){
		str_end = str + std::strlen(str);
	}
	size_t strl = str_end-str;

	// sanity check (check for non ascii characters)
	for (size_t i = 0; i < strl; i++) {
		unsigned char c = (unsigned char)str[i];
		if (c == 0 || c > 127) {
			MCU_LOGF_(ATmega32u4::LogLevel_Warning, "Couldn't load Program from Hex, because it contained a non ASCII character (0x%02x at %" MCU_PRIuSIZE ")", c, i);
			return false;
		}
	}

	size_t str_ind = 0;
	uint16_t flashInd = 0;
	while (str_ind < strl) {
		if (str_ind >= strl) {
			abort();
		}
		str_ind += 1;
		uint8_t ByteCount = StringUtils::hexStrToUIntLen<uint8_t>(str + str_ind, 2);
		str_ind += 2;
		//uint32_t Addr = ((StringUtils::hexStrToUIntLen<uint32_t>(str + str_ind, 2)<<8) | StringUtils::hexStrToUIntLen<uint32_t>(str + str_ind + 2, 2));
		str_ind += 4;
		//uint8_t type = StringUtils::hexStrToUIntLen<uint8_t>(str + str_ind, 2);
		//str_ind += 7;
		for (uint8_t i = 0; i < ByteCount; i++) {
#if MCU_RANGE_CHECK
			if (flashInd >= sizeMax) {
				abort();
			}
#endif
			data[flashInd++] = StringUtils::hexStrToUIntLen<uint8_t>(str + (str_ind+=2), 2);
		}
		str_ind += 4; //skip checksum
		while (str_ind<strl && (str[str_ind] == '\n' || str[str_ind] == '\r')) {
			str_ind++;
		}
	}

	hasProgram = true;
	size_ = flashInd;
#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
	return true;
}
bool A32u4::Flash::loadFromHexFile(const char* path) {
	{
		const char* ext = StringUtils::getFileExtension(path);
		if (std::strcmp(ext, "hex") != 0) {
			MCU_LOGF_(ATmega32u4::LogLevel_Error, "Wrong Extension for loading Flash contents: %s", ext);
			return false;
		}
	}
	
	std::ifstream t;
	size_t len;
	t.open(path, std::ios::binary);
	if (!t.is_open()) {
		t.close();
		MCU_LOGF_(ATmega32u4::LogLevel_Error, "Cannot open file: %s", path);
		return false;
	}
	t.seekg(0, std::ios::end);
	len = (size_t)t.tellg();
	t.seekg(0, std::ios::beg);
	char* buffer = new char[len];
	t.read(buffer, len);
	t.close();
	loadFromHexString(buffer, buffer+len);
	delete[] buffer;
	return true;
}
void A32u4::Flash::populateInstIndCache(){
	for (uint16_t i = 0; i < sizeWords(); i++) {
		populateInstIndCacheEntry(i);
	}
}
void A32u4::Flash::populateInstIndCacheEntry(pc_t pc) {
	uint16_t inst = getInst(pc);
	instCache[pc] = InstHandler::getInstInd(inst);
}

bool A32u4::Flash::isProgramLoaded() const {
	return hasProgram;
}

void A32u4::Flash::getState(std::ostream& output){
	getRomState(output);
}
void A32u4::Flash::setState(std::istream& input){
	setRomState(input);
}

void A32u4::Flash::getRomState(std::ostream& output) {
	StreamUtils::write(output, size_);
	output.write((const char*)data, sizeMax);
}
void A32u4::Flash::setRomState(std::istream& input){
	StreamUtils::read(input, &size_);
	input.read((char*)data, sizeMax);

#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
	hasProgram = true;
}

bool A32u4::Flash::operator==(const Flash& other) const{
	return size_==other.size_ && std::memcmp(data,other.data,sizeMax) == 0 
#if FLASH_USE_INSTIND_CACHE
		&& std::memcmp(instCache,other.instCache,sizeMax/2) == 0
#endif
		;
}
size_t A32u4::Flash::sizeBytes() const {
	size_t sum = 0;

	sum += sizeof(size_);

	sum += sizeMax;
#if MCU_USE_HEAP
	sum += sizeof(data);
#endif

#if FLASH_USE_INSTIND_CACHE
	sum += sizeMax / 2;
#if MCU_USE_HEAP
	sum += sizeof(instCache);
#endif
#endif

	sum += sizeof(hasProgram);

	return sum;
}


/*

uint8_t A32u4::Flash::readhexStrByte(const char* str) {
uint8_t out = 0;
for (uint8_t c = 0; c < 2; c++) {
out |= stringExtras::HexDigitToInt(str[c]) << (1 - c) * 4;
}
return out;
}

*/