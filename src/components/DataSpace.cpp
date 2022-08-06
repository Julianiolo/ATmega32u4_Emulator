#include "DataSpace.h"

#include <iostream>

#include "../ATmega32u4.h"
#include "Debugger.h"

#include "../utils/bitMacros.h"

#include "CPUTemplates.h" // for addCycles (EEPROM)



A32u4::DataSpace::Timers::Timers(ATmega32u4* mcu) : mcu(mcu), lastCounter(0), 
REF_TCCR0B(mcu->dataspace.data[A32u4::DataSpace::Consts::TCCR0B]), 
REF_TIFR0(mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0])
{

}

void A32u4::DataSpace::Timers::reset() {
	timer0_presc_cache = 0;
	lastCounter = 0;
	lastTimer0Update = 0;
}

void A32u4::DataSpace::Timers::update() {
	abort();
#define USE_TIM0_CACHE 1

#if 1
#if USE_TIM0_CACHE
	if (timer0_presc_cache == 3) {
#else
	if ((REF_TCCR0B & 0b111) == 3) {
#endif
		goto fast_goto_64;
	}
#endif

	bool doTick;
#if USE_TIM0_CACHE
	switch (timer0_presc_cache) {
#else
	switch (REF_TCCR0B & 0b111) {
#endif
	case 0:
		doTick = false; break;
	case 1:
		doTick = true; break;
	case 2:
		doTick = ((uint32_t)mcu->cpu.totalCycls / 8) != lastCounter;
		lastCounter = (uint32_t)mcu->cpu.totalCycls / 8;
		break;
	case 3:
#if 1
		fast_goto_64:
		doTick = ((uint32_t)mcu->cpu.totalCycls / 64) != lastCounter;
		lastCounter = (uint32_t)mcu->cpu.totalCycls / 64;
#else
		{
			uint64_t val = mcu->cpu.totalCycls / 64;
			doTick = val != lastCounter;
			lastCounter = val;
		}
#endif
		break;
	case 4:
		doTick = ((uint32_t)mcu->cpu.totalCycls / 256) != lastCounter;
		lastCounter = (uint32_t)mcu->cpu.totalCycls / 256;
		break;
	case 5:
		doTick = ((uint32_t)mcu->cpu.totalCycls / 1024) != lastCounter;
		lastCounter = (uint32_t)mcu->cpu.totalCycls / 1024;
		break;
	default:
		abort();
	}

	//uint8_t& REF_TIFR0 = mcu->dataspace.getByteRefAtAddr(TIFR0);

	if (doTick) {
#if 1
		if (isBitSetNB(mcu->dataspace.data[DataSpace::Consts::PRR0], 5)) {
#else
		if (isBitSetNB(REF_PRR0, 5)) {
#endif
			return;
		}
		uint8_t& timer0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
		timer0++;
		if (timer0 == 0) {	
			REF_TIFR0 |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
			goto direct_intr;
		}
	}

	if (REF_TIFR0 & (1 << DataSpace::Consts::TIFR0_TOV0)) {
		direct_intr:
		if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TIMSK0) & (1 << DataSpace::Consts::TIMSK0_TOIE0)) {
			if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG) & (1 << DataSpace::Consts::SREG_I)) {
				//mcu->cpu.queueInterrupt(23); // 0x2E timer0 overflow interrupt vector
				REF_TIFR0 &= ~(1 << DataSpace::Consts::TIFR0_TOV0);
				mcu->cpu.directExecuteInterrupt(23);
			}
		}
	}
}

