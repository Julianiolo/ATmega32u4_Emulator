#include "DataSpace.h"

#include <iostream>

#include "../utils/bitMacros.h"
#include "StreamUtils.h"
#include "DataUtils.h"

#include "../ATmega32u4.h"
#include "../extras/Debugger.h"

#define LU_MODULE "DataSpace"

void A32u4::DataSpace::LastSet::getState(std::ostream& output){
	StreamUtils::write(output, EECR_EEMPE);
	StreamUtils::write(output, PLLCSR_PLLE);
	StreamUtils::write(output, ADCSRA_ADSC);
	StreamUtils::write(output, Timer0Update);
	StreamUtils::write(output, Timer3Update);
	StreamUtils::write(output, Timer4Update);
}
void A32u4::DataSpace::LastSet::setState(std::istream& input){
	StreamUtils::read(input, &EECR_EEMPE);
	StreamUtils::read(input, &PLLCSR_PLLE);
	StreamUtils::read(input, &ADCSRA_ADSC);
	StreamUtils::read(input, &Timer0Update);
	StreamUtils::read(input, &Timer3Update);
	StreamUtils::read(input, &Timer4Update);
}

void A32u4::DataSpace::LastSet::resetAll() {
	EECR_EEMPE = 0;
	PLLCSR_PLLE = 0;
	ADCSRA_ADSC = 0;
	Timer0Update = 0;
	Timer3Update = 0;
	Timer4Update = 0;
}

bool A32u4::DataSpace::LastSet::operator==(const LastSet& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(EECR_EEMPE) && _CMP_(PLLCSR_PLLE) && _CMP_(ADCSRA_ADSC) && 
	_CMP_(Timer0Update) && _CMP_(Timer3Update) && _CMP_(Timer4Update);
#undef _CMP_
}

size_t A32u4::DataSpace::LastSet::sizeBytes() const {
	size_t sum = 0;
	sum += sizeof(EECR_EEMPE);
	sum += sizeof(PLLCSR_PLLE);
	sum += sizeof(ADCSRA_ADSC);
	sum += sizeof(Timer0Update);
	sum += sizeof(Timer3Update);
	sum += sizeof(Timer4Update);
	return sum;
}
uint32_t A32u4::DataSpace::LastSet::hash() const noexcept{
	uint32_t h = 0;
	DU_HASHC(h,EECR_EEMPE);
	DU_HASHC(h,PLLCSR_PLLE);
	DU_HASHC(h,ADCSRA_ADSC);
	DU_HASHC(h,Timer0Update);
	DU_HASHC(h,Timer3Update);
	DU_HASHC(h,Timer4Update);
	return h;
}

// ##### DataSpace #####

A32u4::DataSpace::DataSpace(ATmega32u4* mcu) : mcu(mcu), 
#if MCU_USE_HEAP
data(new uint8_t[Consts::data_size]), eeprom(new uint8_t[Consts::eeprom_size])
#endif
{
#if 1
	std::memset(data, 0, Consts::data_size);
	std::memset(eeprom, 0, Consts::eeprom_size);
#endif
}

A32u4::DataSpace::~DataSpace() {
#if MCU_USE_HEAP
	delete[] data;
	delete[] eeprom;
#endif
}

A32u4::DataSpace::DataSpace(const DataSpace& src): 
#if MCU_USE_HEAP
data(new uint8_t[Consts::data_size]), eeprom(new uint8_t[Consts::eeprom_size])
#endif
{
	operator=(src);
}
A32u4::DataSpace& A32u4::DataSpace::operator=(const DataSpace& src){
	std::memcpy(data, src.data, Consts::data_size);
	std::memcpy(eeprom, src.eeprom, Consts::eeprom_size);

	std::memcpy(sreg, src.sreg, 8);

	lastSet = src.lastSet;

	return *this;
}

