#include <algorithm>

template<bool debug, bool analyse>
void A32u4::CPU::execute(uint64_t amt) {
	if (debug) {
		if (mcu->debugger.execShouldReturn()) {
			return;
		}
	}
	execute4T<debug, analyse>(amt);
}

template<bool debug, bool analyse>
void A32u4::CPU::execute4T(uint64_t amt) {
	targetCycs += amt;
	while (totalCycls < targetCycs) {
		breakOutOfOptim = false;

		if (mcu->debugger.isHalted() && !mcu->debugger.doStep) {
			targetCycs = std::max(targetCycs - amt, totalCycls);
			return;
		}

		if (!CPU_sleep) {
			if(mcu->dataspace.timers.getTimer0Presc() <= 1){
				if(mcu->dataspace.timers.getTimer0Presc() == 0){
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug, analyse>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
				}else{
					InstHandler::inst_effect_t res = InstHandler::handleCurrentInstT<debug, analyse>(mcu);
					totalCycls += res.addToCycs;
					PC += res.addToPC;
					mcu->dataspace.timers.doTicks(mcu->dataspace.data[DataSpace::Consts::TCNT0], res.addToCycs);
					mcu->dataspace.timers.checkForIntr();
				}
			}
			else{
				uint64_t cycsToNextInt = cycsToNextTimerInt();
				
				bool doTimerTick = true;
				uint64_t currTargetCycs = totalCycls + cycsToNextInt;
				
				if (currTargetCycs > targetCycs) {
					currTargetCycs = targetCycs;
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
					mcu->dataspace.timers.doTick(timer0);
				}
				else {
					timer0 += (uint8_t)((totalCycls - mcu->dataspace.timers.lastTimer0Update) / DataSpace::Timers::presc[mcu->dataspace.timers.getTimer0Presc()]);
				}
				mcu->dataspace.timers.markTimer0Update();
				mcu->dataspace.timers.checkForIntr();
			}
		}
		else {
#if !SLEEP_SKIP
			addCycles((uint8_t)8);
			mcu->dataspace.timers.update();
#else
			if (sleepCycsLeft == 0) {
				sleepCycsLeft = cycsToNextTimerInt();
				//printf("entering sleep for: %llu, with t:%d at %llu\n", sleepCycsLeft, mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0), mcu->cpu.totalCycls);
			}

			uint8_t& timer0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
			if (sleepCycsLeft <= (targetCycs - totalCycls) ) {
				
				timer0 = 255;

				totalCycls += sleepCycsLeft;

				mcu->dataspace.timers.doTick(timer0);
				mcu->dataspace.timers.checkForIntr();

				if(analyse){
					mcu->analytics.sleepSum += sleepCycsLeft;
				}
				//printf("done: slept for: %llu at %llu\n", sleepCycsLeft, mcu->cpu.totalCycls);
				sleepCycsLeft = 0;
			}
			else {
				uint64_t skipCycs = targetCycs - totalCycls;
				sleepCycsLeft -= skipCycs;
				if(analyse){
					mcu->analytics.sleepSum += skipCycs;
				}
				totalCycls += skipCycs;
				timer0 += (uint8_t)(sleepCycsLeft / DataSpace::Timers::presc[mcu->dataspace.timers.getTimer0Presc()]);
				mcu->dataspace.timers.markTimer0Update();
				//printf("slept for: %llu, continuing later\n", sleepCycsLeft);
			}
#endif
		}
		//executeInterrupts();
	}
}