void A32u4::DataSpace::Timers::doTick(uint8_t& timer) {
#if 1
	if (isBitSetNB(mcu->dataspace.data[DataSpace::Consts::PRR0], 5)) {
#else
	if (isBitSetNB(REF_PRR0, 5)) {
#endif
		return;
	}
	timer++;
	if (timer == 0) {
		REF_TIFR0 |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
		//printf("OVERFLOW at %llu\n", mcu->cpu.totalCycls);
	}

	markTimer0Update();


	static uint64_t last = 0;
	uint64_t diff = mcu->cpu.totalCycls - last;
	//printf("%llu\n", diff);
	last = mcu->cpu.totalCycls;

	if (diff > 17000)
		int a = 0;


}
void A32u4::DataSpace::Timers::doTicks(uint8_t& timer, uint8_t num) {
#if 1
	if (isBitSetNB(mcu->dataspace.data[DataSpace::Consts::PRR0], 5)) {
#else
	if (isBitSetNB(REF_PRR0, 5)) {
#endif
		return;
	}

	uint8_t& timer0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
	uint8_t tim_copy = timer0;
	timer0 += num;
	if (timer0 < tim_copy) {
		REF_TIFR0 |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
		//printf("OVERFLOW at %llu\n", mcu->cpu.totalCycls);
	}

	markTimer0Update();
}

void A32u4::DataSpace::Timers::checkForIntr() {
	if (REF_TIFR0 & (1 << DataSpace::Consts::TIFR0_TOV0)) {
		//std::cout << mcu->cpu.totalCycls << std::endl;
		if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TIMSK0) & (1 << DataSpace::Consts::TIMSK0_TOIE0)) {
			if (mcu->dataspace.sreg[DataSpace::Consts::SREG_I]) {
				//mcu->cpu.queueInterrupt(23); // 0x2E timer0 overflow interrupt vector
				REF_TIFR0 &= ~(1 << DataSpace::Consts::TIFR0_TOV0);
				mcu->cpu.directExecuteInterrupt(23);
				//printf("int at %llu\n", mcu->cpu.totalCycls);
			}
		}
	}
}
uint8_t A32u4::DataSpace::Timers::getTimer0Presc() const {
	return timer0_presc_cache;
}
uint16_t A32u4::DataSpace::Timers::getTimer0PrescDiv() const {
	return DataSpace::Timers::presc[getTimer0Presc()];
}
void A32u4::DataSpace::Timers::markTimer0Update(bool print) { 
	// functions is supposed to set lastTimer0Update to the exact technically correct value, even if we are already past that
	uint64_t diff = mcu->cpu.totalCycls - lastTimer0Update;
	diff = (diff / getTimer0PrescDiv()) * getTimer0PrescDiv();
	lastTimer0Update += diff;
	//if(print)
	//	printf("marked at %llu\n", lastTimer0Update);
}



/*

DataSpace:

*/

A32u4::DataSpace::DataSpace(ATmega32u4* mcu) : mcu(mcu), 
#if USE_HEAP
data(new uint8_t[Consts::data_size]), eeprom(new uint8_t[Consts::eeprom_size]),
#endif
timers(mcu)
{

}

A32u4::DataSpace::~DataSpace() {
#if USE_HEAP
	delete[] data;
	delete[] eeprom;
#endif
}

void A32u4::DataSpace::reset() {
	resetIO();
	timers.reset();
	lastEECR_EEMPE_set = 0;
	lastPLLCSR_PLLE_set = 0;
	std::memset(sreg, 0, 8);
}
void A32u4::DataSpace::resetIO() {
	//add: set all IO Registers to initial Values
	for (size_t i = 0; i < Consts::io_size; i++) {
		data[Consts::io_start + i] = 0;
	}
	for (size_t i = 0; i < Consts::ext_io_size; i++) {
		data[Consts::ext_io_start + i] = 0;
	}
	for (size_t i = 0; i < Consts::eeprom_size; i++) {
		eeprom[i] = 0;
	}

	setSP(Consts::SP_initaddr);
}

MCU_INLINE uint8_t& A32u4::DataSpace::getByteRefAtAddr(uint16_t addr) {
	A32U4_ASSERT_INRANGE_M(addr, 0, Consts::data_size, "getByteRef addr out of bounds: " + std::to_string(addr), "DataSpace", return errorIndicator);
	return data[addr];
}
MCU_INLINE uint8_t& A32u4::DataSpace::getGPRegRef(uint8_t ind) {
	A32U4_ASSERT_INRANGE_M(ind, 0, Consts::GPRs_size, "General Purpouse Register Index out of bounds: " + std::to_string((int)ind), "DataSpace", return errorIndicator);
	return data[ind];
}
MCU_INLINE uint8_t A32u4::DataSpace::getGPReg(uint8_t ind) const {
	A32U4_ASSERT_INRANGE_M(ind, 0, Consts::GPRs_size, "General Purpouse Register Index out of bounds: " + std::to_string((int)ind), "DataSpace", return 0);
	return data[ind];
}