void A32u4::DataSpace::reset() {
	std::memset(data, 0, Consts::data_size);
	resetIO();
	lastSet.resetAll();

	std::memset(sreg, 0, 8); // reset sreg cache
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

void A32u4::DataSpace::doTicks(uint8_t num) {
	if (isBitSetNB(data[Consts::PRR0], 5)) {
		return;
	}

	const uint8_t timer0 = data[Consts::TCNT0];
	const uint8_t timer0plus = timer0 + num;
	if (timer0plus < timer0) {
		mcu->dataspace.data[Consts::TIFR0] |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
		//printf("OVERFLOW at %llu\n", mcu->cpu.totalCycls);
	}
	data[Consts::TCNT0] = timer0plus;


	markTimer0Update();
}
void A32u4::DataSpace::updateTimers(){
	if (!isBitSetNB(data[Consts::PRR0], Consts::PRR0_PRTIM0)) {
		const uint64_t addAmt = (mcu->cpu.getTotalCycles() - lastSet.Timer0Update) / getTimer0PrescDiv();
		if(addAmt > 0) {
			const uint8_t timer0 = data[Consts::TCNT0];
			const uint8_t timer0Next = timer0 + (uint8_t)addAmt;
			if(timer0Next < timer0 || addAmt >= 256) { // overflow
				data[Consts::TIFR0] |= (1 << Consts::TIFR0_TOV0); // set TOV0 in TIFR0
			}
			data[Consts::TCNT0] = timer0Next;
			markTimer0Update();
		}
	}
	if (!isBitSetNB(data[Consts::PRR1], Consts::PRR1_PRTIM3) && (data[Consts::TIMSK3] & (1 << Consts::TIMSK3_OCIE3A))) {
		const uint64_t addAmt = (mcu->cpu.getTotalCycles() - lastSet.Timer3Update) / getTimer3PrescDiv();
		if(addAmt > 0) {
			const uint16_t timer3 = getWordRegRam_(Consts::TCNT3L);
			const uint16_t timer3Next = timer3 + (uint16_t)addAmt;
			const uint16_t target = getWordRegRam_(Consts::OCR3AL);

			bool matchesA = false;
			if(addAmt >= 0x10000) {
				matchesA = true;
			}else{
				if(timer3 <= timer3Next) {
					if(timer3 <= target && target <= timer3Next)
						matchesA = true;
				}else{
					if(timer3 <= target || target <= timer3Next)
						matchesA = true;
				}
			}

			if(matchesA) {
				data[Consts::TIFR3] |= (1 << Consts::TIFR3_OCF3A);
				Timer3ATriggered = true;
			} else{
				Timer3ATriggered = false;
			}
			setWordRegRam_(Consts::TCNT3L, timer3Next);
			markTimer3Update();
		}
	}
	if (!isBitSetNB(data[Consts::PRR1], Consts::PRR1_PRTIM4) && getTimer4Presc()) {
		const uint64_t addAmt = (mcu->cpu.getTotalCycles() - lastSet.Timer4Update) / getTimer4PrescDiv();
		if(addAmt > 0) {
			const uint16_t timer4 = getWordRegRam_(Consts::TCNT4) & 0b1111111111;
			const uint16_t timer4Next = (timer4 + (uint16_t)addAmt) & 0b1111111111;
			if(data[Consts::TIMSK4] & (1<<Consts::TIMSK4_TOIE4)) {
				if(timer4Next < timer4 || addAmt >= (1<<10)) { // overflow
					data[Consts::TIFR4] |= (1 << Consts::TIFR4_TOV4); // set TOV0 in TIFR0
				}
			}
			if(data[Consts::DDRC]) {
				const uint16_t target = data[Consts::OCR4A];
				bool matchesA = false;
				if(addAmt >= 0x10000) {
					matchesA = true;
				}else{
					if(timer4 <= timer4Next) {
						if(timer4 <= target && target <= timer4Next)
							matchesA = true;
					}else{
						if(timer4 <= target || target <= timer4Next)
							matchesA = true;
					}
				}

				uint8_t portc = data[Consts::PORTC];
				if(matchesA) {
					if(data[Consts::DDRC] & (1<<Consts::DDRC_DDC7))
						portc |= (1<<7);
					if(data[Consts::DDRC] & (1<<Consts::DDRC_DDC6))
						portc &= ~(1<<6);
				}else{
					if(data[Consts::DDRC] & (1<<Consts::DDRC_DDC7))
						portc &= ~(1<<7);
					if(data[Consts::DDRC] & (1<<Consts::DDRC_DDC6))
						portc |= (1<<6);
				}
				setByteAt(Consts::PORTC, portc);
				Timer4ATriggered = matchesA;
			}
			setWordRegRam_(Consts::TCNT4, timer4Next);
			markTimer4Update();
		}
	}
	checkForIntr();
}

void A32u4::DataSpace::checkForIntr() {
	if(!sreg[Consts::SREG_I]) // check if interrupts are disabled
		return;

	if (data[Consts::TIFR0] & (1 << Consts::TIFR0_TOV0)) {
		if (data[Consts::TIMSK0] & (1 << Consts::TIMSK0_TOIE0)) {
			//mcu->cpu.queueInterrupt(23); // 0x2E timer0 overflow interrupt vector
			data[Consts::TIFR0] &= ~(1 << Consts::TIFR0_TOV0);
			mcu->cpu.directExecuteInterrupt(23);
		}
	}

#if 0 // not needed atm
	if (data[Consts::TIFR3] & (1 << Consts::TIFR3_TOV3)) {
		if (data[Consts::TIMSK3] & (1 << Consts::TIMSK3_TOIE3)) {
			data[Consts::TIFR3] &= ~(1 << Consts::TIFR3_TOV3);
			mcu->cpu.directExecuteInterrupt();
		}
	}
#endif
	if (data[Consts::TIFR3] & (1 << Consts::TIFR3_OCF3A)) {
		if (data[Consts::TIMSK3] & (1 << Consts::TIMSK3_OCIE3A)) {
			data[Consts::TIFR3] &= ~(1 << Consts::TIFR3_OCF3A);
			mcu->cpu.directExecuteInterrupt(32);
		}
	}

	if (data[Consts::TIFR4] & (1 << Consts::TIFR4_TOV4)) {
		if (data[Consts::TIMSK4] & (1 << Consts::TIMSK4_TOIE4)) {
			data[Consts::TIFR4] &= ~(1 << Consts::TIFR4_TOV4);
			mcu->cpu.directExecuteInterrupt(41);
		}
	}
}
uint8_t A32u4::DataSpace::getTimer0Presc() const {
	return mcu->dataspace.data[DataSpace::Consts::TCCR0B] & 0b111;
}
uint8_t A32u4::DataSpace::getTimer3Presc() const {
	return mcu->dataspace.data[DataSpace::Consts::TCCR3B] & 0b111;
}
uint8_t A32u4::DataSpace::getTimer4Presc() const {
	return mcu->dataspace.data[DataSpace::Consts::TCCR4B] & 0b1111;
}
uint16_t A32u4::DataSpace::getTimer0PrescDiv() const {
	return DataSpace::timerPresc[getTimer0Presc()];
}
uint16_t A32u4::DataSpace::getTimer3PrescDiv() const {
	return DataSpace::timerPresc[getTimer3Presc()];
}
uint16_t A32u4::DataSpace::getTimer4PrescDiv() const {
	return 1 << (getTimer4Presc()-1);
}
void A32u4::DataSpace::markTimer0Update() { 
	// functions is supposed to set lastTimer0Update to the exact technically correct value, even if we are already past that
	uint64_t diff = mcu->cpu.getTotalCycles() - lastSet.Timer0Update;
	diff = (diff / getTimer0PrescDiv()) * getTimer0PrescDiv();
	lastSet.Timer0Update += diff;
}
void A32u4::DataSpace::markTimer3Update() { 
	// functions is supposed to set lastTimer3Update to the exact technically correct value, even if we are already past that
	uint64_t diff = mcu->cpu.getTotalCycles() - lastSet.Timer3Update;
	diff = (diff / getTimer3PrescDiv()) * getTimer3PrescDiv();
	lastSet.Timer3Update += diff;
}
void A32u4::DataSpace::markTimer4Update() { 
	// functions is supposed to set lastTimer4Update to the exact technically correct value, even if we are already past that
	uint64_t diff = mcu->cpu.getTotalCycles() - lastSet.Timer4Update;
	diff = (diff / getTimer4PrescDiv()) * getTimer4PrescDiv();
	lastSet.Timer4Update += diff;
}
uint64_t A32u4::DataSpace::cycsToNextTimerInt() {
	uint64_t amt = -1;
	if(data[Consts::TIMSK0] & (1 << Consts::TIMSK0_TOIE0)){ // timer0 Overflow
		uint8_t timer0 = data[DataSpace::Consts::TCNT0];
		uint64_t nextOverflow = lastSet.Timer0Update + (256-timer0)*getTimer0PrescDiv();
		uint64_t interruptIn = nextOverflow - mcu->cpu.getTotalCycles();
		amt = std::min(amt, interruptIn);
	}

	if(data[Consts::TIMSK3] & (1 << Consts::TIMSK3_OCIE3A) && !Timer3ATriggered){ // timer3 A match
		uint16_t timer3 = getWordRegRam_(Consts::TCNT3L);
		uint16_t target = getWordRegRam_(Consts::OCR3AL);
		uint64_t nextMatch = lastSet.Timer3Update + (uint16_t)(target-timer3)*getTimer3PrescDiv();
		uint64_t interruptIn = nextMatch - mcu->cpu.getTotalCycles();
		amt = std::min(amt, interruptIn);
	}

	if(data[Consts::TIMSK4] & (1 << Consts::TIMSK4_TOIE4)){ // timer4 Overflow
		if(getTimer4Presc() > 0) {
			uint16_t timer4 = getWordRegRam_(Consts::TCNT4) & 0b1111111111;
			if(data[Consts::TIMSK4] & (1<<Consts::TIMSK4_OCIE4A)){
				uint64_t nextMatch = lastSet.Timer4Update + (uint16_t)((1<<10)-timer4)*getTimer4PrescDiv();
				uint64_t interruptIn = nextMatch - mcu->cpu.getTotalCycles();
				amt = std::min(amt, interruptIn);
			}

			if(data[Consts::DDRC] && !Timer4ATriggered) {
				const uint16_t target = data[Consts::OCR4A];
				uint64_t nextMatch = lastSet.Timer4Update + (uint16_t)(target-timer4)*getTimer4PrescDiv();
				uint64_t interruptIn = nextMatch - mcu->cpu.getTotalCycles();
				amt = std::min(amt, interruptIn);
			}
		}
	}
	return amt;
}


MCU_INLINE uint8_t& A32u4::DataSpace::getGPRegRef(regind_t ind) {
	A32U4_ASSERT_INRANGE(ind, 0, Consts::GPRs_size, return data[0], "General Purpouse Register Index out of bounds: %" PRIu8, ind);
	return data[ind];
}
MCU_INLINE uint8_t A32u4::DataSpace::getGPReg(regind_t ind) const {
	A32U4_ASSERT_INRANGE(ind, 0, Consts::GPRs_size, return 0, "General Purpouse Register Index out of bounds: %" PRIu8, ind);
	return data[ind];
}
MCU_INLINE void A32u4::DataSpace::setGPReg(regind_t ind, reg_t val) {
	A32U4_ASSERT_INRANGE(ind, 0, Consts::GPRs_size, return, "General Purpouse Register Index out of bounds: %" PRIu8, ind);
	data[ind] = val;
}

MCU_INLINE uint8_t A32u4::DataSpace::getGPReg_(regind_t ind) const {
	return data[ind];
}
MCU_INLINE void A32u4::DataSpace::setGPReg_(regind_t ind, reg_t val) {
	data[ind] = val;
}

MCU_INLINE uint16_t A32u4::DataSpace::getWordRegRam_(uint16_t id) const {
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
MCU_INLINE void A32u4::DataSpace::setWordRegRam_(uint16_t id, uint16_t val) {
	data[id + 1] = val >> 8;
	data[id] = (uint8_t)val;
}
uint8_t A32u4::DataSpace::getByteAt(uint16_t addr) {
	A32U4_ASSERT_INRANGE2(addr, 0, Consts::data_size, return 0, "Data get Index out of bounds: " MCU_ADDR_FORMAT);

	if (addr >= Consts::GPRs_size && addr <= Consts::io_start + Consts::io_size + Consts::ext_io_size) { //only io needs updates
		update_Get(addr);
	}

#if MCU_RW_RECORD
	mcu->analytics.ramRead(addr);
#endif

	return data[addr];
}
void A32u4::DataSpace::setByteAt(uint16_t addr, uint8_t val) {
	A32U4_ASSERT_INRANGE2(addr, 0, Consts::data_size, return, "Data set Index out of bounds: " MCU_ADDR_FORMAT);

	uint8_t oldVal = data[addr];
	data[addr] = val;
	if (addr <= Consts::io_start + Consts::io_size + Consts::ext_io_size && addr >= Consts::GPRs_size) { //only io needs updates
		update_Set(addr, val, oldVal);
	}

#if MCU_RW_RECORD
	mcu->analytics.ramWrite(addr);
#endif
}
uint8_t A32u4::DataSpace::getIOAt(uint8_t ind) {
	return getByteAt(ind + Consts::io_start);
}
void A32u4::DataSpace::setIOAt(uint8_t ind, uint8_t val) {
	setByteAt(ind + Consts::io_start, val);
}

uint8_t A32u4::DataSpace::getRegBit(uint16_t id, uint8_t bit) {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::data_size, return 0, "getRegBit Index out of bounds: " MCU_ADDR_FORMAT);

	return (getByteAt(id) & (1 << bit)) != 0;
}
void A32u4::DataSpace::setRegBit(uint16_t id, uint8_t bit, bool val) {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::data_size, return, "setRegBit Index out of bounds: " MCU_ADDR_FORMAT);

	uint8_t byte = getByteAt(id);
	if (val) {
		byte |= 1 << bit;
	}
	else {
		byte &= ~(1 << bit);
	}
	setByteAt(id, byte);
}

