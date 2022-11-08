#ifndef _A32u4_INSTHANDLERTEMPLATES
#define _A32u4_INSTHANDLERTEMPLATES

#include "Disassembler.h"

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleCurrentInstT(ATmega32u4* mcu) noexcept {
	if (debug) {
		if (mcu->debugger.checkBreakpoints()) {
			return inst_effect_t(0,0);
		}
	}

	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);

	return handleInstT<debug,analyse>(mcu,word);
}

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstT(ATmega32u4* mcu, uint16_t word) noexcept {
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
#if 0
	return instOnlyList[ind](mcu,word);
#else
	return callInstSwitch(ind, mcu, word);
#endif
}

template<bool debug, bool analyse>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstRawT(ATmega32u4* mcu, uint16_t word) noexcept {
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

	return instOnlyList[ind](mcu,word);
}

#endif