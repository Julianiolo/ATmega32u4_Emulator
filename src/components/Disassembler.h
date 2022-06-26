#ifndef _A32u4_DISASSEMBLER
#define _A32u4_DISASSEMBLER

#include <string>
#include <vector>
#include <set>
#include <map>
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
			std::vector<size_t> lines; // [linenumber] = start index of line
			std::vector<uint16_t> addrs; // [linenumber] = PC address
			std::map<uint16_t, size_t> labels;

			struct BranchRoot {
				uint8_t displayDepth;
				at_addr_t dest;
			};

			std::vector<std::unique_ptr<BranchRoot>> branchRoots; // [linenumber] = pointer to branch root object of this line (null if line is not a branchroot)
			std::vector<std::vector<at_addr_t>> passingBranches;  // [linenumber] = root addresses of all branches passing this address/line
			
			struct DisasmData { // data for disasm process
				BitArray disasmed;
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
			
			static constexpr uint16_t Addrs_notAnAddr = -1;
			static constexpr uint16_t Addrs_symbolLabel = -2;
		private:
			std::shared_ptr<DisasmData> disasmData;

			// turn DisasmData into actual String
			void generateContent();

			// setup stuff like line indexes etc
			
			static uint16_t generateAddrFromLine(const char* start, const char* end);
			static bool isValidHexAddr(const char* start, const char* end);
			void addAddrToList(const char* start, const char* end, size_t lineInd);
			void processContent();
		public:
			
			DisasmFile();

			void addDisasmData(size_t size);
			const DisasmData* getDisasmData();
			
			bool loadSrcFile(const char* path);
			void disassembleBinFile(const Flash* data);
			void disassembleBinFileWithAnalytics(const Flash* data, const Analytics* analytics);

			// helpers/utility
			size_t getLineIndFromAddr(uint16_t Addr) const;
			bool isEmpty() const;
			size_t getNumLines() const;
			bool isSelfDisassembled() const;
		};

		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(int32_t val);

		static void disasmRecurse(pc_t start, const Flash* data, DisasmFile::DisasmData* disasmData);
	};
}

#endif