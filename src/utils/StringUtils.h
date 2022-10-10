#ifndef __STRING_UTILS_H__
#define __STRING_UTILS_H__

#include <stdint.h>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstring>

namespace StringUtils {
	constexpr char hexDigitsLowerCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	constexpr char hexDigitsUpperCase[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
	extern char texBuf[128];

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
		std::string s(digits, ' ');
		uIntToHexBuf<upperCase>(num, digits, &s[0]);
		return s;
	}
	void uIntToHexBufCase(uint64_t num, uint8_t digits, char* buf, bool upperCase);
	

	void uIntToNumBaseBuf(uint64_t num, uint8_t digits, char* buf, uint8_t base = 10, bool upperCase = false);
	std::string uIntToNumBaseStr(uint64_t num, uint8_t digits, uint8_t base = 10, bool upperCase = false);


	template<typename T = uint64_t>
	T numBaseStrToUInt(uint8_t base, const char* str, const char* strEnd = nullptr){
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
	template<uint8_t base, typename T = uint64_t>
	T numBaseStrToUIntT(const char* str, const char* strEnd = nullptr) {
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		T num = 0;
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
			num += cNum;
		}
		return num;
	}

	template<typename T = uint64_t>
	T binStrToUInt(const char* str, const char* strEnd = nullptr){
		if (strEnd == nullptr)
			strEnd = str + std::strlen(str);

		T out = 0;
		for (const char* cPtr = str; cPtr < strEnd; cPtr++) {
			out <<= 1;
			if (*cPtr == '1')
				out |= 1;
		}
		return out;
	}
	template<typename T = uint64_t>
	T hexStrToUInt(const char* str, const char* strEnd = nullptr){
		return numBaseStrToUIntT<16,T>(str, strEnd);
	}
	template<typename T = uint64_t>
	T hexStrToUIntLen(const char* str, size_t len){
		return hexStrToUInt<T>(str, str + len);
	}

	template<typename T = uint64_t>
	T numStrToUInt(const char* str, const char* strEnd = nullptr){
		// str at least 3 long
		if (str + 2 < strEnd && str[0] == '0') {
			switch (str[1]) {
				case 'b':
					return binStrToUInt<T>(str+2, strEnd);
				case 'x':
					return hexStrToUInt<T>(str+2, strEnd);
				default:
					return numBaseStrToUIntT<8,T>(str+2, strEnd);
			}
		}

		return numBaseStrToUIntT<10,T>(str, strEnd);
	}

	void uIntToBinBuf(uint64_t num, uint8_t digits, char* buf);
	std::string uIntToBinStr(uint64_t num, uint8_t digits);



	std::string paddLeft(const std::string& s, int paddedLength, char paddWith);
	std::string paddRight(const std::string& s, int paddedLength, char paddWith);

	template<typename ... Args>
	std::string format(const char* str, Args ... args) { // https://stackoverflow.com/a/26221725
		int size_i = std::snprintf(NULL, 0, str, args ...) + 1;
		if (size_i <= 0)
			throw std::runtime_error("error during string formatting");

		char* buf = new char[size_i];
		std::snprintf(buf, size_i, str, args ...);
		return std::string(buf, buf+size_i-1);
	}

	std::string loadFileIntoString(const char* path, bool* success = 0);
	bool writeStringToFile(const std::string& str, const char* path);

	std::vector<uint8_t> loadFileIntoByteArray(const char* path, bool* success = 0);
	bool writeBytesToFile(const uint8_t* data, size_t dataLen, const char* path);

	size_t findCharInStr(char c, const char* str, const char* strEnd = nullptr);
	size_t findCharInStrFromBack(char c, const char* str, const char* strEnd = nullptr);
	std::vector<std::pair<size_t,std::string>> findStrings(const uint8_t* data, size_t dataLen, size_t minLen = 1);

	int strcasecmp(const char* a, const char* b, const char* a_end = NULL, const char* b_end = NULL);


	// this function ignores any f/l suffixes, as the size of the float is explicitly stated through the bits
	uint64_t stof(const char* str, const char* str_end, uint8_t exponentBits = 8, uint8_t fractionBits = 23, bool atof_compadible = false);

	uint8_t getLBS(uint64_t x);
	uint8_t getHBS(uint64_t x);

	const char* getFileExtension(const char* path, const char* path_end = NULL);
	const char* getFileName(const char* path, const char* path_end = NULL);

	std::vector<size_t> generateLineIndexArr(const char* str);

	std::string addThousandsSeperator(const char* str, const char* str_end = 0, const char* seperator = ",");
}
#endif

/*

uint8_t HexDigitToInt(char digit);
uint32_t HexStrToInt(const char* str);

*/