MCU_INLINE uint16_t A32u4::DataSpace::getWordReg(uint8_t id) const {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::GPRs_size-1, return 0, "getWordReg Index out of bounds: " MCU_ADDR_FORMAT);
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
MCU_INLINE void A32u4::DataSpace::setWordReg(uint8_t id, uint16_t val) {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::GPRs_size-1, return, "setWordReg Index out of bounds: " MCU_ADDR_FORMAT);
	data[id + 1] = val >> 8;
	data[id] = (uint8_t)val;
}
MCU_INLINE uint16_t A32u4::DataSpace::getWordRegRam(uint16_t id) const {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::data_size-1, return 0, "getWordRegRam Index out of bounds: " MCU_ADDR_FORMAT);
	return ((uint16_t)data[id + 1] << 8) | data[id];
}
MCU_INLINE void A32u4::DataSpace::setWordRegRam(uint16_t id, uint16_t val) {
	A32U4_ASSERT_INRANGE2(id, 0, Consts::data_size-1, return, "setWordRegRam Index out of bounds: " MCU_ADDR_FORMAT);
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

uint32_t A32u4::DataSpace::getExtendedZ() {
	return ((uint32_t)data[Consts::RAMPZ] << 16) | getZ();
}

void A32u4::DataSpace::setSP(uint16_t val) {
	setWordRegRam(Consts::SPL, val);
#if MCU_INCLUDE_EXTRAS
	if(mcu->analytics.maxSP > val)
		mcu->analytics.maxSP = val;
#endif
}

template<bool onlyOne>
void A32u4::DataSpace::update_Get_impl(uint16_t addr) {
	switch (addr) {
		case 0xFFFF: //case for updating everything
		case Consts::EECR: {
			if ((data[Consts::EECR] & (1 << Consts::EECR_EEMPE)) && mcu->cpu.getTotalCycles() - lastSet.EECR_EEMPE > 4) { // check if EE;PE is set but shouldnt
				data[Consts::EECR] &= ~(1 << Consts::EECR_EEMPE); //clear EEMPE
			}
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
		}

		case Consts::PLLCSR: {
			if ((data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLLE)) && 
				!(data[Consts::PLLCSR] & (1 << Consts::PLLCSR_PLOCK)) && 
				mcu->cpu.getTotalCycles() - lastSet.PLLCSR_PLLE > PLLCSR_PLOCK_wait) { //if PLLE is 1 and PLOCK is 0 and enough time since PLLE set
				data[Consts::PLLCSR] |= (1 << Consts::PLLCSR_PLOCK); //set PLOCK
			}
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
		}

		case Consts::TCNT0: {
			data[Consts::TCNT0] += (uint8_t)((mcu->cpu.getTotalCycles() - lastSet.Timer0Update) / DataSpace::timerPresc[getTimer0Presc()]);
			markTimer0Update();
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
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
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
		}
		
		case Consts::ADCSRA: {
			if (mcu->cpu.getTotalCycles() >= lastSet.ADCSRA_ADSC + 0) { // clear bit if conversion is done
				data[Consts::ADCSRA] &= ~(1<<Consts::ADCSRA_ADSC);
			}
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
		}
		
		case Consts::ADCH: {
			//TODO: maybe unlock changing of ADC value
			if (mcu->cpu.getTotalCycles() >= lastSet.ADCSRA_ADSC + 0) {
				if (!(data[Consts::ADCSRA] & (1 << Consts::ADMUX_ADLAR))) { // normal order => right adjusted
					data[Consts::ADCH] = getADCVal()>>8;
				}
				else { // left adjusted
					data[Consts::ADCH] = getADCVal()>>2;
				}
			}
			CU_IF_LIKELY(onlyOne) break;
			else CU_FALLTHROUGH;
		}
		case Consts::ADCL: {
			//TODO: maybe lock changing of ADC value
			if (mcu->cpu.getTotalCycles() >= lastSet.ADCSRA_ADSC + 0) {
				if (!(data[Consts::ADCSRA] & (1 << Consts::ADMUX_ADLAR))) { // normal order => right adjusted
					data[Consts::ADCL] = (uint8_t)getADCVal();
				}
				else { // left adjusted
					data[Consts::ADCL] = getADCVal() << 6;
				}
			}
			CU_IF_LIKELY(onlyOne) break;
			//else CU_FALLTHROUGH;
		}
	}
}