uint8_t A32u4::DataSpace::getByteAt(uint16_t addr) {
	A32U4_ASSERT_INRANGE_M(addr, 0, Consts::data_size, A32U4_ADDR_ERR_STR("Data get Index out of bounds: ",addr,4), "DataSpace", return 0);

	update_Get(addr, true);

	return data[addr];
}

void A32u4::DataSpace::setByteAt(uint16_t addr, uint8_t val) {
	A32U4_ASSERT_INRANGE_M(addr, 0, Consts::data_size, A32U4_ADDR_ERR_STR("Data set Index out of bounds: ",addr,4), "DataSpace", return);

	uint8_t oldVal = data[addr];
	data[addr] = val;
	update_Set(addr, val, oldVal);
}
uint8_t A32u4::DataSpace::getIOAt(uint8_t ind) {
	return getByteAt(ind + Consts::io_start);
}
void A32u4::DataSpace::setIOAt(uint8_t ind, uint8_t val) {
	setByteAt(ind + Consts::io_start, val);
}

uint8_t A32u4::DataSpace::getRegBit(uint16_t id, uint8_t bit) {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::data_size, A32U4_ADDR_ERR_STR("getRegBit Index out of bounds: ",id,4), "DataSpace", return 0);

	return (getByteRefAtAddr(id) & (1 << bit)) != 0;
}
void A32u4::DataSpace::setRegBit(uint16_t id, uint8_t bit, bool val) {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::data_size, A32U4_ADDR_ERR_STR("setRegBit Index out of bounds: ",id,4), "DataSpace", return);

	uint8_t& byte = getByteRefAtAddr(id);;
	if (val) {
		byte |= 1 << bit;
	}
	else {
		byte &= ~(1 << bit);
	}
}

MCU_INLINE uint16_t A32u4::DataSpace::getWordReg(uint8_t id) const {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::GPRs_size-1, A32U4_ADDR_ERR_STR("getWordReg Index out of bounds",id,2), "DataSpace", return 0);
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
MCU_INLINE void A32u4::DataSpace::setWordReg(uint8_t id, uint16_t val) {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::GPRs_size-1, A32U4_ADDR_ERR_STR("setWordReg Index out of bounds",id,2), "DataSpace", return);
	data[id + 1] = val >> 8;
	data[id] = (uint8_t)val;
}
MCU_INLINE uint16_t A32u4::DataSpace::getWordRegRam(uint16_t id) const {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::data_size-1, A32U4_ADDR_ERR_STR("getWordRegRam Index out of bounds",id,2), "DataSpace", return 0);
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
MCU_INLINE void A32u4::DataSpace::setWordRegRam(uint16_t id, uint16_t val) {
	A32U4_ASSERT_INRANGE_M(id, 0, Consts::data_size-1, A32U4_ADDR_ERR_STR("setWordRegRam Index out of bounds",id,2), "DataSpace", return);
	data[id + 1] = val >> 8;
	data[id] = (uint8_t)val;
}

MCU_INLINE uint16_t A32u4::DataSpace::getX() const {
	return ((uint16_t)data[0x1b] << 8) | data[0x1a];
}
MCU_INLINE uint16_t A32u4::DataSpace::getY() const {
	return ((uint16_t)data[0x1d] << 8) | data[0x1c];
}
MCU_INLINE uint16_t A32u4::DataSpace::getZ() const {
	return ((uint16_t)data[0x1f] << 8) | data[0x1e];
}
MCU_INLINE void A32u4::DataSpace::setX(uint16_t word) {
	data[0x1b] = (word & 0xFF00) >> 8;
	data[0x1a] = word & 0xFF;
}
MCU_INLINE void A32u4::DataSpace::setY(uint16_t word) {
	data[0x1d] = (word & 0xFF00) >> 8;
	data[0x1c] = word & 0xFF;
}
MCU_INLINE void A32u4::DataSpace::setZ(uint16_t word) {
	data[0x1f] = (word & 0xFF00) >> 8;
	data[0x1e] = word & 0xFF;
}

