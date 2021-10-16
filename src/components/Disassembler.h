#ifndef _A32u4_DISASSEMBLER
#define _A32u4_DISASSEMBLER

#include <string>
#include <vector>
#include <set>
#include <memory>
#include <stdint.h>
#include "../utils/bitArray.h"
#include "Flash.h"
#include "Analytics.h"

namespace A32u4 {
	class Disassembler {
	public:
		class DisasmFile{
		public:
			std::string content;
			
			struct DisasmData {
				BitArray disasmed;
				struct Line{
					uint16_t addr;
					std::string str;
					uint8_t inst_ind;
				};
				std::vector<Line> lines;

				std::set<uint16_t> funcCalls;
				void addFuncCallAddr(uint16_t addr);

				DisasmData(size_t size);
			};
			std::shared_ptr<DisasmData> disasmData;
			
			DisasmFile();

			void addDisasmData(size_t size);
			void generateContent();
			void disassembleBinFile(const Flash* data);
			void disassembleBinFileWithAnalytics(const Flash* data, const Analytics* analytics);
		};

		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(uint32_t val);

		static void disasmRecurse(uint16_t start, const Flash* data, DisasmFile::DisasmData* disasmData);
	};
}
#endif