#include "symbolTable.h"

#include <algorithm>
#include <cstring>
#include <cmath>

#include "StringUtils.h"

#include "../ATmega32u4.h"

A32u4::SymbolTable::Symbol::Section::Section() {

}
A32u4::SymbolTable::Symbol::Section::Section(const std::string& name) : name(name) {

}

bool A32u4::SymbolTable::Symbol::operator<(const Symbol& rhs) const {
	return this->value < rhs.value;
}
uint64_t A32u4::SymbolTable::Symbol::addrEnd() const {
	return value + size;
}

A32u4::SymbolTable::SymbolTable(ATmega32u4* mcu) : mcu(mcu){

}

void A32u4::SymbolTable::setSymbolsPostProcFunc(SymbolsPostProcFuncPtr func, void* userData){
	symbolsPostProcFunc = func;
	symbolsPostProcFuncUserData = userData;
}

A32u4::SymbolTable::Symbol::Flags A32u4::SymbolTable::generateSymbolFlags(const char* str) {
	Symbol::Flags flags;
	switch (str[0]) {
		case ' ':
			flags.scope = Symbol::Flags_Scope_None; 
			break;
		case 'l':
			flags.scope = Symbol::Flags_Scope_Local; 
			break;
		case 'g':
			flags.scope = Symbol::Flags_Scope_Global; 
			break;
		case 'u':
			flags.scope = Symbol::Flags_Scope_Global | Symbol::Flags_Scope_Unique; 
			break;
		case '!':
			flags.scope = Symbol::Flags_Scope_Global | Symbol::Flags_Scope_Local; 
			break;
		default:
			abort();
	}

	switch (str[1]) {
		case ' ':
			flags.isWeak = false;
			break;
		case 'w':
			flags.isWeak = true;
			break;
		default:
			abort();
	}

	switch (str[2]) {
		case ' ':
			flags.isConstuctor = false;
			break;
		case 'C':
			flags.isConstuctor = true;
			break;
		default:
			abort();
	}

	switch (str[3]) {
		case ' ':
			flags.isWarning = false;
			break;
		case 'W':
			flags.isWarning = true;
			break;
		default:
			abort();
	}

	switch (str[4]) {
		case ' ':
			flags.indirectFlags = Symbol::Flags_Indirect_Normal;
			break;
		case 'I':
			flags.indirectFlags = Symbol::Flags_Indirect_RefrenceToSymbol;
			break;
		case 'i':
			flags.indirectFlags = Symbol::Flags_Indirect_evalWhileReloc;
			break;
		default:
			abort();
	}

	switch (str[5]) {
		case ' ':
			flags.debugDynamicFlags = Symbol::Flags_DebDyn_Normal;
			break;
		case 'd':
			flags.debugDynamicFlags = Symbol::Flags_DebDyn_DebugSymbol;
			break;
		case 'D':
			flags.debugDynamicFlags = Symbol::Flags_DebDyn_DynamicSymbol;
			break;
		default:
			abort();
	}

	switch (str[6]) {
		case ' ':
			flags.funcFileObjectFlags = Symbol::Flags_FuncFileObj_Normal;
			break;
		case 'F':
			flags.funcFileObjectFlags = Symbol::Flags_FuncFileObj_Function;
			break;
		case 'f':
			flags.funcFileObjectFlags = Symbol::Flags_FuncFileObj_File;
			break;
		case 'O':
			flags.funcFileObjectFlags = Symbol::Flags_FuncFileObj_Obj;
			break;
		default:
			abort();
	}

	return flags;
}
A32u4::SymbolTable::Symbol::Section* A32u4::SymbolTable::generateSymbolSection(const char* str, const char* strEnd, size_t* sectStrLen) {
	if (!strEnd)
		strEnd = str + std::strlen(str);

	size_t sectLen;
	{
		const char* strPtr = str;
		while (*strPtr != '\t' && strPtr != strEnd)
			strPtr++;
		sectLen = strPtr - str;
		if (sectStrLen)
			*sectStrLen = sectLen;
	}
	
	std::string sectStr = std::string(str, str + sectLen);
	if (sections.find(sectStr) == sections.end()) {
		sections[sectStr] = Symbol::Section(sectStr);
	}
	
	return &sections[sectStr];
}

