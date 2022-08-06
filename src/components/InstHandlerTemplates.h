#ifndef _A32u4_INSTHANDLERTEMPLATES
#define _A32u4_INSTHANDLERTEMPLATES

#include "Disassembler.h"

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleCurrentInstT() {
	if (debug) {
		if (mcu->debugger.checkBreakpoints()) {
			return inst_effect_t(0,0);
		}
	}

	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);

	return handleInstT<debug,analyse>(word);
}

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstT(uint16_t word){
	cycs = 0;
	PC_add = 0;

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

	(this->*(instOnlyList[ind]))(word);
	//std::invoke(instList[ind].func, this, word);

	return inst_effect_t(cycs,PC_add);
}

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstRawT(uint16_t word){
	cycs = 0;
	PC_add = 0;

	if (debug) {
		if (mcu->debugger.printDisassembly) {
			uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
			mcu->log(ATmega32u4::LogLevel_Output, Disassembler::disassemble(word, word2, mcu->cpu.PC));
		}
	}

	uint8_t ind = getInstInd(word);

	if (analyse) {
		mcu->analytics.addData(ind, mcu->cpu.PC);
	}

	(this->*(instOnlyList[ind]))(word);
	//std::invoke(instList[ind].func, this, word);

	return inst_effect_t(cycs,PC_add);
}

#endif