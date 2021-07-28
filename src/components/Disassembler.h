#ifndef _A32u4_DISASSEMBLER
#define _A32u4_DISASSEMBLER

#include <string>

namespace A32u4 {
	class Disassembler {
	public:
		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(uint32_t val);
	};
}
#endif