A32u4::SymbolTable::Symbol A32u4::SymbolTable::parseLine(const char* start, const char* end) {
	Symbol symbol;
	size_t ptr = 0;
	symbol.value = StringUtils::hexStrToUIntLen<uint64_t>(start, 8) & 0xFFFF;
	ptr += 8 + 1;

	symbol.flags = generateSymbolFlags(start + ptr);
	symbol.flagStr = std::string(start + ptr, 7);
	ptr += 7 + 1;

	size_t sectStrLen;
	symbol.section = generateSymbolSection(start + ptr, end, &sectStrLen);
	ptr += sectStrLen + 1;

	symbol.size = StringUtils::hexStrToUIntLen<uint64_t>(start + ptr, 8);
	ptr += 8 + 1;

	symbol.isHidden = false;
	if (*(start + ptr) == '.') {
		constexpr char hiddenStr[] = ".hidden";
		if ((start + ptr + sizeof(hiddenStr) <= end) && (std::string(start + ptr, start + ptr + sizeof(hiddenStr) - 1) == hiddenStr)) {
			symbol.isHidden = true;
			ptr += sizeof(hiddenStr) - 1 + 1;
		}
	}

	size_t tabPos = StringUtils::findCharInStr('\t', start + ptr, end);
	if (tabPos == (size_t)-1) {
		symbol.name = std::string(start + ptr, end);
		symbol.note = "";
	}
	else {
		symbol.name = std::string(start + ptr, start+ptr+tabPos);
		symbol.note = std::string(start + ptr + tabPos+1, end);
		size_t nlPos;
		while ((nlPos = symbol.note.find("\\n")) != std::string::npos)
			symbol.note.replace(nlPos, 2, "\n");
	}
	symbol.hasDemangledName = symbol.name != symbol.demangled;

	return symbol;
}

void A32u4::SymbolTable::parseList(std::vector<Symbol>* vec, const char* str, size_t size) {
	constexpr char startStr[] = "SYMBOL TABLE:";
	const char* startStrOff = std::strstr(str, startStr);

	const size_t strOff = startStrOff != nullptr ? (startStrOff-str) + sizeof(startStr) : 0;

	if (size == (size_t)-1)
		size = std::strlen(str);

	size_t lastLineStart = strOff;
	for (size_t i = strOff; i < size; i++) {
		if (str[i] == '\n') {
			if ((str + i) - (str + lastLineStart) >= (8 + 1 + 7 + 1 + 0 + 1 + 8 + 1))
				vec->push_back(parseLine(str + lastLineStart, str + i));
			lastLineStart = i + 1;
		}
	}
}

void A32u4::SymbolTable::setupConnections() {
	if(symbolsPostProcFunc)
		symbolsPostProcFunc(&symbolStorage[0], symbolStorage.size(), symbolsPostProcFuncUserData);

	std::sort(symbolStorage.begin(), symbolStorage.end());

	{
		const Symbol::Section* bssSection = getSection(".bss");

		const Symbol::Section* textSection = getSection(".text");
		const Symbol::Section* dataSection = getSection(".data");
		for (auto& s : symbolStorage) {
			symbsNameMap[s.name] = &s;

			if (s.section == bssSection || s.section == dataSection)
				symbolsRam.push_back(&s);

			if (s.section == textSection)
				symbolsRom.push_back(&s);
		}

		for(auto& s : symbolsRam){
			if(s->addrEnd() > maxRamAddrEnd)
				maxRamAddrEnd = s->addrEnd();
		}
	}
}


