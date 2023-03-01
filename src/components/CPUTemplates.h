#include <algorithm>
#include "../A32u4Types.h"

template<bool debug, bool analyse>
void A32u4::CPU::execute(uint64_t amt) {
#if MCU_INCLUDE_EXTRAS
	if (debug) {
		if (mcu->debugger.execShouldReturn()) {
			return;
		}
	}
#endif
	execute4T<debug, analyse>(amt);
}

template<bool debug, bool analyse>
void A32u4::CPU::execute4T(uint64_t amt) {
	targetCycls += amt;
	size_t cnt = 0;
	while (totalCycls < targetCycls) {
		breakOutOfOptim = false;
		cnt++;
		if(cnt >= amt*2) {
			printf("WTF %" PRIu64 " %" PRIu64 " %" PRIu64 " %d %" PRIu64 "\n",totalCycls,targetCycls,amt,(int)CPU_sleep,cycsToNextTimerInt());
			abort();
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
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug, analyse>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
				}else{
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug, analyse>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
					mcu->dataspace.doTicks(res.addToCycs);
					mcu->dataspace.checkForIntr();
				}
			}
			else{
				uint64_t cycsToNextInt = cycsToNextTimerInt();
				
				bool doTimerTick = true;
				uint64_t currTargetCycs = totalCycls + cycsToNextInt;
				
				if (currTargetCycs > targetCycls) {
					currTargetCycs = targetCycls;
					doTimerTick = false;
				}

				while(totalCycls < currTargetCycs && !breakOutOfOptim) {
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug, analyse>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
				}

				uint8_t& timer0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
				if (totalCycls >= currTargetCycs && doTimerTick) {
					timer0 = 255;
					mcu->dataspace.doTick(timer0);
				}
				else {
					timer0 += (uint8_t)((totalCycls - mcu->dataspace.lastSet.Timer0Update) / mcu->dataspace.getTimer0PrescDiv());
				}
				mcu->dataspace.markTimer0Update();
				mcu->dataspace.checkForIntr();
			}
		}
		else {
#if !SLEEP_SKIP
			addCycles((uint8_t)1);
			mcu->dataspace.timers.update();
#else
			if (sleepCycsLeft == 0) {
				sleepCycsLeft = cycsToNextTimerInt();
				//printf("entering sleep for: %llu, with t:%d at %llu\n", sleepCycsLeft, mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0), mcu->cpu.totalCycls);
			}

			uint8_t& timer0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
			if (sleepCycsLeft <= (targetCycls - totalCycls) ) {
				// done sleeping
				timer0 = 255;

				totalCycls += sleepCycsLeft;

				mcu->dataspace.doTick(timer0);
				mcu->dataspace.checkForIntr();

#if MCU_INCLUDE_EXTRAS
				if(analyse){
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
				if(analyse){
					mcu->analytics.sleepSum += skipCycs;
				}
#endif

				totalCycls += skipCycs;
				timer0 += (uint8_t)(sleepCycsLeft / mcu->dataspace.getTimer0PrescDiv());
				mcu->dataspace.markTimer0Update();
				//printf("slept for: %llu, continuing later\n", sleepCycsLeft);
			}
#endif
		}
		//executeInterrupts();
	}
}