void A32u4::DataSpace::setSP(uint16_t val) {
	setWordRegRam(Consts::SPL, val);
	if(mcu->analytics.maxSP > val)
		mcu->analytics.maxSP = val;
}

void A32u4::DataSpace::update_Get(uint16_t Addr, bool onlyOne) {
	if (Addr >= Consts::GPRs_size && Addr <= Consts::io_start + Consts::io_size + Consts::ext_io_size) { //only io needs updates
		switch (Addr) {
			case 0xFFFF: //case for updating everything
			case Consts::EECR: {
				if ((data[Consts::EECR] & (1 << Consts::EECR_EEMPE)) && mcu->cpu.totalCycls - lastEECR_EEMPE_set > 4) { // check if EE;PE is set but shouldnt
					data[Consts::EECR] &= ~(1 << Consts::EECR_EEMPE); //clear EEMPE
				}
				if (onlyOne) break;
			}

			case Consts::PLLCSR: {
				if ((data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLLE)) && !(data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLOCK)) && mcu->cpu.totalCycls - lastPLLCSR_PLLE_set > PLLCSR_PLOCK_wait) { //if PLLE is 1 and PLOCK is 0 and enought time since PLLE set
					data[Consts::PLLCSR] |= (1 << Consts::PLLCSR_PLOCK); //set PLOCK
				}
				if (onlyOne) break;
			}

			case Consts::TCNT0: {
				data[Consts::TCNT0] += (uint8_t)((mcu->cpu.totalCycls - mcu->dataspace.timers.lastTimer0Update) / DataSpace::Timers::presc[mcu->dataspace.timers.getTimer0Presc()]);
				timers.markTimer0Update(false);
				if (onlyOne) break;
			}

			case Consts::SREG: {
				uint8_t val = 0;
				val |= (sreg[Consts::SREG_C] != 0) << Consts::SREG_C;
				val |= (sreg[Consts::SREG_Z] != 0) << Consts::SREG_Z;
				val |= (sreg[Consts::SREG_N] != 0) << Consts::SREG_N;
				val |= (sreg[Consts::SREG_V] != 0) << Consts::SREG_V;
				val |= (sreg[Consts::SREG_S] != 0) << Consts::SREG_S;
				val |= (sreg[Consts::SREG_H] != 0) << Consts::SREG_H;
				val |= (sreg[Consts::SREG_T] != 0) << Consts::SREG_T;
				val |= (sreg[Consts::SREG_I] != 0) << Consts::SREG_I;
				data[Consts::SREG] = val;
				if (onlyOne) break;
			}
		}
	}
}

void A32u4::DataSpace::update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal) {
	if (Addr <= Consts::io_start+Consts::io_size+Consts::ext_io_size && Addr >= Consts::GPRs_size) { //only io needs updates
		switch (Addr) {
			case Consts::EECR:
				return setEECR(val, oldVal);

			case Consts::PLLCSR:
				setPLLCSR(val, oldVal);
				break;

			case Consts::SPDR:
				setSPDR();
				break;

			case Consts::TCCR0B:
				setTCCR0B(val);
				break;

			case Consts::TIFR0:
				timers.checkForIntr();
				break;

			case Consts::SREG:
				for (uint8_t i = 0; i < 8; i++) {
					sreg[i] = val & (1 << i);
				}
				if((val & (1<<Consts::SREG_I)) && (data[Consts::TIFR0] & (1 << DataSpace::Consts::TIFR0_TOV0)))
					mcu->cpu.breakOutOfOptim = true; // we need to break out of Optimisation to check if an interrupt can now occur (Global Interrupt Enable)
				break;

			case Consts::TCNT0:
				printf("TIMER!\n");
				break;
		}
	}
}