void A32u4::DataSpace::update_Get(uint16_t Addr) {
	update_Get_impl<true>(Addr);
}
void A32u4::DataSpace::update_Get_all() {
	update_Get_impl<false>(0xFFFF);
}

void A32u4::DataSpace::update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal) {
	switch (Addr) {
		case Consts::EECR:
			setEECR(val, oldVal);
			break;

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
			checkForIntr();
			break;

		case Consts::SREG:
			updateSREGCache();
			if((val & (1<<Consts::SREG_I)) && (data[Consts::TIFR0] & (1 << DataSpace::Consts::TIFR0_TOV0)))
				mcu->cpu.breakOutOfOptimisation(); // we need to break out of Optimisation to check if an interrupt can now occur (Global Interrupt Enable)
			break;

		case Consts::TCNT0:
			markTimer0Update();
			if (val < oldVal) {
				mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0] |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
				mcu->cpu.breakOutOfOptimisation();
			}
			//mcu->debugger.halt();
			break;

		case Consts::ADCSRA:
			if (oldVal & (1 << Consts::ADCSRA_ADSC) && !(val & (1 << Consts::ADCSRA_ADSC))) { // ADCSRA_ADSC has been set to 0
				data[Consts::ADCSRA] &= ~(1 << 1 << Consts::ADCSRA_ADSC); // clear again => should have no effect
			}
			if (!(oldVal & (1 << Consts::ADCSRA_ADSC)) && (val & (1 << Consts::ADCSRA_ADSC))) { // ADCSRA_ADSC has been set to 1 => start conversion
				if (val & (1 << Consts::ADCSRA_ADEN)) { // check if adc is enabled
					lastSet.ADCSRA_ADSC = mcu->cpu.getTotalCycles();
				}
			}
			break;

		case Consts::PORTB: pinChange(ATmega32u4::PinChange_PORTB, oldVal, val); break;
		case Consts::PORTC: pinChange(ATmega32u4::PinChange_PORTC, oldVal, val); break;
		case Consts::PORTD: pinChange(ATmega32u4::PinChange_PORTD, oldVal, val); break;
		case Consts::PORTE: pinChange(ATmega32u4::PinChange_PORTE, oldVal, val); break;
		case Consts::PORTF: pinChange(ATmega32u4::PinChange_PORTF, oldVal, val); break;
	}
}

