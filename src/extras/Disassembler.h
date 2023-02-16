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

#include "../components/Flash.h"
#include "Analytics.h"



namespace A32u4 {
	class ATmega32u4;

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
			std::vector<bool> isLineProgram; // [linenumber] = true if line is part of the program, false if not (like data, empty...)
			std::map<uint16_t, size_t> labels; // [symbAddress] = linenumber



			struct BranchRoot {
				addrmcu_t start;
				addrmcu_t dest;
				size_t startLine;
				size_t destLine;
				size_t displayDepth;

				size_t addrDist() const;
			};

			std::vector<BranchRoot> branchRoots; // list of all branch roots
			std::vector<size_t> branchRootInds; // [linenumber] = ind to branch root object of this line (-1 if line is not a branchroot)
			struct PassingBranchs {
				std::vector<size_t> passing;
				size_t startLine;
				static constexpr size_t bitArrSize = 2048;
				BitArray<bitArrSize> occupied;
			};
			std::vector<PassingBranchs> passingBranchesVec;
			std::vector<size_t> passingBranchesInds;  // [linenumber] = ind to pass to passingBranchesVec to get: branchRootInd of all branches passing this address/line
			size_t maxBranchDisplayDepth = 0;
			constexpr static size_t maxBranchShowDist = 256;
			
			struct DisasmData { // data for disasm process
				BitVector disasmed;
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
		private:
			
			static constexpr addrmcu_t Addrs_notAnAddr = -1;
			static constexpr addrmcu_t Addrs_symbolLabel = -2;
		public:
			static bool addrIsActualAddr(addrmcu_t addr);
			static bool addrIsNotProgram(addrmcu_t addr);
			static bool addrIsSymbol(addrmcu_t addr);

			struct AdditionalDisasmInfo {
				// mcu Analytics:
				const Analytics* analytics = NULL;

				// source line info (debug_line)
				std::function<bool(addrmcu_t addr, std::string* out)> getLineInfoFromAddr = NULL;

				// symbol info (symboltable)
				std::function<bool(addrmcu_t addr, bool ramNotRom, std::string* out)> getSymbolNameFromAddr = NULL;

				struct Symbol {
					size_t value;
					size_t size;
					std::string name;
				};
				std::function<Symbol(size_t ind)> dataSymbol;
				size_t numOfDataSymbols = -1;

				std::vector<pc_t> additionalDisasmSeeds;

				inline AdditionalDisasmInfo(){} // needed, bc else clang throws error??
			};
		private:
			std::shared_ptr<DisasmData> disasmData;

			// turn DisasmData into actual String
			void generateContent(const Flash* data,const AdditionalDisasmInfo& info = AdditionalDisasmInfo());

			// setup stuff like line indexes etc
			
			static uint16_t generateAddrFromLine(const char* start, const char* end);
			static bool isValidHexAddr(const char* start, const char* end);
			void addAddrToList(const char* start, const char* end, size_t lineInd);

			void processBranches();
			size_t processBranchesRecurse(size_t i, size_t depth = 0); //const BitArray<256>&
			void processContent();
		public:
			
			DisasmFile();

			void addDisasmData(size_t size);
			const DisasmData* getDisasmData() const;
			
			void loadSrc(const char* str, const char* strEnd = NULL);
			bool loadSrcFile(const char* path);
			void disassembleBinFile(const Flash* data, const AdditionalDisasmInfo& info = AdditionalDisasmInfo());

			// helpers/utility
			size_t getLineIndFromAddr(addrmcu_t Addr) const; // if addr not present, returns the index of the pos to insert at
			bool isEmpty() const;
			size_t getNumLines() const;
			bool isSelfDisassembled() const;

			addrmcu_t getPrevActualAddr(size_t line) const;
			addrmcu_t getNextActualAddr(size_t line) const;
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