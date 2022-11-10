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
			struct FileConsts {
				static constexpr size_t instBytesStart = 10;
				static constexpr size_t instBytesEnd = 21;
			};


			std::string content;
			std::vector<size_t> lines; // [linenumber] = start index of line
			std::vector<addrmcu_t> addrs; // [linenumber] = PC address
			std::map<uint16_t, size_t> labels; // [symbAddress] = linenumber



			struct BranchRoot {
				addrmcu_t start;
				addrmcu_t dest;
				size_t startLine;
				size_t destLine;
				size_t displayDepth;
			};

			std::vector<BranchRoot> branchRoots; // list of all branch roots
			std::vector<size_t> branchRootInds; // [linenumber] = ind to branch root object of this line (-1 if line is not a branchroot)
			std::vector<std::vector<size_t>> passingBranches;  // [linenumber] = branchRootInd of all branches passing this address/line
			uint16_t maxBranchDisplayDepth = 0;
			
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

			struct AdditionalDisasmInfo {
				// mcu Analytics:
				const Analytics* analytics = NULL;

				// source line info (debug_line)
				bool (*getLineInfoFromAddr)(addrmcu_t addr, std::string* out, void* userData) = NULL;
				void* lineUserData = NULL;

				// symbol info (symboltable)
				bool (*getSymbolNameFromAddr)(addrmcu_t addr, bool ramNotRom, std::string* out, void* userData) = NULL;
				void* symbolUserData = NULL;

				inline AdditionalDisasmInfo(){} // needed, bc else clang throws error??
			};
		private:
			std::shared_ptr<DisasmData> disasmData;

			// turn DisasmData into actual String
			void generateContent(const AdditionalDisasmInfo& info = AdditionalDisasmInfo());

			// setup stuff like line indexes etc
			
			static uint16_t generateAddrFromLine(const char* start, const char* end);
			static bool isValidHexAddr(const char* start, const char* end);
			void addAddrToList(const char* start, const char* end, size_t lineInd);

			void processBranches();
			size_t processBranchesRecurse(size_t i, size_t depth = 0);
			void processContent();
		public:
			
			DisasmFile();

			void addDisasmData(size_t size);
			const DisasmData* getDisasmData() const;
			
			void loadSrc(const char* str, const char* strEnd = NULL);
			bool loadSrcFile(const char* path);
			void disassembleBinFile(const Flash* data, const AdditionalDisasmInfo& info = AdditionalDisasmInfo());

			// helpers/utility
			size_t getLineIndFromAddr(uint16_t Addr) const; // if addr not present, returns the index of the pos to insert at
			bool isEmpty() const;
			size_t getNumLines() const;
			bool isSelfDisassembled() const;
		};

		static std::string disassembleRaw(uint16_t word, uint16_t word2);
		static std::string disassemble(uint16_t word, uint16_t word2, uint16_t PC);

	private:
		static std::string getParamStr(uint16_t val, uint8_t type);
		static std::string getSignInt(int32_t val);

		static pc_t getJumpDests(uint16_t word, uint16_t word2, pc_t pc); // get jump dests of inst, return -1 if not a jump/branch

		static void disasmRecurse(pc_t start, const Flash* data, DisasmFile::DisasmData* disasmData);
	};
}

#endif