void A32u4::DataSpace::updateSREGCache() {
	uint8_t val = data[Consts::SREG];
	for (uint8_t i = 0; i < 8; i++) {
		sreg[i] = val & (1 << i);
	}
}
void A32u4::DataSpace::updateCache() {
	updateSREGCache();
}


uint16_t A32u4::DataSpace::getADCVal() {
	return 0;
}


void A32u4::DataSpace::setEECR(uint8_t val, uint8_t oldVal){
	if (val & (1 << Consts::EECR_EERE)) {
		data[Consts::EEDR] = eeprom[getWordRegRam(Consts::EEARL)];
		val = val & ~(1 << Consts::EECR_EERE); //idk if this should be done bc its not stated anywhere but its the only logical thing
		mcu->cpu.totalCycls += 4;
	}

	if ((val & (1 << Consts::EECR_EEMPE)) && !(oldVal & (1 << Consts::EECR_EEMPE))) {
		lastSet.EECR_EEMPE = mcu->cpu.totalCycls;
	}

	if (val & (1 << Consts::EECR_EEPE)) {
		if (data[Consts::EECR] & (1 << Consts::EECR_EEMPE)) {
			uint8_t mode = data[Consts::EECR] >> 4;
			uint16_t Addr = getWordRegRam(Consts::EEARL);

			A32U4_ASSERT_INRANGE2(Addr, 0, Consts::eeprom_size, return, "Eeprom addr out of bounds" MCU_ADDR_FORMAT);

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
			lastSet.PLLCSR_PLLE = mcu->cpu.getTotalCycles();
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

	if (SCK_Callback != NULL) {
		for (uint8_t i = 0; i < 8; i++) {
			//Set SCK High
			//Set MOSI to REF_SPDR&0b1
			data[Consts::SPDR] >>= 1;
			SCK_Callback();
			//Set SCK LOW
		}
	}
	else {
		data[Consts::SPDR] = 0;
	}
	data[Consts::SPSR] |= (1 << Consts::SPSR_SPIF);

	//request Interrupt if SPIE set
	// TODO?
}
void A32u4::DataSpace::setTCCR0B(uint8_t val) {
	mcu->cpu.breakOutOfOptimisation();
	//printf("SWITCH to div:%d\n", timers.getTimer0PrescDiv());
	lastSet.Timer0Update = mcu->cpu.getTotalCycles(); // dont use mark here since it shouldnt align with previous overflows (since this is the start and there are no previous overflows)
	//printf("fmark at %llu\n", mcu->cpu.totalCycls);
}

void A32u4::DataSpace::pushByteToStack(uint8_t val) {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE2(SP, Consts::ISRAM_start, Consts::data_size, return, "Stack pointer while push Byte out of bounds: " MCU_ADDR_FORMAT);
	data[SP] = val;
	setSP(SP - 1);

#if MCU_INCLUDE_EXTRAS
#if MCU_RW_RECORD
	mcu->analytics.ramWrite(SP);
#endif
#endif
}
uint8_t A32u4::DataSpace::popByteFromStack() {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE2(SP+1, Consts::ISRAM_start, Consts::data_size, return 0, "Stack pointer while pop Byte out of bounds: " MCU_ADDR_FORMAT);
	uint8_t Byte = data[SP + 1];

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.registerStackDec(SP + 1);

#if MCU_RW_RECORD
	mcu->analytics.ramRead(SP+1);
#endif
#endif

	setSP(SP + 1);

	return Byte;
}

void A32u4::DataSpace::pushAddrToStack(addrmcu_t Addr) {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE2(SP, Consts::ISRAM_start+1, Consts::data_size, return, "Stack pointer while push Addr out of bounds: " MCU_ADDR_FORMAT);
	data[SP] = (uint8_t)Addr; //maybe this should be SP-1 and SP-2
	data[SP - 1] = (uint8_t)(Addr >> 8);

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.registerAddressBytes(SP);

#if MCU_RW_RECORD
	mcu->analytics.ramWrite(SP);
	mcu->analytics.ramWrite(SP-1);
#endif
#endif

	setSP(SP - 2);
}
addrmcu_t A32u4::DataSpace::popAddrFromStack() {
	uint16_t SP = getWordRegRam(Consts::SPL);
	A32U4_ASSERT_INRANGE2(SP+1, Consts::ISRAM_start, Consts::data_size-1, return 0, "Stack pointer while pop Addr out of bounds: " MCU_ADDR_FORMAT);
	uint16_t Addr = data[SP + 2];//maybe this should be SP-1 and SP-2 
	Addr |= ((uint16_t)data[SP + 1]) << 8;

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.registerStackDec(SP + 2);

#if MCU_RW_RECORD
	mcu->analytics.ramRead(SP + 2);
	mcu->analytics.ramRead(SP + 1);
#endif
#endif

	setSP(SP + 2);
	return Addr;
}

void A32u4::DataSpace::setSPIByteCallB(std::function<void(uint8_t)> func) {
	SPI_Byte_Callback = func;
}
uint8_t* A32u4::DataSpace::getEEPROM() {
	return eeprom;
}
// get a pointer to the updated Dataspce Data arr (only gets updated on first call if cpu.totalcycles doesnt change)
const uint8_t* A32u4::DataSpace::getData() {
	static uint64_t lastCycs = 0;
	if (lastCycs != mcu->cpu.getTotalCycles()) {
		lastCycs = mcu->cpu.getTotalCycles();
		update_Get_all();
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

void A32u4::DataSpace::pinChange(uint8_t num, reg_t oldVal, reg_t val) {
	if(mcu->pinChangeCallB) {
		mcu->pinChangeCallB(num, oldVal, val);
	}
}

#define FAST_FLAGSET 1
#if 1
void A32u4::DataSpace::setFlags_NZ(uint8_t res) {
#if FAST_FLAGSET
	sreg[DataSpace::Consts::SREG_N] = res >= 0b10000000;//res & 0b10000000;
	sreg[DataSpace::Consts::SREG_Z] = res == 0;
#else
	bool N = (res & 0b10000000) != 0;
	bool Z = res == 0;
	uint8_t val = 0;
	if (N) {
		val |= 1 << DataSpace::Consts::SREG_N;
	}
	if (Z) {
		val |= 1 << DataSpace::Consts::SREG_Z;
	}
	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11111001) | val;
#endif
}
void A32u4::DataSpace::setFlags_NZ(uint16_t res) {
#if FAST_FLAGSET
	sreg[DataSpace::Consts::SREG_N] = res >= 0b1000000000000000;//(res & 0b1000000000000000) != 0;
	sreg[DataSpace::Consts::SREG_Z] = res == 0;
#else
	bool N = (res & 0b1000000000000000) != 0;
	bool Z = res == 0;
	uint8_t val = 0;
	if (N) {
		val |= 1 << DataSpace::Consts::SREG_N;
	}
	if (Z) {
		val |= 1 << DataSpace::Consts::SREG_Z;
	}
	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11111001) | val;
#endif
}

void A32u4::DataSpace::setFlags_HSVNZC_ADD(uint8_t a, uint8_t b, uint8_t c, uint8_t res) {
#if FAST_FLAGSET
	bool V;
#if 0
	int8_t sum8 = (int8_t)a + (int8_t)b + c;
	int16_t sum16 = (int8_t)a + (int8_t)b + c;
	sreg[DataSpace::Consts::SREG_V] = V = sum8 != sum16;
#else
	// this is stolen from simavr, for testing
	// seems to be faster?
	sreg[DataSpace::Consts::SREG_V] = V = ((a & b & ~res) | (~a & ~b & res)) >> 7;
#endif

	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = res >= 0b10000000;

	sreg[DataSpace::Consts::SREG_S] = N != V;

	sreg[DataSpace::Consts::SREG_Z] = res == 0;

#if 1

	uint16_t usum16 = a + b + c;
	sreg[DataSpace::Consts::SREG_C] = 
		usum16 >> 8;     // fast
		//usum16 > 0xFF; // not as fast
		//isBitSet(usum16, 8);  // sloow

	uint8_t usum4 = (a & 0b1111) + (b & 0b1111) + c;
	sreg[DataSpace::Consts::SREG_H] = isBitSetNB(usum4, 4);
#else
	// this is stolen from simavr for testing
	// seems to be slower??
	uint8_t thing = (a & b) | (a & ~res) | (b & ~res);
	sreg[DataSpace::Consts::SREG_C] = thing & (1<<7);//isBitSet(usum16, 8);

	sreg[DataSpace::Consts::SREG_H] = thing & (1<<3);
#endif

#else
	uint8_t val = 0;
	int8_t sum8 = (int8_t)a + (int8_t)b + c;
	int16_t sum16 = (int8_t)a + (int8_t)b + c;
	bool V = sum8 != sum16;
	val |= V << DataSpace::Consts::SREG_V;
	bool N = (res & 0b10000000) != 0;
	val |= N << DataSpace::Consts::SREG_N;
	bool S = N ^ V;
	val |= S << DataSpace::Consts::SREG_S;
	bool Z = res == 0;
	val |= Z << DataSpace::Consts::SREG_Z;
	uint16_t usum16 = a + b + c; //(a&0b10000000) + (b&0b10000000) + c;
	bool C = isBitSet(usum16, 8);
	val |= C << DataSpace::Consts::SREG_C;
	uint8_t usum4 = (a & 0b1111) + (b & 0b1111) + c;
	bool H = isBitSet(usum4, 4);
	val |= H << DataSpace::Consts::SREG_H;

	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11000000) | val;
#endif
}
void A32u4::DataSpace::setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t c, uint8_t res, bool Incl_Z) {
#if FAST_FLAGSET
	bool V;
	int16_t res16 = (int8_t)a - (int8_t)b - c;
	sreg[DataSpace::Consts::SREG_V] = V = (int8_t)res != res16;

	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = res >= 0b10000000;

	sreg[DataSpace::Consts::SREG_S] = N != V;

	sreg[DataSpace::Consts::SREG_Z] = (res == 0) && (!Incl_Z || sreg[DataSpace::Consts::SREG_Z]);

	sreg[DataSpace::Consts::SREG_C] = a < (uint16_t)b + c;
	sreg[DataSpace::Consts::SREG_H] = (b & 0b1111) + c > (a & 0b1111);
#else
	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);

	int16_t res16 = (int8_t)a - (int8_t)b - c;
	bool V = (int8_t)res != res16;
	bool N = (res & 0b10000000) != 0;
	bool S = N ^ V;
	bool Z;
	if (!Incl_Z) {
		Z = res == 0;
	} else {
		Z = (res == 0) && (reg &  (1 << DataSpace::Consts::SREG_Z));
	}
	bool C = a < (uint16_t)b + c; //res > a;
	bool H = (b & 0b1111)+c > (a & 0b1111);//(a3 && b3) || (b3 && !r3) || (a3 && !r3);

	uint8_t val = 0;
	val |= V << DataSpace::Consts::SREG_V;
	val |= S << DataSpace::Consts::SREG_S;
	val |= N << DataSpace::Consts::SREG_N;
	val |= Z << DataSpace::Consts::SREG_Z;
	val |= C << DataSpace::Consts::SREG_C;
	val |= H << DataSpace::Consts::SREG_H;

	reg = (reg & 0b11000000) | val;
