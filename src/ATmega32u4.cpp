#include "ATmega32u4.h"

#include "StringUtils.h"
#include "StreamUtils.h"

#define MCU_MODULE "Inst Handler"
#include "components/InstHandlerTemplates.h"
#undef MCU_MODULE
#define MCU_MODULE "CPU"
#include "components/CPUTemplates.h"
#undef MCU_MODULE

#define MCU_MODULE "ATmega32u4"

#undef MCU_MCUPTR_PREFIX
#define MCU_MCUPTR_PREFIX 

A32u4::ATmega32u4* A32u4::ATmega32u4::currLogTarget = nullptr;

A32u4::ATmega32u4::ATmega32u4(): cpu(this), dataspace(this), flash(this), debugger(this), symbolTable(this) {
	activateLog();
}
A32u4::ATmega32u4::ATmega32u4(const ATmega32u4& src): 
logCallB(src.logCallB), wasReset(src.wasReset),
cpu(src.cpu), dataspace(src.dataspace), flash(src.flash), debugger(src.debugger), symbolTable(src.symbolTable)
{
	setMcu();
}
A32u4::ATmega32u4& A32u4::ATmega32u4::operator=(const ATmega32u4& src){
	logCallB = src.logCallB;
	wasReset = src.wasReset;

	cpu = src.cpu;
	dataspace = src.dataspace;
	flash = src.flash;
	debugger = src.debugger;
	analytics = src.analytics;
	symbolTable = src.symbolTable;
	setMcu();

	return *this;
}

void A32u4::ATmega32u4::getState(std::ostream& output){
	StreamUtils::write(output, wasReset);

	cpu.getState(output);
	dataspace.getState(output);
	flash.getState(output);

	debugger.getState(output);
	analytics.getState(output);
	symbolTable.getState(output);
}
void A32u4::ATmega32u4::setState(std::istream& input){
	StreamUtils::read(input, &wasReset);

	cpu.setState(input);
	dataspace.setState(input);
	flash.setState(input);

	debugger.setState(input);
	analytics.setState(input);
	symbolTable.setState(input);
}

void A32u4::ATmega32u4::setMcu() {
	cpu.mcu = this;
	dataspace.mcu = this;
	flash.mcu = this;
	debugger.mcu = this;
	symbolTable.mcu = this;
}

void A32u4::ATmega32u4::reset() { //add: reason
	MCU_LOG(LogLevel_Output, "Reset");
	debugger.reset();
	analytics.reset();
	resetHardware();

	wasReset = true;
}
void A32u4::ATmega32u4::resetHardware() {
	dataspace.reset();
	cpu.reset();
}
void A32u4::ATmega32u4::powerOn() {
	reset();
	dataspace.setRegBit(A32u4::DataSpace::Consts::MCUSR, A32u4::DataSpace::Consts::MCUSR_PORF, true);
}

void A32u4::ATmega32u4::execute(uint64_t cyclAmt, uint8_t flags) {
	if (!wasReset)
		abort();

	if(!flash.isProgramLoaded())
		return;
	switch (flags) {
		case ExecFlags_None:
			cpu.execute<false,false>(cyclAmt);
			break;
		case ExecFlags_Debug:
			cpu.execute<true, false>(cyclAmt);
			break;
		case ExecFlags_Analyse:
			cpu.execute<false, true>(cyclAmt);
			break;
		case ExecFlags_Debug | ExecFlags_Analyse:
			cpu.execute<true, true>(cyclAmt);
			break;
		default:
			MCU_LOG(LogLevel_Error, "Unhandeled Flags: " + StringUtils::uIntToBinStr(flags,8));
			break;
	}
}

void A32u4::ATmega32u4::log(LogLevel logLevel, const char* msg, const char* fileName, int lineNum, const char* module) {
	MCU_ASSERT(logCallB != nullptr);
	logCallB(logLevel, msg,fileName,lineNum,module,logCallBUserData);

	if (logLevel == LogLevel_Error) {
		debugger.halt();
	}
}
void A32u4::ATmega32u4::log(LogLevel logLevel, const std::string& msg, const char* fileName, int lineNum, const char* module) {
	log(logLevel, msg.c_str(), fileName, lineNum, module);
}

