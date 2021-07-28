#include "DataSpace.h"

#include <iostream>

#include "../ATmega32u4.h"
#include "Debugger.h"
#include "../utils/stringExtras.h"

#include "../utils/bitMacros.h"

A32u4::DataSpace::DataSpace(ATmega32u4* mcu) :
#if USE_HEAP
	data(new uint8_t[Consts::data_size]), eeprom(new uint8_t[Consts::eeprom_size]),
#endif
	mcu(mcu), funcs(mcu), timers(mcu)
{

}

A32u4::DataSpace::~DataSpace() {
#if USE_HEAP
	delete[] data;
	delete[] eeprom;
#endif
}

A32u4::DataSpace::Updates::Updates(ATmega32u4* mcu) : mcu(mcu), 
REF_EECR(mcu->dataspace.data[Consts::EECR]), REF_PLLCSR(mcu->dataspace.data[Consts::PLLCSR]),
REF_PORTB(mcu->dataspace.data[Consts::PORTB]), REF_SPDR(mcu->dataspace.data[Consts::SPDR]){

}
A32u4::DataSpace::Timers::Timers(ATmega32u4* mcu) : mcu(mcu), lastCounter(0), 
REF_TCCR0B(mcu->dataspace.data[A32u4::DataSpace::Consts::TCCR0B]), 
REF_TIFR0(mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0])
{

}

void A32u4::DataSpace::Timers::update() {

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

	//uint8_t& REF_TIFR0 = mcu->dataspace.getByteRefAtAddr(mcu->dataspace.TIFR0);

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
	}
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
	}
}

void A32u4::DataSpace::Timers::checkForIntr() {
	if (REF_TIFR0 & (1 << DataSpace::Consts::TIFR0_TOV0)) {
		//std::cout << mcu->cpu.totalCycls << std::endl;
		if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TIMSK0) & (1 << DataSpace::Consts::TIMSK0_TOIE0)) {
			if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG) & (1 << DataSpace::Consts::SREG_I)) {
				//mcu->cpu.queueInterrupt(23); // 0x2E timer0 overflow interrupt vector
				REF_TIFR0 &= ~(1 << DataSpace::Consts::TIFR0_TOV0);
				mcu->cpu.directExecuteInterrupt(23);
			}
		}
	}
}

uint8_t& A32u4::DataSpace::getByteRefAtAddr(uint16_t addr) {
#if RANGE_CHECK
	if (addr < DataSpace::Consts::data_size) {
		return data[addr];
	}
	mcu->log("unhandled DataSpace address: "+std::to_string(addr));
	return errorIndicator;
#else
	return data[addr];
#endif
}
uint8_t& A32u4::DataSpace::getGPRegRef(uint8_t ind) {
#if RANGE_CHECK
	if (ind < DataSpace::Consts::GPRs_size) {

		return data[ind];
	}
	mcu->log("General Purpouse Register Index out of bounds: " + std::to_string((int)ind));
	return errorIndicator;
#else
	return data[ind];
#endif
}

uint8_t A32u4::DataSpace::getByteAt(uint16_t addr) {
#if RANGE_CHECK
	if (addr >= DataSpace::Consts::data_size) {
		mcu->log("Data get Index out of bounds" + std::to_string(addr) + " => 0x" + stringExtras::intToHex(addr,4));
		return 0;
	}
#endif

	funcs.update_Get(addr, true);

	return data[addr];
}
uint8_t A32u4::DataSpace::setByteAt(uint16_t addr, uint8_t val) {
#if RANGE_CHECK
	if (addr >= DataSpace::Consts::data_size) {
		mcu->log("Data set Index out of bounds" + std::to_string(addr) + " => 0x" + stringExtras::intToHex(addr, 4));
		return 0;
	}
#endif
	uint8_t oldVal = data[addr];
	data[addr] = val;
	return funcs.update_Set(addr, val, oldVal);
}
uint8_t A32u4::DataSpace::getIOAt(uint8_t ind) {
	return getByteAt(ind + Consts::io_start);
}
uint8_t A32u4::DataSpace::setIOAt(uint8_t ind, uint8_t val) {
	return setByteAt(ind + Consts::io_start, val);
}

uint8_t A32u4::DataSpace::getRegBit(uint16_t id, uint8_t bit) {
#if RANGE_CHECK
	if (id < 0 || id >= Consts::data_size) {
		abort();
	}
#endif
	return (getByteRefAtAddr(id) & (1 << bit)) != 0;
}
void A32u4::DataSpace::setRegBit(uint16_t id, uint8_t bit, bool val) {
#if RANGE_CHECK
	if (id < 0 || id >= Consts::data_size) {
		abort();
	}
#endif
	uint8_t& byte = getByteRefAtAddr(id);;
	if (val) {
		byte |= 1 << bit;
	}
	else {
		byte &= ~(1 << bit);
	}
}