#endif
}

void A32u4::DataSpace::setFlags_SVNZ(uint8_t res) {
#if FAST_FLAGSET
	bool V;
	sreg[DataSpace::Consts::SREG_V] = V = 0;

	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = res >= 0b10000000;

	sreg[DataSpace::Consts::SREG_S] = N;//N != V;

	sreg[DataSpace::Consts::SREG_Z] = res == 0;
#else
	bool V = 0;
	bool N = (res & 0b10000000) != 0;
	bool Z = res == 0;
	bool S = V ^ N;

	uint8_t val = 0;
	if (V) {
		val |= 1 << DataSpace::Consts::SREG_V;
	}
	if (N) {
		val |= 1 << DataSpace::Consts::SREG_N;
	}
	if (Z) {
		val |= 1 << DataSpace::Consts::SREG_Z;
	}
	if (S) {
		val |= 1 << DataSpace::Consts::SREG_S;
	}

	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100001) | val;
#endif
}
void A32u4::DataSpace::setFlags_SVNZC(uint8_t res) {
#if FAST_FLAGSET
	bool V;
	sreg[DataSpace::Consts::SREG_V] = V = 0;

	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = res >= 0b10000000;

	sreg[DataSpace::Consts::SREG_S] = N;//N != V;

	sreg[DataSpace::Consts::SREG_Z] = res == 0;

	sreg[DataSpace::Consts::SREG_C] = 1;
#else
	bool V = 0;
	bool N = (res & 0b10000000) != 0;
	bool Z = res == 0;
	bool S = V ^ N;
	bool C = 1;

	uint8_t val = 0;
	if (V) {
		val |= 1 << DataSpace::Consts::SREG_V;
	}
	if (N) {
		val |= 1 << DataSpace::Consts::SREG_N;
	}
	if (Z) {
		val |= 1 << DataSpace::Consts::SREG_Z;
	}
	if (S) {
		val |= 1 << DataSpace::Consts::SREG_S;
	}
	if (C) {
		val |= 1 << DataSpace::Consts::SREG_C;
	}

	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}

