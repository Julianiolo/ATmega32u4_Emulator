#include "StringUtils.h"

#include <cstring>

#include <fstream>
#include <streambuf>

void StringUtils::uIntToHexBufCase(uint64_t num, uint8_t digits, char* buf, bool upperCase) {
	if(!upperCase)
		StringUtils::uIntToHexBuf<false>(num, digits, buf);
	else
		StringUtils::uIntToHexBuf<true>(num, digits, buf);
}
uint64_t StringUtils::hexStrToUInt(const char* str, const char* strEnd){
	return numBaseStrToUIntT<16>(str, strEnd);
}
uint64_t StringUtils::hexStrToUIntLen(const char* str, size_t len) {
	return hexStrToUInt(str, str + len);
}

void StringUtils::uIntToBinBuf(uint64_t num, uint8_t digits, char* buf) {
	char* bufPtr = buf + digits;
	for (uint8_t i = 0; i < digits; i++) {
		*--bufPtr = num & 1 ? '1' : '0';
		num >>= 1;
	}
}
std::string StringUtils::uIntToBinStr(uint64_t num, uint8_t digits) {
	std::unique_ptr<char[]> buf = std::make_unique<char[]>(digits);
	uIntToBinBuf(num, digits, buf.get());
	return std::string(buf.get(), buf.get() + digits);
}

uint64_t StringUtils::binStrToUInt(const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);

	uint64_t out = 0;
	for (const char* cPtr = str; cPtr < strEnd; cPtr++) {
		out <<= 1;
		if (*cPtr == '1')
			out |= 1;
	}
	return out;
}

uint64_t StringUtils::numBaseStrToUInt(uint8_t base, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);

	uint64_t num = 0;
	const char* strPtr = str;
	while (strPtr != strEnd) {
		const char c = *strPtr++;
		uint8_t cNum = -1;
		if (c >= '0' && c <= '9')
			cNum = c - '0';
		else {
			if (c >= 'A' && c <= 'Z')
				cNum = c - 'A' + 10;
			else if (c >= 'a' && c <= 'z')
				cNum = c - 'a' + 10;
			else
				return -1;
		}
		num *= base;
		num |= cNum;
	}
	return num;
}

uint64_t StringUtils::numStrToUInt(const char* str, const char* strEnd) {
	// str at least 3 long
	if (str + 2 < strEnd && str[0] == '0') {
		switch (str[1]) {
			case 'b':
				return binStrToUInt(str+2, strEnd);
			case 'x':
				return hexStrToUInt(str+2, strEnd);
			default:
				return numBaseStrToUIntT<8>(str+2, strEnd);
		}
	}

	return numBaseStrToUIntT<10>(str, strEnd);
}

std::string StringUtils::paddLeft(std::string s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.begin(), paddedLength - out.size(), paddWith);
	return out;
}
std::string StringUtils::paddRight(std::string s, int paddedLength, char paddWith) {//https://stackoverflow.com/a/667236
	std::string out = s;
	out.insert(out.end(), paddedLength - out.size(), paddWith);
	return out;
}

std::string StringUtils::loadFileIntoString(const char* path, const char* errorMsg) {
	std::ifstream t(path);
	std::string fileStr;

	if(!t.is_open()){
		throw std::runtime_error(std::string("cannot load file: ") + path);
	}

	fileStr = "";

	t.seekg(0, std::ios::end);   
	fileStr.reserve(t.tellg());
	t.seekg(0, std::ios::beg);

	fileStr.assign(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
	return fileStr;
}

bool StringUtils::writeStringToFile(const std::string& str, const char* path) {
	std::ofstream out(path);
	out << str;
	out.close();
	return true;
}

size_t StringUtils::findCharInStr(char c, const char* str, const char* strEnd) {
	if (strEnd == nullptr)
		strEnd = str + std::strlen(str);
	for (const char* ptr = str; ptr < strEnd; ptr++) {
		if (*ptr == c)
			return ptr - str;
	}
	return -1;
}

/*


uint64_t StringUtils::hexToUInt(const char* str, const char* strEnd){
if (strEnd == nullptr)
strEnd = str + std::strlen(str);

uint64_t num = 0;
const char* strPtr = str;
while (strPtr != strEnd) {
const char c = *strPtr++;
uint8_t cNum = -1;
if (c >= '0' && c <= '9')
cNum = c - '0';
else {
if (c >= 'A' && c <= 'F')
cNum = c - 'A' + 10;
else if (c >= 'a' && c <= 'f')
cNum = c - 'a' + 10;
else
return -1;
}
num <<= 4;
num |= cNum;
}
return num;
}


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

uint8_t stringExtras::HexDigitToInt(char digit) {
if(digit >= 'a' && digit <= 'z') {
digit -= ('a'-'A');
}
for (uint8_t ind = 0; ind < 16; ind++) {
if (digit == hexDigits[ind]) {
return ind;
}
}
if(digit == ' ')
return 0;
return 255;
}

uint32_t stringExtras::HexStrToInt(const char * str) {
size_t len = std::strlen(str);
uint32_t out = 0;
for (size_t i = 0; i < len; i++) {
out <<= 4;
out |= HexDigitToInt(*(str + i));
}
return out;
}

*/