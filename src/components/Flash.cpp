#include "Flash.h"

#include <cstring>
#include <string>
#include <fstream>

#include "../utils/StringUtils.h"
#include "../ATmega32u4.h"

A32u4::Flash::Flash(ATmega32u4* mcu):
	mcu(mcu)
#if USE_HEAP
	,data(new uint8_t[size]), instCache(new uint8_t[size / 2])
#endif
{
	
}

A32u4::Flash::~Flash() {
#if USE_HEAP
	delete[] data;
	delete[] instCache;
#endif
}

uint8_t A32u4::Flash::getByte(uint16_t addr) {
	A32U4_ASSERT_INRANGE_M(addr, 0, size, A32U4_ADDR_ERR_STR("Flash getByte Address to Big: ",addr,4), "Flash", return 0);

	return data[addr];
}

uint16_t A32u4::Flash::getWord(uint16_t addr) {
	A32U4_ASSERT_INRANGE_M(addr, 0, size, A32U4_ADDR_ERR_STR("Flash getWord Address to Big: ",addr,4), "Flash", return 0);

	return ((uint16_t)data[addr + 1] << 8) | data[addr];
}

uint16_t A32u4::Flash::getInst(uint16_t addr) {
	return getWord(addr * 2);
}

uint8_t A32u4::Flash::getInstIndCache(uint16_t addr) {
	if (addr >= size / 2) {
		return 0xFF;
	}
	return instCache[addr];
}

void A32u4::Flash::clear() {
	for (uint16_t i = 0; i < size; i++) {
		data[i] = 0;
	}
	hasProgram = false;
}



void A32u4::Flash::loadFromHexString(const char* str) {
	clear();

	size_t strl = std::strlen(str);

	size_t str_ind = 0;
	uint16_t flashInd = 0;
	while (str[str_ind] != NULL && str_ind < strl) {
		if (str_ind >= strl) {
			abort();
		}
		str_ind += 1;
		uint8_t ByteCount = StringUtils::hexStrToUIntLen(str + str_ind, 2);
		str_ind += 2;
		uint32_t Addr = (StringUtils::hexStrToUIntLen(str + str_ind, 2)<<8) | StringUtils::hexStrToUIntLen(str + str_ind + 2, 2);
		str_ind += 4;
		uint8_t type = StringUtils::hexStrToUIntLen(str + str_ind, 2);
		//str_ind += 7;
		for (uint8_t i = 0; i < ByteCount; i++) {
#if RANGE_CHECK
			if (flashInd >= size) {
				abort();
			}
#endif
			data[flashInd++] = StringUtils::hexStrToUIntLen(str + (str_ind+=2), 2);
		}
		str_ind += 4; //skip checksum
		if (str[str_ind] == '\n') {
			str_ind++;
		}
	}

	hasProgram = true;
}

bool A32u4::Flash::loadFromHexFile(const char* str) {
	std::ifstream t;
	size_t len;
	t.open(str);
	if (!t.is_open()) {
		t.close();
		mcu->log(ATmega32u4::LogLevel_Error, (std::string("Cannot open file: ") + str).c_str(), __FILE__, __LINE__, "Flash");
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

uint8_t* A32u4::Flash::getData() {
	return data;
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