bool A32u4::SymbolTable::loadFromDump(const char* str, const char* str_end) {
	//resetAll();
	if(!str_end)
		str_end = str + std::strlen(str);

	parseList(&symbolStorage,str,str_end-str);

	setupConnections();

	doesHaveSymbols = true;
	return true;
}
bool A32u4::SymbolTable::loadFromDumpFile(const char* path) {
	bool success = false;
	std::string fileStr = StringUtils::loadFileIntoString(path, &success);
	if (!success) { // loading didnt work
		mcu->logf(ATmega32u4::LogLevel_Error, "Cannot Open symbol table dump File: %s", path);
		return false;
	}

	return loadFromDump(fileStr.c_str(), fileStr.c_str() + fileStr.size());
}

bool A32u4::SymbolTable::loadFromELF(const ELF::ELFFile& elf) {
	//resetAll();

	for (size_t i = 0; i < elf.symbolTableEntrys.size(); i++) {
		auto& symb = elf.symbolTableEntrys[i];
		Symbol symbol;
		symbol.name = elf.stringTableStr + symb.name;
		symbol.value = symb.value & 0xffff; // theres a 8 in the high bits that we need to mask out, idk why its there
		symbol.size = symb.size;
		if (symb.shndx != ELF::ELFFile::SymbolTableEntry::SpecialSectionInd_SHN_UNDEF && symb.shndx < ELF::ELFFile::SymbolTableEntry::SpecialSectionInd_SHN_LORESERVE) {
			symbol.section = generateSymbolSection(elf.shstringTableStr + elf.sectionHeaders[symb.shndx].name);
		}
		else {
			const char* str = "";
			switch (symb.shndx) {
				case ELF::ELFFile::SymbolTableEntry::SpecialSectionInd_SHN_UNDEF:
					str = "UNDEF";
					break;
				case ELF::ELFFile::SymbolTableEntry::SpecialSectionInd_SHN_ABS:
					str = "ABS";
					break;
				case ELF::ELFFile::SymbolTableEntry::SpecialSectionInd_SHN_COMMON:
					str = "COMMON";
					break;
			}
			symbol.section = generateSymbolSection(str);
		}
		

		{
			auto bind = symb.getInfoBinding();
			//                                       LOCAL                      GLOBAL                      WEAK
			constexpr uint8_t infoToBindFlagLUT[] = {Symbol::Flags_Scope_Local, Symbol::Flags_Scope_Global, Symbol::Flags_Scope_Global};
			symbol.flags.scope = bind <= 2 ? infoToBindFlagLUT[bind] : Symbol::Flags_Scope_None;

			symbol.flags.isWeak = bind == ELF::ELFFile::SymbolTableEntry::SymbolInfoBinding_Weak;
		}
		{
			auto type = symb.getInfoType();
			constexpr uint8_t infoToTypeFlagLUT[] = {Symbol::Flags_FuncFileObj_Normal, Symbol::Flags_FuncFileObj_Obj, Symbol::Flags_FuncFileObj_Function, Symbol::Flags_FuncFileObj_File, Symbol::Flags_FuncFileObj_Section};
			symbol.flags.funcFileObjectFlags = type <= 4 ? infoToTypeFlagLUT[type] : Symbol::Flags_FuncFileObj_Normal;
		}

		symbol.flags.debugDynamicFlags = Symbol::Flags_DebDyn_Normal;
		symbol.flags.indirectFlags = Symbol::Flags_Indirect_Normal;

		symbol.flags.isConstuctor = false;
		symbol.flags.isWarning = false;
		
		symbol.isHidden = false; // idk how to read that???
		symbol.flagStr = "";

		symbolStorage.push_back(symbol);
	}

	setupConnections();

	doesHaveSymbols = true;

	return true;
}