void A32u4::DataSpace::setFlags_SVNZC_ADD_16(uint16_t a, uint16_t b, uint16_t res) {
#if FAST_FLAGSET
	uint32_t sum32 = a + b;
	bool V;
	sreg[DataSpace::Consts::SREG_V] = V = res != sum32;

	const bool R15 = 
		res >> 15;
		//res >= (1<<15);
		//isBitSet(res, 15);
	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = R15;

	sreg[DataSpace::Consts::SREG_S] = N != V;

	sreg[DataSpace::Consts::SREG_Z] = res == 0;

	//bool ah7 = isBitSet(a, 7 + 8);

	sreg[DataSpace::Consts::SREG_C] = 
		(~res & a) >> 15;
		//!R15 && ah7;
#else
	bool ah7 = isBitSet(a, 7 + 8); //bit 7 of high byte of a word
	bool R15 = isBitSet(res, 15);

	uint16_t sum16 = a + b;
	uint32_t sum32 = a + b;
	bool V = sum16 != sum32;
	//bool V = ah7 && R15;
	bool N = R15;
	bool Z = res == 0;
	bool S = V ^ N;
	bool C = !R15 && ah7;

	uint8_t val = 0;
	if (V) {
		val |= 1 << DataSpace::Consts::SREG_V;
	}
	if (N) {
		val |= 1 << DataSpace::Consts::SREG_N;
}
	if (Z) {
		val |= 1 << DataSpace::Consts::SREG_Z;
	}
	if (S) {
		val |= 1 << DataSpace::Consts::SREG_S;
	}
	if (C) {
		val |= 1 << DataSpace::Consts::SREG_C;
	}

	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}
