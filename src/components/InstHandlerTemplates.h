#ifndef _A32u4_INSTHANDLERTEMPLATES
#define _A32u4_INSTHANDLERTEMPLATES

#include "../extras/Disassembler.h"

template<bool debug>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleCurrentInstT(ATmega32u4* mcu) noexcept {
#if MCU_INCLUDE_EXTRAS
	if constexpr (debug) {
		if (mcu->debugger.checkBreakpoints()) {
			return inst_effect_t(0,0);
		}
	}
#endif

	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);

	return handleInstT<debug>(mcu,word);
}

template<bool debug>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstT(ATmega32u4* mcu, uint16_t word) noexcept {
#if MCU_INCLUDE_EXTRAS
	if constexpr (debug) {
		if (mcu->debugger.printDisassembly) {
			uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
			LU_LOG(LogUtils::LogLevel_Output, Disassembler::disassemble(word, word2, mcu->cpu.PC));
		}
	}
#endif

	uint8_t ind = mcu->flash.getInstInd(mcu->cpu.PC);

#if MCU_INCLUDE_EXTRAS
	if constexpr (debug) {
		mcu->analytics.addData(ind, mcu->cpu.PC);
	}
#endif


#if MCU_USE_INST_EXEC_ALG == 0
	return instOnlyList[ind](mcu,word);
#elif MCU_USE_INST_EXEC_ALG == 1
	return callInstSwitch(ind, mcu, word);
#else
	#error There is no INST_EXEC Algorithm selected
#endif
}

template<bool debug>
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::handleInstRawT(ATmega32u4* mcu, uint16_t word) noexcept {
#if MCU_INCLUDE_EXTRAS
	if constexpr (debug) {
		if (mcu->debugger.printDisassembly) {
			uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
			LU_LOG(LogUtils::LogLevel_Output, Disassembler::disassemble(word, word2, mcu->cpu.PC));
		}
	}
#endif

	uint8_t ind = getInstInd(word);

#if MCU_INCLUDE_EXTRAS
	if constexpr (debug) {
		mcu->analytics.addData(ind, mcu->cpu.PC);
	}
#endif

	return instOnlyList[ind](mcu,word);
}

#endif