bool A32u4::SymbolTable::loadDeviceSymbolDump(const char* str, const char* str_end) {
	deviceSpecSymbolStorage.clear();
	parseList(&deviceSpecSymbolStorage,str,str_end-str);
	
	for (size_t i = 0; i < deviceSpecSymbolStorage.size(); i++) {
		symbolsRam.push_back(&deviceSpecSymbolStorage[i]);
	}

	for(auto& s : symbolsRam){
		if(s->addrEnd() > maxRamAddrEnd)
			maxRamAddrEnd = s->addrEnd();
	}

	if(symbolsPostProcFunc)
		symbolsPostProcFunc(&symbolStorage[0], symbolStorage.size(), symbolsPostProcFuncUserData);

	return true;
}
bool A32u4::SymbolTable::loadDeviceSymbolDumpFile(const char* path) {
	bool success = true;
	std::string fileStr = StringUtils::loadFileIntoString(path, &success); // (std::string("Cannot Open device symbol table dump File: ") + path).c_str()
	if (!success) {// loading didnt work
		mcu->logf(A32u4::ATmega32u4::LogLevel_Warning, "Cannot Open device symbol table dump file: %s", path);
		return false;
	}

	loadDeviceSymbolDump(fileStr.c_str(), fileStr.c_str()+fileStr.size());

	return true;
}

void A32u4::SymbolTable::resetAll() {
	deviceSpecSymbolStorage.clear();

	symbolStorage.clear();
	symbsNameMap.clear();
	sections.clear();

	symbolsRam.clear();
	symbolsRamExp.clear();
	symbolsRom.clear();

	maxRamAddrEnd = 0;

	doesHaveSymbols = false;
}


bool A32u4::SymbolTable::hasSymbols() const {
	return doesHaveSymbols;
}

const A32u4::SymbolTable::Symbol::Section* A32u4::SymbolTable::getSection(const std::string& name) const {
	return &sections.at(name);
}

const A32u4::SymbolTable::Symbol* A32u4::SymbolTable::getSymbolByName(const std::string& name) const {
	if (symbsNameMap.find(name) == symbsNameMap.end())
		return nullptr;

	return symbsNameMap.at(name);
}

const A32u4::SymbolTable::Symbol* A32u4::SymbolTable::getSymbolByValue(const symb_size_t value, SymbolListPtr list) {
	if (list->size() == 0)
		return nullptr;

	size_t from = 0;
	size_t to = list->size() - 1;
	while (from != to) {
		size_t mid = from + (to - from) / 2;
		symb_size_t val = (*list)[mid]->value;
		if (val == value) {
			return (*list)[mid];
		}
		else {
			if (val > value) {
				if (to == mid)
					break;
				to = mid;
			}
			else {
				if (from == mid)
					break;
				from = mid;
			}
				
		}
	}
	const Symbol* s = (*list)[from];
	if (value >= s->value && value <= s->value + s->size)
		return s;
	return nullptr;
}

const std::vector<A32u4::SymbolTable::Symbol>& A32u4::SymbolTable::getSymbols() const {
	return symbolStorage;
}

A32u4::SymbolTable::SymbolListPtr A32u4::SymbolTable::getSymbolsRam() const {
	return (SymbolTable::SymbolListPtr)&symbolsRam;
}
A32u4::SymbolTable::SymbolListPtr A32u4::SymbolTable::getSymbolsRamExp() const {
	return (SymbolTable::SymbolListPtr)&symbolsRamExp;
}
A32u4::SymbolTable::SymbolListPtr A32u4::SymbolTable::getSymbolsRom() const {
	return (SymbolTable::SymbolListPtr)&symbolsRom;
}

A32u4::SymbolTable::symb_size_t A32u4::SymbolTable::getMaxRamAddrEnd() const {
	return maxRamAddrEnd;
}



/*

if (startStrOff == nullptr) {
//LogBackend::log(LogBackend::LogLevel_Warning, "could not read symbol table dump since it doesnt contain \"SYMBOL TABLE:\"");
//return false;
}

*/