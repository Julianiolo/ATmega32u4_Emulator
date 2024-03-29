#include "ATmega32u4.h"

#include "StringUtils.h"
#include "StreamUtils.h"
#include "DataUtils.h"

#define LU_MODULE "Inst Handler"
#include "components/InstHandlerTemplates.h"
#undef LU_MODULE
#define LU_MODULE "CPU"
#include "components/CPUTemplates.h"
#undef LU_MODULE

#define LU_MODULE "ATmega32u4"

#undef LU_CONTEXT
#define LU_CONTEXT (std::pair<LogUtils::LogCallB,void*>{A32u4::ATmega32u4::_log,this})



A32u4::ATmega32u4::ATmega32u4(): cpu(this), dataspace(this), flash(this)
#if MCU_INCLUDE_EXTRAS
,debugger(this)
#endif
{

}
A32u4::ATmega32u4::ATmega32u4(const ATmega32u4& src): 
logCallB(src.logCallB), running(src.running),
cpu(src.cpu), dataspace(src.dataspace), flash(src.flash)
#if MCU_INCLUDE_EXTRAS
, debugger(src.debugger)
, analytics(src.analytics)
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
#endif
	setMcu();

	return *this;
}

void A32u4::ATmega32u4::setMcu() {
	cpu.mcu = this;
	dataspace.mcu = this;
	flash.mcu = this;
#if MCU_INCLUDE_EXTRAS
	debugger.mcu = this;
#endif
}

void A32u4::ATmega32u4::activateLog() {
	LogUtils::activateLogTarget(_log, this);
}

void A32u4::ATmega32u4::_log(uint8_t logLevel, const char* msg, const char* fileName, int lineNum, const char* module, void* userData) {
	ATmega32u4* atm = (ATmega32u4*)userData;
	DU_ASSERT(atm->logCallB);

	atm->logCallB(logLevel, msg, fileName, lineNum, module, atm->logCallBUserData);
}

void A32u4::ATmega32u4::reset() { //add: reason
	LU_LOG(LogUtils::LogLevel_Output, "Reset");

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

void A32u4::ATmega32u4::execute(uint64_t cyclAmt, bool debug) {
	if (!running)
		abort();

	if(!flash.isProgramLoaded())
		return;

	if (!debug) {
		cpu.execute<false>(cyclAmt);
	}
	else {
		cpu.execute<true>(cyclAmt);
	}
}


void A32u4::ATmega32u4::setLogCallB(LogUtils::LogCallB newLogCallB, void* userData){
	logCallB = newLogCallB;
	logCallBUserData = userData;
}
void A32u4::ATmega32u4::defaultLogHandler(uint8_t logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData){
	CU_UNUSED(userData);
	printf("[%s]%s: %s", 
		LogUtils::logLevelStrs[logLevel],
		module != nullptr ? (std::string("[")+module+"]").c_str() : "",
		msg
	);
	if(fileName != nullptr || lineNum != -1) {
		printf(" [%s:%d]", StringUtils::getFileName(fileName), lineNum);
	}
	printf("\n");
}


bool A32u4::ATmega32u4::loadFile(const char* path) {
	const char* ext = StringUtils::getFileExtension(path);

	if (std::strcmp(ext, "hex") == 0) {
		return flash.loadFromHexFile(path);
	}
	else if (std::strcmp(ext, "bin") == 0) {
		std::vector<uint8_t> data;
		try {
			data = StringUtils::loadFileIntoByteArray(path);
		}
		catch (const std::runtime_error& e) {
			LU_LOGF(LogUtils::LogLevel_Error, "Couldn't open file: \"%s\"", e.what());
			return false;
		}
		
		return flash.loadFromMemory(data.size()>0? &data[0] : nullptr, data.size());
	}
	else {
		LU_LOGF(LogUtils::LogLevel_Error, "Can't load file with extension %s! Trying to load: %s", ext, path);
		return false;
	}
	return true;
}

void A32u4::ATmega32u4::setPinChangeCallB(const std::function<void(uint8_t pinReg, reg_t oldVal, reg_t val)>& callB){
	pinChangeCallB = callB;
}


void A32u4::ATmega32u4::getState(std::ostream& output){
	StreamUtils::write(output, running);

	cpu.getState(output);
	dataspace.getState(output);
	flash.getState(output);

#if MCU_INCLUDE_EXTRAS
	debugger.getState(output);
	analytics.getState(output);
#endif

#if MCU_WRITE_HASH
	StreamUtils::write(output, hash());
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
#endif

	A32U4_CHECK_HASH("ATmega32u4");
}


bool A32u4::ATmega32u4::operator==(const ATmega32u4& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(running) && _CMP_(cpu) && _CMP_(dataspace) && _CMP_(flash)
#if MCU_INCLUDE_EXTRAS
		&& _CMP_(debugger) && _CMP_(analytics)
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
#endif

	return sum;
}

uint32_t A32u4::ATmega32u4::hash() const noexcept {
	uint32_t h = 0;
	DU_HASHC(h, cpu);
	DU_HASHC(h, dataspace);
	DU_HASHC(h, flash);

#if MCU_INCLUDE_EXTRAS
	DU_HASHC(h, analytics);
	DU_HASHC(h, debugger);
#endif

	return h;
}
