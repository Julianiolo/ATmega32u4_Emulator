#ifndef _A32u4_INSTHANDLERTEMPLATES
#define _A32u4_INSTHANDLERTEMPLATES

#include "Disassembler.h"

template<bool debug, bool analyse>
void A32u4::InstHandler::handleInstT(uint8_t& CYCL_ADD_Ref, int16_t& PC_ADD_Ref) {
	if (debug) {
		if (mcu->debugger.checkBreakpoints()) {
			PC_ADD_Ref = 0;
			CYCL_ADD_Ref = 0;
			return;
		}
	}

	cycs = 0;
	PC_add = 0;

	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);

	if (debug) {
		if (mcu->debugger.printDisassembly) {
			uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
			mcu->log(ATmega32u4::LogLevel_Output, Disassembler::disassemble(word, word2, mcu->cpu.PC));
		}
	}

	uint8_t ind = mcu->flash.getInstInd(mcu->cpu.PC);

	if (analyse) {
		mcu->analytics.addData(ind, mcu->cpu.PC);
	}

	(this->*(instList[ind].func))(word);
	//std::invoke(instList[ind].func, this, word);

	PC_ADD_Ref = PC_add;
	CYCL_ADD_Ref = cycs;
}

#endif