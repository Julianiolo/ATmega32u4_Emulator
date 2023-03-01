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

A32u4::ATmega32u4::ATmega32u4(): cpu(this), dataspace(this), flash(this)
#if MCU_INCLUDE_EXTRAS
,debugger(this)
,symbolTable(this) 
#endif
{
	activateLog();
}
A32u4::ATmega32u4::ATmega32u4(const ATmega32u4& src): 
logCallB(src.logCallB), running(src.running),
cpu(src.cpu), dataspace(src.dataspace), flash(src.flash)
#if MCU_INCLUDE_EXTRAS
, debugger(src.debugger)
, analytics(src.analytics)
, symbolTable(src.symbolTable)
#endif
{
	setMcu();
}
A32u4::ATmega32u4& A32u4::ATmega32u4::operator=(const ATmega32u4& src){
	logCallB = src.logCallB;
	running = src.running;

	cpu = src.cpu;
	dataspace = src.dataspace;
	flash = src.flash;

#if MCU_INCLUDE_EXTRAS
	debugger = src.debugger;
	analytics = src.analytics;
	symbolTable = src.symbolTable;
#endif
	setMcu();

	return *this;
}

void A32u4::ATmega32u4::getState(std::ostream& output){
	StreamUtils::write(output, running);

	cpu.getState(output);
	dataspace.getState(output);
	flash.getState(output);

#if MCU_INCLUDE_EXTRAS
	debugger.getState(output);
	analytics.getState(output);
	symbolTable.getState(output);
#endif
}
void A32u4::ATmega32u4::setState(std::istream& input){
	StreamUtils::read(input, &running);

	cpu.setState(input);
	dataspace.setState(input);
	flash.setState(input);

#if MCU_INCLUDE_EXTRAS
	debugger.setState(input);
	analytics.setState(input);
	symbolTable.setState(input);
#endif
}

void A32u4::ATmega32u4::setMcu() {
	cpu.mcu = this;
	dataspace.mcu = this;
#if MCU_INCLUDE_EXTRAS
	debugger.mcu = this;
	symbolTable.mcu = this;
#endif
}

void A32u4::ATmega32u4::reset() { //add: reason
	MCU_LOG(LogLevel_Output, "Reset");

#if MCU_INCLUDE_EXTRAS
	debugger.reset();
	analytics.reset();
#endif

	resetHardware();

	running = true;
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
	if (!running)
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
		running = false;
#if MCU_INCLUDE_EXTRAS
		debugger.halt();
#endif
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
		printf(" [%s:%d]", StringUtils::getFileName(fileName), lineNum);
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
		return flash.loadFromHexFile(path);
	}
	else if (std::strcmp(ext, "bin") == 0) {
		bool success = true;
		std::vector<uint8_t> data = StringUtils::loadFileIntoByteArray(path, &success);
		if (!success) {
			MCU_LOGF(LogLevel_Error, "Was not able to open file: \"%s\"", path);
			return false;
		}
		
		return flash.loadFromMemory(data.size()>0? &data[0] : nullptr, data.size());
	}
	else if (std::strcmp(ext, "elf") == 0) {
		return loadFromELFFile(path);
	}
	else {
		MCU_LOGF(LogLevel_Error, "Can't load file with extension %s! Trying to load: %s", ext, path);
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

#if MCU_INCLUDE_EXTRAS
	symbolTable.loadFromELF(elf);
#endif

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
	return _CMP_(running) && _CMP_(cpu) && _CMP_(dataspace) && _CMP_(flash)
#if MCU_INCLUDE_EXTRAS
		&& _CMP_(debugger) && _CMP_(analytics) && _CMP_(symbolTable)
#endif
		;
#undef _CMP_
}

size_t A32u4::ATmega32u4::sizeBytes() const {
	size_t sum = 0;
	sum += sizeof(logCallB);
	sum += sizeof(logCallBUserData);
	sum += sizeof(running);

	sum += cpu.sizeBytes();
	sum += dataspace.sizeBytes();
	sum += flash.sizeBytes();

#if MCU_INCLUDE_EXTRAS
	sum += debugger.sizeBytes();
	sum += analytics.sizeBytes();
	sum += symbolTable.sizeBytes();
#endif

	return sum;
}