uint16_t A32u4::DataSpace::getWordReg(uint8_t id) {
#if RANGE_CHECK
	if (id < 0 || (id + 1) >= Consts::data_size) {
		abort();
	}
#endif
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
void A32u4::DataSpace::setWordReg(uint8_t id, uint16_t val) {
#if RANGE_CHECK
	if (id < 0 || (id + 1) >= Consts::data_size) {
		abort();
	}
#endif
	data[id + 1] = val >> 8;
	data[id] = (uint8_t)val;
}

uint16_t A32u4::DataSpace::getX() {
	return ((uint16_t)data[0x1b] << 8) | data[0x1a];
}
uint16_t A32u4::DataSpace::getY() {
	return ((uint16_t)data[0x1d] << 8) | data[0x1c];
}
uint16_t A32u4::DataSpace::getZ() {
	return ((uint16_t)data[0x1f] << 8) | data[0x1e];
}
void A32u4::DataSpace::setX(uint16_t word) {
	data[0x1b] = (word & 0xFF00) >> 8;
	data[0x1a] = word & 0xFF;
}
void A32u4::DataSpace::setY(uint16_t word) {
	data[0x1d] = (word & 0xFF00) >> 8;
	data[0x1c] = word & 0xFF;
}
void A32u4::DataSpace::setZ(uint16_t word) {
	data[0x1f] = (word & 0xFF00) >> 8;
	data[0x1e] = word & 0xFF;
}

void A32u4::DataSpace::resetIO() {
	//add: set all IO Registers to initial Values
	for (uint8_t i = 0; i < Consts::io_size; i++) {
		data[Consts::io_start + i] = 0;
	}
	setWordReg(Consts::SPL, Consts::ISRAM_start + Consts::ISRAM_size - 1);
}

void A32u4::DataSpace::setSP(uint16_t val) {
	setWordReg(Consts::SPL, val);
}

void A32u4::DataSpace::Updates::update_Get(uint16_t Addr, bool onlyOne) {
	if (Addr <= Consts::io_start + Consts::io_size + Consts::ext_io_size && Addr >= Consts::GPRs_size) { //only io needs updates
		switch (Addr) {
			case 0xFFFF: //case for updating everything
			case Consts::EECR: {
				if ((REF_EECR & (1 << Consts::EECR_EEMPE)) && mcu->cpu.totalCycls - lastEECR_EEMPE_set > 4) { // check if EE;PE is set but shouldnt
					REF_EECR &= ~(1 << Consts::EECR_EEMPE); //clear EEMPE
				}
				if (onlyOne) break;
			}

			case Consts::PLLCSR: {
				if ((REF_PLLCSR & (1 << Consts::PLLCSR_PLLE)) && !(REF_PLLCSR & (1 << Consts::PLLCSR_PLOCK)) && mcu->cpu.totalCycls - lastPLLCSR_PLLE_set > PLLCSR_PLOCK_wait) { //if PLLE is 1 and PLOCK is 0 and enought time since PLLE set
					REF_PLLCSR |= (1 << Consts::PLLCSR_PLOCK); //set PLOCK
				}
				if (onlyOne) break;
			}
		}
	}
}
uint8_t A32u4::DataSpace::Updates::update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal) {
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
				mcu->dataspace.timers.checkForIntr();
				break;
		}
	}
	return 0;
}

uint8_t A32u4::DataSpace::Updates::setEECR(uint8_t val, uint8_t oldVal){
	if (val & (1 << Consts::EECR_EERE)) {
		mcu->dataspace.data[Consts::EEDR] = mcu->dataspace.eeprom[mcu->dataspace.getWordReg(Consts::EEARL)];
		val = val & ~(1 << Consts::EECR_EERE); //idk if this should be done bc its not stated anywhere but its the only logical thing
		return 4;
	}

	if ((val & (1 << Consts::EECR_EEMPE)) && (oldVal & (1 << Consts::EECR_EEMPE))) {
		lastEECR_EEMPE_set = mcu->cpu.totalCycls;
		return 0;
	}

	if (val & (1 << Consts::EECR_EEPE)) {
		if (REF_EECR & (1 << Consts::EECR_EEMPE)) {
			uint8_t mode = mcu->dataspace.data[Consts::EECR] >> 4;
			uint16_t Addr = mcu->dataspace.getWordReg(Consts::EEARL);
#if RANGE_CHECK
			if (Addr < 0 || Addr >= Consts::eeprom_size) {
				abort();
			}
#endif
			switch (mode) {
			case 0b00:
				mcu->dataspace.eeprom[Addr] = mcu->dataspace.data[Consts::EEDR];
				break;
			case 0b01:
				mcu->dataspace.eeprom[Addr] = 0;
				break;
			case 0b10:
				mcu->dataspace.eeprom[Addr] |= mcu->dataspace.data[Consts::EEDR]; //idk if it actually works like that
				break;
			}
		}
		return 0;
	}
	return 0;
}
void A32u4::DataSpace::Updates::setPLLCSR(uint8_t val, uint8_t oldVal) {
	if (val & (1 << Consts::PLLCSR_PLLE)) {
		if (!(oldVal & (1 << Consts::PLLCSR_PLLE))) { //if PLLE is 0 but should be 1
			lastPLLCSR_PLLE_set = mcu->cpu.totalCycls;
		}
	}
	else {
		if (oldVal & (1 << Consts::PLLCSR_PLLE)) { //if PLLE is 1 but should be 0
			REF_PLLCSR &= ~(1 << Consts::PLLCSR_PLOCK); //clear PLOCK
		}
	}
}
void A32u4::DataSpace::Updates::setSPDR() {
	if(SPI_Byte_Callback)
		SPI_Byte_Callback(REF_SPDR);

	for (uint8_t i = 0; i < 8; i++) {
		//Set SCK High
		//Set MOSI to REF_SPDR&0b1
		REF_SPDR >>= 1;
		if (SCK_Callback != NULL) {
			SCK_Callback();
		}
		//Set SCK LOW
	}
	mcu->dataspace.data[Consts::SPSR] |= (1 << Consts::SPSR_SPIF);

	//request Interrupt if SPIE set
}
void A32u4::DataSpace::Updates::setTCCR0B(uint8_t val) {
	mcu->dataspace.timers.timer0_presc_cache = val & 0b111;
	mcu->cpu.breakOutOfOptim = true;

	std::cout << "switch to " << ((int)val & 0b111) << std::endl;

#if 1
	switch (mcu->dataspace.timers.timer0_presc_cache) {
	case 2:
		mcu->dataspace.timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 8;
		break;
	case 3:
		mcu->dataspace.timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 64;
		break;
	case 4:
		mcu->dataspace.timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 256;
		break;
	case 5:
		mcu->dataspace.timers.lastCounter = (uint32_t)mcu->cpu.totalCycls / 1024;
		break;
	}
#endif
}

