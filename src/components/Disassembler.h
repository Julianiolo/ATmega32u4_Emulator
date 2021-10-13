#ifndef _A32u4_DISASSEMBLER
#define _A32u4_DISASSEMBLER

#include <string>
#include <vector>
#include <stdint.h>
#include "../utils/bitArray.h"
#include "Flash.h"

namespace A32u4 {
	class Disassembler {
	public:
		class DisasmFile{
		public:
			std::string content;
			BitArray disasmed;
			
			DisasmFile();
			DisasmFile(size_t size);
		};

		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

		static DisasmFile disassembleBin(const Flash* data);

	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(uint32_t val);

		static void disasmRecurse(uint16_t start, const Flash* data, BitArray* disasmed, std::vector<std::pair<uint16_t,std::string>>* lines);
	};
}
#endif