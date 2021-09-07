#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <stdexcept>

namespace StringUtils {
	constexpr char hexDigitsLowerCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	constexpr char hexDigitsUpperCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

	template<bool upperCase = false>
	void uIntToHexBuf(uint64_t num, uint8_t digits, char* buf) {
		char* bufPtr = buf + digits;
		while (digits--) {
			*--bufPtr = (upperCase ? hexDigitsUpperCase : hexDigitsLowerCase)[num & 0xF];
			num >>= 4;
		}
	}
	template<bool upperCase = false>
	std::string uIntToHexStr(uint64_t num, uint8_t digits) {
		std::unique_ptr<char[]> buf = std::make_unique<char[]>(digits);
		uIntToHexBuf<upperCase>(num, digits, buf.get());
		return std::string(buf.get(), buf.get() + digits);
	}
	void uIntToHexBufCase(uint64_t num, uint8_t digits, char* buf, bool upperCase);

	uint64_t hexStrToUInt(const char* str, const char* strEnd = nullptr);
	uint64_t hexStrToUIntLen(const char* str, size_t len);

	void uIntToBinBuf(uint64_t num, uint8_t digits, char* buf);
	std::string uIntToBinStr(uint64_t num, uint8_t digits);

	uint64_t binStrToUInt(const char* str, const char* strEnd = nullptr);
	
	
	uint64_t numBaseStrToUInt(uint8_t base, const char* str, const char* strEnd = nullptr);
	template<uint8_t base>
	uint64_t numBaseStrToUIntT(const char* str, const char* strEnd) {
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		uint64_t num = 0;
		const char* strPtr = str;
		while (strPtr != strEnd) {
			const char c = *strPtr++;
			uint8_t cNum = -1;
			if (c >= '0' && c <= '9')
				cNum = c - '0';
			else if (c == ' ')
				cNum = 0;
			else
				if(base > 10) {
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

	uint64_t numStrToUInt(const char* str, const char* strEnd = nullptr);

	std::string paddLeft(std::string s, int paddedLength, char paddWith);
	std::string paddRight(std::string s, int paddedLength, char paddWith);

	template<typename ... Args>
	std::shared_ptr<char[]> format(const char* str, Args ... args) { // https://stackoverflow.com/a/26221725
		int size_i = std::snprintf(NULL, 0, str, args ...) + 1;
		if (size_i <= 0)
			throw std::runtime_error("error during string formatting");

		size_t bufSize = size_i;
		std::shared_ptr<char[]> buf(new char[bufSize]); //std::shared_ptr<char[]> buf = std::make_shared<char[]>(bufSize);
		std::snprintf(buf.get(), bufSize, str, args ...); 
		return buf;
	}

	std::string loadFileIntoString(const char* path, const char* errorMsg = nullptr);
};
#endif

/*

uint8_t HexDigitToInt(char digit);
uint32_t HexStrToInt(const char* str);

*/