void A32u4::DataSpace::pushByteToStack(uint8_t val) {
	uint16_t SP = getWordReg(Consts::SPL);
#if RANGE_CHECK
	if (SP < Consts::ISRAM_start || SP > Consts::data_size) {
		abort();
	}
#endif
	data[SP] = val;
	setSP(SP - 1);
}
uint8_t A32u4::DataSpace::popByteFromStack() {
	uint16_t SP = getWordReg(Consts::SPL);
#if RANGE_CHECK
	if ((SP+1) < Consts::ISRAM_start || (SP+1) > Consts::data_size) {
		abort();
	}
#endif
	uint8_t Byte = data[SP + 1];

	mcu->debugger.clearAddressByte(SP + 1);

	setSP(SP + 1);
	return Byte;
}

void A32u4::DataSpace::pushAddrToStack(uint16_t Addr) {
	uint16_t SP = getWordReg(Consts::SPL);
#if RANGE_CHECK
	if (SP < Consts::ISRAM_start || SP > Consts::data_size) {
		abort();
	}
	if ((SP-1) < Consts::ISRAM_start || (SP-1) > Consts::data_size) {
		abort();
	}
#endif
	data[SP] = (uint8_t)Addr; //maybe this should be SP-1 and SP-2
	data[SP - 1] = (uint8_t)(Addr >> 8);

	mcu->debugger.registerAddressBytes(SP);

	setSP(SP - 2);
}
uint16_t A32u4::DataSpace::popAddrFromStack() {
	uint16_t SP = getWordReg(Consts::SPL);
#if RANGE_CHECK
	if ((SP+2) < Consts::ISRAM_start || (SP+2) > Consts::data_size) {
		abort();
	}
	if ((SP+1) < Consts::ISRAM_start || (SP+1) > Consts::data_size) {
		abort();
	}
#endif
	uint16_t Addr = data[SP + 2];//maybe this should be SP-1 and SP-2 
	Addr |= ((uint16_t)data[SP + 1]) << 8;

	mcu->debugger.clearAddressByteRaw(SP+2);
	mcu->debugger.clearAddressByteRaw(SP+1);

	setSP(SP + 2);
	return Addr;
}

void A32u4::DataSpace::setSPIByteCallB(SPIByteCallB func) {
	funcs.SPI_Byte_Callback = func;
}
uint8_t* A32u4::DataSpace::getEEPROM() {
	return eeprom;
}
const uint8_t* A32u4::DataSpace::getData() {
	static uint64_t lastCycs = 0;
	if (lastCycs != mcu->cpu.totalCycls) {
		lastCycs = mcu->cpu.totalCycls;
		funcs.update_Get(0xFFFF, false);
	}
	return data;
}
const uint8_t A32u4::DataSpace::getDataByte(uint16_t Addr) {
	return getByteAt(Addr);
}
void A32u4::DataSpace::setDataByte(uint16_t Addr, uint8_t byte) {
	setByteAt(Addr, byte);
}

void A32u4::DataSpace::setBitTo(uint16_t Addr, uint8_t bit, bool val) {
	uint8_t byte = getData()[Addr];
	if (val)
		byte |= 1 << bit;
	else
		byte &= ~(1 << bit);
	setDataByte(Addr, byte);
}

void A32u4::DataSpace::setBitsTo(uint16_t Addr, uint8_t mask, uint8_t bits) {
	uint8_t byte = getData()[Addr];
	byte = (byte & ~mask) | bits;
	setDataByte(Addr, byte);
}