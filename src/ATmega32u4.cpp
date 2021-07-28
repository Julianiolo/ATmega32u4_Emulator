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
			log("Unhandeled Flags: " + stringExtras::intToBin(flags,8));
			break;
	}
}

void A32u4::ATmega32u4::log(const char* msg) {
	if (logCallB != nullptr) {
		logCallB(msg);
	}
	else {
		std::cout << msg << std::endl;
	}
}
void A32u4::ATmega32u4::log(const std::string& msg) {
	log(msg.c_str());
}

void A32u4::ATmega32u4::setLogCallB(void (*newCallB)(const char *str)){
	logCallB = newCallB;
}