void A32u4::ATmega32u4::activateLog() {
	currLogTarget = this;
}
void A32u4::ATmega32u4::log_(LogLevel logLevel, const char* msg, const char* fileName, int lineNum, const char* module) {
	if (currLogTarget)
		currLogTarget->log(logLevel, msg, fileName, lineNum, module);
}
void A32u4::ATmega32u4::log_(LogLevel logLevel, const std::string& msg, const char* fileName, int lineNum, const char* module){
	if (currLogTarget)
		currLogTarget->log(logLevel, msg, fileName, lineNum, module);
}

void A32u4::ATmega32u4::setLogCallB(LogCallB newLogCallB, void* userData){
	logCallB = newLogCallB;
	logCallBUserData = userData;
}
void A32u4::ATmega32u4::defaultLogHandler(LogLevel logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData){
	MCU_UNUSED(userData);
	printf("[%s]%s: %s", 
		A32u4::ATmega32u4::logLevelStrs[logLevel],
		module != nullptr ? (std::string("[")+module+"]").c_str() : "",
		msg
	);
	if(fileName != nullptr || lineNum != -1) {
		printf(" [%s:%d]", fileName, lineNum);
	}
	printf("\n");
}


bool A32u4::ATmega32u4::load(const uint8_t* data, size_t dataLen){
	bool isElf = false;
	if(dataLen >= 4 && std::memcmp(data, "\x7f" "ELF", 4) == 0){ // check for magic number
		isElf = true;
	}

	bool success = false;
	if(isElf){
		success = loadFromELF(data, dataLen);
		if(!success){
			MCU_LOG(LogLevel_Error, "Couldn't load program from ELF Data");
			return false;
		}
	}else{
		success = flash.loadFromHexString((const char*)data);
		if(!success){
			MCU_LOG(LogLevel_Error, "Couldn't load program from Hex Data: ");
			return false;
		}
	}

	return true;
}
bool A32u4::ATmega32u4::loadFile(const char* path) {
	const char* ext = StringUtils::getFileExtension(path);

	if (std::strcmp(ext, "hex") == 0) {
		flash.loadFromHexFile(path);
	}
	else if (std::strcmp(ext, "elf") == 0) {
		loadFromELFFile(path);
	}
	else {
		MCU_LOGF(LogLevel_Error, "Cant load file with extension %s! Trying to load: %s", ext, path);
		return false;
	}
	return true;
}

bool A32u4::ATmega32u4::loadFromHex(const uint8_t* data, size_t dataLen){
	return flash.loadFromHexString((const char*)data, (const char*)data+dataLen);
}
bool A32u4::ATmega32u4::loadFromHexFile(const char* path){
	return flash.loadFromHexFile(path);
}

bool A32u4::ATmega32u4::loadFromELF(const uint8_t* data, size_t dataLen) {
	ELF::ELFFile elf = ELF::parseELFFile(data, dataLen);

	symbolTable.loadFromELF(elf);

	size_t textInd = elf.getIndOfSectionWithName(".text");
	size_t dataInd = elf.getIndOfSectionWithName(".data");

	if (textInd != (size_t)-1 && dataInd != (size_t)-1) {
		size_t len = elf.sectionContents[textInd].second + elf.sectionContents[dataInd].second;
		uint8_t* romData = new uint8_t[len];
		std::memcpy(romData, &elf.data[0] + elf.sectionContents[textInd].first, elf.sectionContents[textInd].second);
		std::memcpy(romData + elf.sectionContents[textInd].second, &elf.data[0] + elf.sectionContents[dataInd].first, elf.sectionContents[dataInd].second);

		flash.loadFromMemory(romData, len);

		delete[] romData;

		MCU_LOG(LogLevel_DebugOutput, "Successfully loaded Flash content from elf!");
		return true;
	}
	else {
		MCU_LOGF(LogLevel_Error, "Couldn't find required sections for execution: %s %s", textInd == (size_t)-1 ? ".text" : "", dataInd == (size_t)-1 ? ".data" : "");
		return false;
	}
}

bool A32u4::ATmega32u4::loadFromELFFile(const char* path) {
	bool success = true;
	std::vector<uint8_t> content = StringUtils::loadFileIntoByteArray(path, &success);
	if (!success) {
		MCU_LOGF(LogLevel_Error, "Couldn't load ELF file: %s", path);
		return false;
	}
		
	return loadFromELF(&content[0], content.size());
}

bool A32u4::ATmega32u4::operator==(const ATmega32u4& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(wasReset) &&
		_CMP_(cpu) && _CMP_(dataspace) && _CMP_(flash)&&
		_CMP_(debugger) && _CMP_(analytics);
#undef _CMP_
}