void A32u4::DataSpace::setEECR(uint8_t val, uint8_t oldVal){
	if (val & (1 << Consts::EECR_EERE)) {
		data[Consts::EEDR] = eeprom[getWordRegRam(Consts::EEARL)];
		val = val & ~(1 << Consts::EECR_EERE); //idk if this should be done bc its not stated anywhere but its the only logical thing
		mcu->cpu.totalCycls += 4;
	}

	if ((val & (1 << Consts::EECR_EEMPE)) && !(oldVal & (1 << Consts::EECR_EEMPE))) {
		lastEECR_EEMPE_set = mcu->cpu.totalCycls;
	}

	if (val & (1 << Consts::EECR_EEPE)) {
		if (data[Consts::EECR] & (1 << Consts::EECR_EEMPE)) {
			uint8_t mode = data[Consts::EECR] >> 4;
			uint16_t Addr = getWordRegRam(Consts::EEARL);

			A32U4_ASSERT_INRANGE_M(Addr, 0, Consts::eeprom_size, A32U4_ADDR_ERR_STR("Eeprom addr out of bounds",Addr,4), "DataSpace", return);

			switch (mode) {
			case 0b00:
				eeprom[Addr] = data[Consts::EEDR];
				break;
			case 0b01:
				eeprom[Addr] = 0;
				break;
			case 0b10:
				eeprom[Addr] |= data[Consts::EEDR]; //idk if it actually works like that
				break;
			}

			// set EEPE to 0 to indicate being ready again
			data[Consts::EECR] &= ~(1 << Consts::EECR_EEPE);
		}
	}
}
void A32u4::DataSpace::setPLLCSR(uint8_t val, uint8_t oldVal) {
	if (val & (1 << Consts::PLLCSR_PLLE)) {
		if (!(oldVal & (1 << Consts::PLLCSR_PLLE))) { //if PLLE is 0 but should be 1
			lastPLLCSR_PLLE_set = mcu->cpu.totalCycls;
		}
	}
	else {
		if (oldVal & (1 << Consts::PLLCSR_PLLE)) { //if PLLE is 1 but should be 0
			data[Consts::PLLCSR] &= ~(1 << Consts::PLLCSR_PLOCK); //clear PLOCK
		}
	}
}
void A32u4::DataSpace::setSPDR() {
	if(SPI_Byte_Callback)
		SPI_Byte_Callback(data[Consts::SPDR]);

	for (uint8_t i = 0; i < 8; i++) {
		//Set SCK High
		//Set MOSI to REF_SPDR&0b1
		data[Consts::SPDR] >>= 1;
		if (SCK_Callback != NULL) {
			SCK_Callback();
		}
		//Set SCK LOW
	}
	data[Consts::SPSR] |= (1 << Consts::SPSR_SPIF);

	//request Interrupt if SPIE set
}
void A32u4::DataSpace::setTCCR0B(uint8_t val) {
	timers.timer0_presc_cache = val & 0b111;
	mcu->cpu.breakOutOfOptim = true;
	//printf("SWITCH to div:%d\n", timers.getTimer0PrescDiv());
	timers.lastTimer0Update = mcu->cpu.totalCycls; // dont use mark here since it shouldnt align with previous overflows (since this is the start and there are no previous overflows)
	//printf("fmark at %llu\n", mcu->cpu.totalCycls);

#if 1
	switch (timers.timer0_presc_cache) {
	case 2:
		timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 8;
		break;
	case 3:
		timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 64;
		break;
	case 4:
		timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 256;
		break;
	case 5:
		timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 1024;
		break;
	}
#endif
}

void A32u4::DataSpace::pushByteToStack(uint8_t val) {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE_M(SP, Consts::ISRAM_start, Consts::data_size, A32U4_ADDR_ERR_STR("Stack pointer while push Byte out of bounds: ",SP,4), "DataSpace", return);
	data[SP] = val;
	setSP(SP - 1);
}
uint8_t A32u4::DataSpace::popByteFromStack() {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE_M(SP+1, Consts::ISRAM_start, Consts::data_size, A32U4_ADDR_ERR_STR("Stack pointer while pop Byte out of bounds: ",SP,4), "DataSpace", return 0);
	uint8_t Byte = data[SP + 1];

	mcu->debugger.registerStackDec(SP + 1);

	setSP(SP + 1);
	return Byte;
}

