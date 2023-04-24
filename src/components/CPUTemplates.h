#include <algorithm>
#include "../A32u4Types.h"

template<bool debug>
void A32u4::CPU::execute(uint64_t amt) {
#if MCU_INCLUDE_EXTRAS
	if (debug) {
		if (mcu->debugger.execShouldReturn()) {
			return;
		}
	}
#endif
	execute4T<debug>(amt);
}

template<bool debug>
void A32u4::CPU::execute4T(uint64_t amt) {
	targetCycls += amt;
	size_t cnt = 0;
	while (totalCycls < targetCycls) {
		breakOutOfOptim = false;
		cnt++;
		if(cnt >= amt*2) {
			LU_LOGF(LogUtils::LogLevel_Error,"WTF %" PRIu64 " %" PRIu64 " %" PRIu64 " %d %" PRIu64, totalCycls,targetCycls,amt,(int)CPU_sleep,mcu->dataspace.cycsToNextTimerInt());
			mcu->debugger.halt();
			return;
		}

#if MCU_INCLUDE_EXTRAS
		if (mcu->debugger.isHalted() && !mcu->debugger.doStep) {
			targetCycls = std::max(targetCycls - amt, totalCycls);
			return;
		}
#endif

		if (!CPU_sleep) {
			if(mcu->dataspace.getTimer0Presc() <= 1){
				if(mcu->dataspace.getTimer0Presc() == 0){
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
				}else{
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
					mcu->dataspace.doTicks(res.addToCycs);
					mcu->dataspace.checkForIntr();
				}
			}
			else{
				uint64_t cycsToNextInt = mcu->dataspace.cycsToNextTimerInt();
				
				uint64_t currTargetCycs = totalCycls + cycsToNextInt;
				
				if (currTargetCycs > targetCycls) {
					currTargetCycs = targetCycls;
				}

				while(totalCycls < currTargetCycs && !breakOutOfOptim) {
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
				}

				mcu->dataspace.updateTimers();
			}
		}
		else { // sleeping
			DU_ASSERT(totalCycls < targetCycls);
#if !SLEEP_SKIP
			addCycles((uint8_t)1);
			mcu->dataspace.timers.update();
#else
			if (sleepCycsLeft == 0) {
				sleepCycsLeft = mcu->dataspace.cycsToNextTimerInt();
				if(sleepCycsLeft == 0) {
					printf("");
				}
				//printf("entering sleep for: %llu, with t:%d at %llu\n", sleepCycsLeft, mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0), mcu->cpu.totalCycls);
			}

			if (sleepCycsLeft <= (targetCycls - totalCycls) ) {
				// done sleeping
				totalCycls += sleepCycsLeft;

				mcu->dataspace.updateTimers();
				if(mcu->dataspace.data[DataSpace::Consts::TCNT0] != 0 || !(mcu->dataspace.data[DataSpace::Consts::TIFR0] & (1 << DataSpace::Consts::TIFR0_TOV0)))
					printf("AAAAAAA");

#if MCU_INCLUDE_EXTRAS
				if(debug){
					mcu->analytics.sleepSum += sleepCycsLeft;
				}
#endif
				//printf("done: slept for: %llu at %llu\n", sleepCycsLeft, mcu->cpu.totalCycls);
				sleepCycsLeft = 0;
			}
			else {
				uint64_t skipCycs = targetCycls - totalCycls;
				sleepCycsLeft -= skipCycs;

#if MCU_INCLUDE_EXTRAS
				if(debug){
					mcu->analytics.sleepSum += skipCycs;
				}
#endif

				totalCycls += skipCycs;
				//mcu->dataspace.updateTimers();
				//printf("slept for: %llu, %llu left, continuing later\n", skipCycs, sleepCycsLeft);
			}
#endif
		}
		//executeInterrupts();
	}
}