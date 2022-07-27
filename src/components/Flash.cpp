#include "Flash.h"

#include <cstring>
#include <string>
#include <fstream>

#include "../utils/StringUtils.h"
#include "../ATmega32u4.h"
#include "InstHandler.h"

A32u4::Flash::Flash(ATmega32u4* mcu):
	mcu(mcu)
#if USE_HEAP
	,data(new uint8_t[sizeMax]), instCache(new uint8_t[sizeMax / 2])
#endif
{
	
}

A32u4::Flash::~Flash() {
#if USE_HEAP
	delete[] data;
	delete[] instCache;
#endif
}

uint8_t A32u4::Flash::getByte(addrmcu_t addr) const {
	A32U4_ASSERT_INRANGE_M(addr, 0, sizeMax, A32U4_ADDR_ERR_STR("Flash getByte Address to Big: ",addr,4), "Flash", return 0);

	return data[addr];
}
uint16_t A32u4::Flash::getWord(addrmcu_t addr) const {
	A32U4_ASSERT_INRANGE_M(addr, 0, sizeMax, A32U4_ADDR_ERR_STR("Flash getWord Address to Big: ",addr,4), "Flash", return 0);

	return ((uint16_t)data[addr + 1] << 8) | data[addr];
}

// get Inst Word at PC
uint16_t A32u4::Flash::getInst(pc_t pc) const {
	return getWord(pc * 2);
}

uint8_t A32u4::Flash::getInstIndCache(pc_t pc) const {
	A32U4_ASSERT_INRANGE_M(pc, 0, sizeMax, A32U4_ADDR_ERR_STR("Flash getInstIndCache Address to Big: ",pc,4), "Flash", return 0xFF);
	return instCache[pc];
}
uint8_t A32u4::Flash::getInstInd(pc_t pc) const{
#if FLASH_USE_INSTIND_CACHE
	return getInstIndCache(pc);
#else
	return InstHandler::getInstInd(getInst(pc));
#endif
}

uint8_t* A32u4::Flash::getData() {
	return data;
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

void A32u4::Flash::loadFromMemory(const uint8_t* data_, size_t dataLen) {
	if (dataLen >= sizeMax) {
		mcu->logf(ATmega32u4::LogLevel_Warning ,"%llu bytes is more than fits into the Flash, max is %llu bytes", dataLen, sizeMax);
		// return; // should we return here?
	}

	clear();

	memcpy(data, data_, dataLen);
	size_ = (sizemcu_t)dataLen;
	hasProgram = true;

#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
}

void A32u4::Flash::loadFromHexString(const char* str) {
	clear();

	size_t strl = std::strlen(str);

	size_t str_ind = 0;
	uint16_t flashInd = 0;
	while ((str[str_ind] != 0) && (str_ind < strl)) {
		if (str_ind >= strl) {
			abort();
		}
		str_ind += 1;
		uint8_t ByteCount = StringUtils::hexStrToUIntLen<uint8_t>(str + str_ind, 2);
		str_ind += 2;
		uint32_t Addr = ((StringUtils::hexStrToUIntLen<uint32_t>(str + str_ind, 2)<<8) | StringUtils::hexStrToUIntLen<uint32_t>(str + str_ind + 2, 2));
		str_ind += 4;
		uint8_t type = StringUtils::hexStrToUIntLen<uint8_t>(str + str_ind, 2);
		//str_ind += 7;
		for (uint8_t i = 0; i < ByteCount; i++) {
#if RANGE_CHECK
			if (flashInd >= sizeMax) {
				abort();
			}
#endif
			data[flashInd++] = StringUtils::hexStrToUIntLen<uint8_t>(str + (str_ind+=2), 2);
		}
		str_ind += 4; //skip checksum
		if (str[str_ind] == '\n') {
			str_ind++;
		}
	}

	hasProgram = true;
	size_ = flashInd;
#if FLASH_USE_INSTIND_CACHE
	populateInstIndCache();
#endif
}
bool A32u4::Flash::loadFromHexFile(const char* path) {
	{
		const char* ext = StringUtils::getFileExtension(path);
		if (std::strcmp(ext, "hex") != 0) {
			mcu->log(ATmega32u4::LogLevel_Error, StringUtils::format("Wrong Extension for loading Flash contents: %s", ext).get(), __FILE__, __LINE__, "Flash");
			return false;
		}
	}
	
		

	std::ifstream t;
	size_t len;
	t.open(path);
	if (!t.is_open()) {
		t.close();
		mcu->log(ATmega32u4::LogLevel_Error, StringUtils::format("Cannot open file: %s", path).get(), __FILE__, __LINE__, "Flash");
		return false;
	}
	t.seekg(0, std::ios::end);
	len = (size_t)t.tellg();
	t.seekg(0, std::ios::beg);
	char* buffer = new char[len];
	t.read(buffer, len);
	t.close();
	loadFromHexString(buffer);
	delete[] buffer;
	return true;
}
void A32u4::Flash::populateInstIndCache(){
	for (uint16_t i = 0; i < sizeWords(); i++) {
		uint16_t inst = getInst(i);
		instCache[i] = InstHandler::getInstInd(inst);
	}
}

bool A32u4::Flash::isProgramLoaded() const {
	return hasProgram;
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