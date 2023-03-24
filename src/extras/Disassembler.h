#ifndef _A32u4_DISASSEMBLER
#define _A32u4_DISASSEMBLER

#include <string>
#include <vector>
#include <set>
#include <map>
#include <memory>
#include <stdint.h>
#include <functional>

#include "bitVector.h"
#include "bitArray.h"

#include "../A32u4Types.h"

#include "../components/Flash.h"
#include "Analytics.h"



namespace A32u4 {
	class ATmega32u4;

	class Disassembler {
	public:
		struct AdditionalDisasmInfo {
			// mcu Analytics:
#if MCU_INCLUDE_EXTRAS
			const Analytics* analytics = NULL;
#endif

			const std::vector<std::pair<uint32_t, std::string>>* srcLines = nullptr; // [addr,str]

			const std::vector<std::pair<uint32_t, std::string>>* funcSymbs = nullptr;

			const std::vector<std::tuple<std::string, uint32_t, uint32_t>>* dataSymbs = nullptr; // [name, value, size]

			const std::vector<addrmcu_t>* additionalDisasmSeeds = nullptr;

			inline AdditionalDisasmInfo(){} // needed, bc else clang throws error??
		};
		static std::string disassembleBinFile(const Flash& data, const AdditionalDisasmInfo& info = AdditionalDisasmInfo());


		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

		static pc_t getJumpDests(uint16_t word, uint16_t word2, pc_t pc); // get jump dests of inst, return -1 if not a jump/branch
	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(int32_t val);


		struct DisasmData { // data for disasm process
			std::vector<bool> disasmed;
			struct Line{
				pc_t addr;
				std::string str;
				uint8_t inst_ind;
			};
			std::vector<Line> lines;

			std::set<pc_t> funcCalls;
			void addFuncCallAddr(pc_t addr);

			DisasmData(size_t size);
		};
		static void disasmRecurse(pc_t start, const Flash& data, DisasmData& disasmData);
	};
}



#endif