#include "Flash.h"

#include <cstring>
#include <string>
#include <fstream>

#include "StringUtils.h"
#include "StreamUtils.h"
#include "DataUtils.h"

#include "../ATmega32u4.h"
#include "InstHandler.h"

#define LU_MODULE "Flash"

A32u4::Flash::Flash(ATmega32u4* mcu): mcu(mcu)
#if MCU_USE_HEAP
	,data(new uint8_t[sizeMax])
#if MCU_USE_INSTCACHE
	, instCache(new uint8_t[sizeMax / 2])
#endif
#endif
{
	
}

A32u4::Flash::~Flash() {
#if MCU_USE_HEAP
	delete[] data;
#if MCU_USE_INSTCACHE
	delete[] instCache;
#endif
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

#if MCU_USE_INSTCACHE
uint8_t A32u4::Flash::getInstIndCache(pc_t pc) const {
	A32U4_ASSERT_INRANGE2(pc, 0, sizeMax, return 0xEE, "Flash getInstIndCache Address to Big: " MCU_ADDR_FORMAT);
	return instCache[pc];
}
#endif
uint8_t A32u4::Flash::getInstInd(pc_t pc) const{
#if MCU_USE_INSTCACHE
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
#if MCU_USE_INSTCACHE
	populateInstIndCacheEntry(addr/2);
#endif
}
void A32u4::Flash::setInst(pc_t pc, uint16_t val){
	A32U4_ASSERT_INRANGE2(pc, 0, sizeMax/2, return, "Flash setWord pc too Big: " MCU_ADDR_FORMAT);
	data[pc*2] = val&0xFF;
	data[pc*2+1] = (val>>8)&0xFF;

#if MCU_USE_INSTCACHE
	populateInstIndCacheEntry(pc);
#endif
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
		LU_LOGF_(LogUtils::LogLevel_Warning,"%" CU_PRIuSIZE " bytes is more than fits into the Flash, max is %" MCU_PRIuSIZEMCU " bytes", dataLen, sizeMax);
		// return; // should we return here?
	}

	clear();

	size_ = (sizemcu_t)std::min(dataLen,(size_t)sizeMax);
	if(size_ > 0)
		std::memcpy(data, data_, size_);
	hasProgram = true;

#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
	return true;
}

bool A32u4::Flash::loadFromHexString(const char* str, const char* str_end) {
	auto res = StringUtils::parseHexFileStr(str, str_end);
	return loadFromMemory(&res[0], res.size());
}
bool A32u4::Flash::loadFromHexFile(const char* path) {
	{
		const char* ext = StringUtils::getFileExtension(path);
		if (std::strcmp(ext, "hex") != 0) {
			LU_LOGF_(LogUtils::LogLevel_Warning, "Unknown extension for loading flash contents via hex file: \"%s\"", ext);
		}
	}

	std::string content = StringUtils::loadFileIntoString(path);
	return loadFromHexString(content.c_str(), content.c_str() + content.size());
}
#if MCU_USE_INSTCACHE
void A32u4::Flash::populateInstIndCache(){
	for (uint16_t i = 0; i < sizeWords(); i++) {
		populateInstIndCacheEntry(i);
	}
}
void A32u4::Flash::populateInstIndCacheEntry(pc_t pc) {
	uint16_t inst = getInst(pc);
	instCache[pc] = InstHandler::getInstInd(inst);
}
#endif

bool A32u4::Flash::isProgramLoaded() const {
	return hasProgram;
}

void A32u4::Flash::getRomState(std::ostream& output) {
	StreamUtils::write(output, size_);
	output.write((const char*)data, sizeMax);
}
void A32u4::Flash::setRomState(std::istream& input){
	StreamUtils::read(input, &size_);
	DU_ASSERTEX(size_ <= sizeMax, StringUtils::format("Flash size read from state is too big: %" CU_PRIuSIZE, (size_t)size_));
	input.read((char*)data, sizeMax);

#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
}

void A32u4::Flash::getState(std::ostream& output){
	getRomState(output);

	StreamUtils::write(output, hasProgram);
#if MCU_WRITE_HASH
	StreamUtils::write(output, hash());
#endif
}
void A32u4::Flash::setState(std::istream& input){
	setRomState(input);

	StreamUtils::read(input, &hasProgram);
	A32U4_CHECK_HASH("Flash");
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
uint32_t A32u4::Flash::hash() const noexcept{
	uint32_t h = 0;
	DU_HASHC(h,size_);
	DU_HASHCB(h, data, sizeMax);
	DU_HASHC(h,hasProgram);
	return h;
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