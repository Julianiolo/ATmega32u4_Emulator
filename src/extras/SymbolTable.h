#ifndef _ABB_SYMBOLTABLE
#define _ABB_SYMBOLTABLE

#include <string>
#include <vector>
#include <map>

#include "elfReader.h"

namespace A32u4 {
	class ATmega32u4;

	class SymbolTable {
		friend class ATmega32u4;
	public:
		typedef uint64_t symb_size_t;

		struct Symbol {
			struct Flags {
				uint8_t scope = Flags_Scope_None;
				bool isWeak = false;
				bool isConstuctor = false;
				bool isWarning = false;
				uint8_t indirectFlags = Flags_Indirect_Normal;
				uint8_t debugDynamicFlags = Flags_DebDyn_Normal;
				uint8_t funcFileObjectFlags = Flags_FuncFileObj_Normal;
			};
			enum Flags_Scope {
				Flags_Scope_None = 0,
				Flags_Scope_Local = 1<<0,
				Flags_Scope_Global = 1<<1,
				Flags_Scope_Unique = 1<<2
			};
			enum Flags_Indirect {
				Flags_Indirect_Normal = 0,
				Flags_Indirect_RefrenceToSymbol = 1,
				Flags_Indirect_evalWhileReloc = 2
			};
			enum Flags_DebDyn {
				Flags_DebDyn_Normal = 0,
				Flags_DebDyn_DebugSymbol = 1,
				Flags_DebDyn_DynamicSymbol = 2
			};
			enum Flags_FuncFileObj {
				Flags_FuncFileObj_Normal = 0,
				Flags_FuncFileObj_Function = 1,
				Flags_FuncFileObj_File = 2,
				Flags_FuncFileObj_Obj = 3,
				Flags_FuncFileObj_Section = 4,
			};

			struct Section {
				std::string name;
				Section();
				Section(const std::string& name);
			};

			symb_size_t value;
			Flags flags;
			std::string flagStr;
			std::string name;
			std::string demangled;
			std::string note;
			bool hasDemangledName = false;
			symb_size_t size;
			std::string section;

			uint32_t id = -1;

			bool isHidden = false;

			void* extraData = nullptr;

			bool operator<(const Symbol& rhs) const;
			symb_size_t addrEnd() const;
		};

		typedef void (*SymbolsPostProcFuncPtr)(Symbol* symbs, size_t len, void* userData);
		typedef std::vector<uint32_t> SymbolList;
	private:
		ATmega32u4* mcu;

		SymbolsPostProcFuncPtr symbolsPostProcFunc = nullptr;
		void* symbolsPostProcFuncUserData = nullptr;

		std::vector<Symbol> symbolStorage;
		std::map<uint32_t, size_t> symbsIdMap;
		std::map<std::string, Symbol::Section> sections;
		std::map<std::string, uint32_t> symbsNameMap;

		std::map<std::string, std::vector<uint32_t>> symbolsBySections;

		std::vector<uint32_t> symbolsRam;
		std::vector<uint32_t> symbolsRom;

		symb_size_t maxRamAddrEnd = 0;

		bool doesHaveSymbols = false;

		uint32_t genSymbolId();

		Symbol::Flags generateSymbolFlags(const char* str);
		std::string generateSymbolSection(const char* str, const char* strEnd = 0, size_t* sectStrLen = nullptr);
		Symbol parseLine(const char* start, const char* end);
		size_t parseList(std::vector<Symbol>* vec,const char* str, size_t size = -1);

		void setupConnections(size_t cnt); 

		void resetAll();
	public:

		SymbolTable(ATmega32u4* mcu);

		void setSymbolsPostProcFunc(SymbolsPostProcFuncPtr func, void* userData);

		void addSymbol(Symbol&& symbol); // sets id if id==-1

		void generateFlagStrForSymbol(Symbol* symbol);


		bool hasSymbols() const;

		bool loadFromDump(const char* str, const char* str_end=0);
		bool loadFromDumpFile(const char* path);
		bool loadFromELF(const ELF::ELFFile& elf);
		bool loadDeviceSymbolDump(const char* str, const char* str_end=0);
		bool loadDeviceSymbolDumpFile(const char* path);

		const Symbol::Section* getSection(const std::string& name) const;
		const Symbol* getSymbolByName(const std::string& name) const;
		const Symbol* getSymbolByValue(const symb_size_t value, const SymbolList& list) const;
		const Symbol* getSymbolById(uint32_t id) const;

		const std::vector<Symbol>& getSymbols() const;
		const std::map<std::string, Symbol::Section>& getSections() const;

		const Symbol* getSymbol(const SymbolList& symbs, size_t ind) const;
		const SymbolList& getSymbolsRam() const;
		const SymbolList& getSymbolsRom() const;
		const SymbolList& getSymbolsBySection(const std::string& section);

		symb_size_t getMaxRamAddrEnd() const;
	};
}

#endif