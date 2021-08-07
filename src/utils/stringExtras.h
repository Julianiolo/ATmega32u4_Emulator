#ifndef _STRINGEXTRAS
#define _STRINGEXTRAS

#include <string>

class stringExtras {
private:
	static constexpr const char hexDigits[] = { '0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F' };

public:

	static std::string intToBin(uint16_t val, uint8_t digits);
	static std::string intToHex(uint16_t val, uint8_t digits);

	static uint8_t HexDigitToInt(char digit);
	static uint32_t HexStrToInt(const char* str);

	static std::string paddLeft(std::string s, int paddedLength, char paddWith);
	static std::string paddRight(std::string s, int paddedLength, char paddWith);
};
#endif