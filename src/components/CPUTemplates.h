#include <algorithm>

template<typename T>
void A32u4::CPU::addCycles(T amt) {
	totalCycls += amt;
}

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
void A32u4::CPU::execute3T(uint64_t amt) {
	targetCycs += amt;
	while (totalCycls < targetCycs) {
		breakOutOfOptim = false;
		if (!CPU_sleep) {
			uint16_t prescCycs;
			switch (mcu->dataspace.timers.timer0_presc_cache) {
			case 0: {
				uint8_t addToCycs;
				int16_t addToPC;
				instHandler.handleCurrentInstT<debug, analyse>(addToCycs, addToPC);
				addCycles(addToCycs);
				PC += addToPC;
				goto skip_for;
			}
			case 1: {
				uint8_t addToCycs;
				int16_t addToPC;
				instHandler.handleCurrentInstT<debug, analyse>(addToCycs, addToPC);
				addCycles(addToCycs);
				PC += addToPC;
				mcu->dataspace.timers.doTicks(mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0), addToCycs);
				mcu->dataspace.timers.checkForIntr();
				goto skip_for;
			}
			case 2:
				prescCycs = 8;
				break;

			case 3:
				prescCycs = 64;
				break;

			case 4:
				prescCycs = 256;
				break;

			case 5:
				prescCycs = 1024;
				break;
			}

			{
				uint8_t addToCycs;
				int16_t addToPC;

				uint16_t amtCycs = prescCycs;

				for (int i = totalCycls % prescCycs; i <= amtCycs && !breakOutOfOptim; i += addToCycs) {
					instHandler.handleCurrentInstT<debug, analyse>(addToCycs, addToPC);
					PC += addToPC;
					addCycles(addToCycs);
				}
				if (!breakOutOfOptim) {
					mcu->dataspace.timers.doTick(mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0));
				}
				mcu->dataspace.timers.checkForIntr();
			}

		skip_for:
		;
		}
		else {
	#if !SLEEP_SKIP
			addCycles((uint8_t)8);
			mcu->dataspace.timers.update();
	#else
			uint8_t& REF_TIMER0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
			uint8_t timerTicksLeft = 255 - REF_TIMER0;
			REF_TIMER0 = 255;

			uint16_t prescCycs = DataSpace::Timers::presc[mcu->dataspace.timers.timer0_presc_cache];
			uint32_t cycsLeft = timerTicksLeft * prescCycs + (prescCycs - (totalCycls%prescCycs));

			mcu->dataspace.timers.doTick(mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0));
			mcu->dataspace.timers.checkForIntr();

			addCycles(cycsLeft);
			if(analyse){
				mcu->analytics.sleepSum += cycsLeft;
			}
	#endif
		}
		//executeInterrupts();
		if (mcu->debugger.isHalted()) {
			targetCycs = std::max(targetCycs - amt, totalCycls);
			return;
		}
	}
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
					InstHandler::inst_effect_t res = instHandler.handleCurrentInstT<debug, analyse>();
					addCycles(res.addCycs);
					PC += res.addPC;
				}else{
					InstHandler::inst_effect_t res = instHandler.handleCurrentInstT<debug, analyse>();
					addCycles(res.addCycs);
					PC += res.addPC;
					mcu->dataspace.timers.doTicks(mcu->dataspace.data[DataSpace::Consts::TCNT0], res.addCycs);
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
					InstHandler::inst_effect_t res = instHandler.handleCurrentInstT<debug, analyse>();
					addCycles(res.addCycs);
					PC += res.addPC;
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

				addCycles(sleepCycsLeft);

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
				addCycles(skipCycs);
				timer0 += (uint8_t)(sleepCycsLeft / DataSpace::Timers::presc[mcu->dataspace.timers.getTimer0Presc()]);
				mcu->dataspace.timers.markTimer0Update();
				//printf("slept for: %llu, continuing later\n", sleepCycsLeft);
			}
#endif
		}
		//executeInterrupts();
	}
}