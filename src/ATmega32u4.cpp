#include "ATmega32u4.h"

#include <iostream>

#include "components/InstHandlerTemplates.h"
#include "components/CPUTemplates.h"

#include "utils/stringExtras.h"

A32u4::ATmega32u4::ATmega32u4(): cpu(this), dataspace(this), flash(this), debugger(this) {

}

void A32u4::ATmega32u4::reset() { //add: reason
	debugger.reset();
	analytics.reset();
	hardwareReset();
}
void A32u4::ATmega32u4::hardwareReset() {
	dataspace.resetIO();
	cpu.reset();
}
void A32u4::ATmega32u4::powerOn() {
	reset();
	dataspace.setRegBit(A32u4::DataSpace::Consts::MCUSR, A32u4::DataSpace::Consts::MCUSR_PORF, true);
}

void A32u4::ATmega32u4::execute(uint64_t cyclAmt, uint8_t flags) {
	currentExecFlags = flags;
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
			log("Unhandeled Flags: " + stringExtras::intToBin(flags,8), LogLevel_Error);
			break;
	}
}

int A32u4::ATmega32u4::logFlags = 0;

void A32u4::ATmega32u4::log(const char* msg, LogLevel logLevel, const char* fileName, size_t lineNum, const char* Module) {
	std::string outMsg = "";
	if (logFlags != LogFlags_None) {
		std::string info = "";
		if (Module && (logFlags & LogFlags_ShowModule)) {
			info += Module;
		}
		if (fileName && lineNum != -1 && (logFlags & LogFlags_ShowModule)) {
			if (info.size() > 0) info += ", ";
			info += "in File: " + std::string(fileName) + " at line: " + std::to_string(lineNum);
		}
		if (info.size() > 0) {
			outMsg += "[" + info + "] ";
		}
	}
	outMsg += msg;

	if (logCallB != nullptr) {
		logCallB(outMsg.c_str());
	}
	else {
		std::cout << outMsg << std::endl;
	}

	if (logLevel == LogLevel_Error) {
		if (currentExecFlags & ExecFlags_Debug) {
			debugger.halt();
		}
		else {
			abort();
		}
	}
}
void A32u4::ATmega32u4::log(const std::string& msg, LogLevel logLevel, const char* fileName, size_t lineNum, const char* Module) {
	log(msg.c_str(), logLevel, fileName, lineNum, Module);
}

void A32u4::ATmega32u4::setLogCallB(void (*newCallB)(const char *str)){
	logCallB = newCallB;
}