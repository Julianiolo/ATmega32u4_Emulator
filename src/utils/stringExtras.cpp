#include "stringExtras.h"

std::string stringExtras::intToBin(uint16_t val, uint8_t digits) {
	std::string out = "";
	for (uint8_t i = 0; i < digits; i++) {
		out = hexDigits[val & 0b1] + out;
		val >>= 1;
	}
	return out;
}
std::string stringExtras::intToHex(uint16_t val, uint8_t digits) {
	std::string out = "";
	for (uint8_t i = 0; i < digits; i++) {
		out = hexDigits[val & 0xF] + out;
		val >>= 4;
	}
	return out;
}

uint8_t stringExtras::HexDigitToInt(const char str) {
	for (uint8_t ind = 0; ind < 16; ind++) {
		if (str == hexDigits[ind]) {
			return ind;
		}
	}
	return 255;
}

uint32_t stringExtras::HexStrToInt(const char * str) {
	size_t len = std::strlen(str);
	uint32_t out = 0;
	for (size_t i = 0; i < len; i++) {
		out <<= 8;
		out |= HexDigitToInt(*(str + i));
	}
	return out;
}

std::string stringExtras::paddLeft(std::string s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.begin(), paddedLength - out.size(), paddWith);
	return out;
}

std::string stringExtras::paddRight(std::string s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.end(), paddedLength - out.size(), paddWith);
	return out;
}