void A32u4::DataSpace::pushAddrToStack(addrmcu_t Addr) {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE_M(SP, Consts::ISRAM_start+1, Consts::data_size, A32U4_ADDR_ERR_STR("Stack pointer while push Addr out of bounds: ",SP,4), "DataSpace", return);
	data[SP] = (uint8_t)Addr; //maybe this should be SP-1 and SP-2
	data[SP - 1] = (uint8_t)(Addr >> 8);

	mcu->debugger.registerAddressBytes(SP);

	setSP(SP - 2);
}
addrmcu_t A32u4::DataSpace::popAddrFromStack() {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE_M(SP+1, Consts::ISRAM_start, Consts::data_size-1, A32U4_ADDR_ERR_STR("Stack pointer while pop Addr out of bounds: ",SP,4), "DataSpace", return 0);
	uint16_t Addr = data[SP + 2];//maybe this should be SP-1 and SP-2 
	Addr |= ((uint16_t)data[SP + 1]) << 8;

	mcu->debugger.registerStackDec(SP + 2);

	setSP(SP + 2);
	return Addr;
}

void A32u4::DataSpace::setSPIByteCallB(SPIByteCallB func) {
	SPI_Byte_Callback = func;
}
uint8_t* A32u4::DataSpace::getEEPROM() {
	return eeprom;
}
// get a pointer to the updated Dataspce Data arr (only gets updated on first call if cpu.totalcycles doesnt change)
const uint8_t* A32u4::DataSpace::getData() {
	static uint64_t lastCycs = 0;
	if (lastCycs != mcu->cpu.totalCycls) {
		lastCycs = mcu->cpu.totalCycls;
		update_Get(0xFFFF, false);
	}
	return data;
}
uint8_t A32u4::DataSpace::getDataByte(addrmcu_t Addr) {
	return getByteAt(Addr);
}

void A32u4::DataSpace::setDataByte(addrmcu_t Addr, uint8_t byte) {
	setByteAt(Addr, byte);
}

void A32u4::DataSpace::setBitTo(addrmcu_t Addr, uint8_t bit, bool val) {
	uint8_t byte = getData()[Addr];
	if (val)
		byte |= 1 << bit;
	else
		byte &= ~(1 << bit);
	setDataByte(Addr, byte);
}
void A32u4::DataSpace::setBitsTo(addrmcu_t Addr, uint8_t mask, uint8_t bits) {
	uint8_t byte = getData()[Addr];
	byte = (byte & ~mask) | bits;
	setDataByte(Addr, byte);
}

addrmcu_t A32u4::DataSpace::getSP() const {
	return getWordRegRam(Consts::SPL);
}

/*


constexpr uint8_t A32u4::DataSpace::getDataByteC(addrmcu_t Addr) {
return getByteAtC(Addr);
}


constexpr uint8_t A32u4::DataSpace::getByteAtC(uint16_t addr) {
A32U4_ASSERT_INRANGE_M(addr, 0, Consts::data_size, A32U4_ADDR_ERR_STR("Data get Index out of bounds: ",addr,4), "DataSpace", return 0);

update_GetC(addr, true);

return data[addr];
}


constexpr void A32u4::DataSpace::update_GetC(uint16_t Addr, bool onlyOne) {
if (Addr <= Consts::io_start + Consts::io_size + Consts::ext_io_size && Addr >= Consts::GPRs_size) { //only io needs updates
switch (Addr) {
case 0xFFFF: //case for updating everything
case Consts::EECR: {
if ((data[Consts::EECR] & (1 << Consts::EECR_EEMPE)) && mcu->cpu.totalCycls - lastEECR_EEMPE_set > 4) { // check if EE;PE is set but shouldnt
data[Consts::EECR] &= ~(1 << Consts::EECR_EEMPE); //clear EEMPE
}
if (onlyOne) break;
}

case Consts::PLLCSR: {
if ((data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLLE)) && !(data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLOCK)) && mcu->cpu.totalCycls - lastPLLCSR_PLLE_set > PLLCSR_PLOCK_wait) { //if PLLE is 1 and PLOCK is 0 and enought time since PLLE set
data[Consts::PLLCSR] |= (1 << Consts::PLLCSR_PLOCK); //set PLOCK
}
if (onlyOne) break;
}
}
}
}

*/