void A32u4::DataSpace::setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res) {
#if FAST_FLAGSET
	bool V;
#if 0
	int16_t sub16 = (int16_t)a - (int16_t)b;
	int32_t sub32 = (int16_t)a - (int16_t)b;
	sreg[DataSpace::Consts::SREG_V] = V = sub16 != sub32;
#else
	sreg[DataSpace::Consts::SREG_V] = V = (a & ~res) >> 15;
#endif

	bool R15 = res >= (1<<15);//isBitSet(res, 15);
	bool N;
	sreg[DataSpace::Consts::SREG_N] = N = R15;

	sreg[DataSpace::Consts::SREG_S] = N != V;

	sreg[DataSpace::Consts::SREG_Z] = res == 0;

	sreg[DataSpace::Consts::SREG_C] = b > a;
#else
	bool R15 = isBitSet(res, 15);

	int16_t sub16 = (int16_t)a - (int16_t)b;
	int32_t sub32 = (int16_t)a - (int16_t)b;
	bool V = sub16 != sub32;
	//bool V = R15 && !ah7;// had ah7 && R15; but seems to be wrong
	bool N = R15;
	bool Z = res == 0;
	bool S = V ^ N;
	//bool S = (int16_t)a < (int16_t)b;
	bool C = b > a;//had R15 && !ah7 before but seems to be wrong

	uint8_t val = 0;

	val |= V << DataSpace::Consts::SREG_V;
	val |= N << DataSpace::Consts::SREG_N;
	val |= Z << DataSpace::Consts::SREG_Z;
	val |= S << DataSpace::Consts::SREG_S;
	val |= C << DataSpace::Consts::SREG_C;

	uint8_t& reg = getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}
#endif

void A32u4::DataSpace::loadDataFromMemory(const uint8_t* data_, size_t len) {
	std::memcpy(data, data_, std::min((size_t)Consts::data_size, len));
	updateCache();
}

void A32u4::DataSpace::getState(std::ostream& output){
	update_Get_all();

	getRamState(output);
	getEepromState(output);

	lastSet.getState(output);
#if MCU_WRITE_HASH
	StreamUtils::write(output, hash());
#endif
}
void A32u4::DataSpace::setState(std::istream& input){
	setRamState(input);
	setEepromState(input);

	lastSet.setState(input);
	A32U4_CHECK_HASH("DataSpace");
}

void A32u4::DataSpace::getRamState(std::ostream& output){
	output.write((const char*)data, Consts::data_size);
}
void A32u4::DataSpace::setRamState(std::istream& input){
	input.read((char*)data, Consts::data_size);
	updateCache();
}
void A32u4::DataSpace::getEepromState(std::ostream& output){
	output.write((const char*)eeprom, Consts::eeprom_size);
}
void A32u4::DataSpace::setEepromState(std::istream& input){
	input.read((char*)eeprom, Consts::eeprom_size);
}

bool A32u4::DataSpace::operator==(const DataSpace& other) const{
#define _CMP_(x) (x==other.x)
	return std::memcmp(data,other.data,Consts::data_size) == 0 &&
		std::memcmp(eeprom,other.eeprom,Consts::eeprom_size) == 0 &&
		std::equal(sreg,sreg+8,other.sreg,[](uint8_t a,uint8_t b){
			return (!!a) == (!!b);
		}) &&
		_CMP_(lastSet);
#undef _CMP_
}

size_t A32u4::DataSpace::sizeBytes() const {
	size_t sum = 0;

	sum += sizeof(mcu);

	sum += Consts::data_size;
	sum += Consts::eeprom_size;
#if MCU_USE_HEAP
	sum += sizeof(data);
	sum += sizeof(eeprom);
#endif

	sum += sizeof(SCK_Callback);
	sum += sizeof(SPI_Byte_Callback);

	sum += sizeof(sreg);

	sum += lastSet.sizeBytes();

	return sum;
}
uint32_t A32u4::DataSpace::hash() const noexcept{
	uint32_t h = 0;
	DU_HASHCB(h, data, Consts::data_size);
	DU_HASHCB(h, eeprom, Consts::eeprom_size);
	{
		uint8_t buf[sizeof(sreg)];
		for(size_t i = 0; i<sizeof(sreg); i++)
			buf[i] = !!sreg[i];
		DU_HASHCB(h, buf, sizeof(sreg));
	}
	DU_HASH_COMB(h, lastSet.hash());
	return h;
}


/*

#if 0
	switch (timer0Presc) {
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


void A32u4::DataSpace::Timers::update() {
	abort();
#define USE_TIM0_CACHE 1

#if 1
#if USE_TIM0_CACHE
	if (getTimer0Presc() == 3) {
#else
	if ((mcu->dataspace.data[A32u4::DataSpace::Consts::TCCR0B] & 0b111) == 3) {
#endif
		goto fast_goto_64;
	}
#endif

	bool doTick;
#if USE_TIM0_CACHE
	switch (getTimer0Presc()) {
#else
	switch (mcu->dataspace.data[A32u4::DataSpace::Consts::TCCR0B] & 0b111) {
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

	//uint8_t& mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0] = mcu->dataspace.getByteRefAtAddr(TIFR0);

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
			mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0] |= (1 << DataSpace::Consts::TIFR0_TOV0); // set TOV0 in TIFR0
			goto direct_intr;
		}
	}

	if (mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0] & (1 << DataSpace::Consts::TIFR0_TOV0)) {
		direct_intr:
		if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TIMSK0) & (1 << DataSpace::Consts::TIMSK0_TOIE0)) {
			if (mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG) & (1 << DataSpace::Consts::SREG_I)) {
				//mcu->cpu.queueInterrupt(23); // 0x2E timer0 overflow interrupt vector
				mcu->dataspace.data[A32u4::DataSpace::Consts::TIFR0] &= ~(1 << DataSpace::Consts::TIFR0_TOV0);
				mcu->cpu.directExecuteInterrupt(23);
			}
		}
	}
}
}


constexpr uint8_t A32u4::DataSpace::getByteAtC(uint16_t addr) {
A32U4_ASSERT_INRANGE2_M(addr, 0, Consts::data_size, A32U4_ADDR_ERR_STR("Data get Index out of bounds: ",addr,4), "DataSpace", return 0);

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