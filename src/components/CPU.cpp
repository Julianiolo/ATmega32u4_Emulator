#include "CPU.h"

#include "../utils/bitMacros.h"
#include "StreamUtils.h"

#include "../ATmega32u4.h"
#include "CPUTemplates.h"

#define FAST_FLAGSET 1
#define fastBitSet(cond,val,mask) (-(cond) ^ (val)) & (mask)

A32u4::CPU::CPU(ATmega32u4* mcu_) : mcu(mcu_), 
PC(0), totalCycls(0), targetCycs(0),
interruptFlags(0), insideInterrupt(false) {

}

void A32u4::CPU::reset() {
	PC = 0; //add: check for reset Vector beeing moved

	insideInterrupt = false;
	interruptFlags = 0;

	CPU_sleep = false;
	sleepCycsLeft = 0;

	totalCycls = 0;
	targetCycs = 0;
	breakOutOfOptim = false;
}

void A32u4::CPU::queueInterrupt(uint16_t addr) {
	interruptFlags |= ((uint64_t)1 << addr);
}
void A32u4::CPU::executeInterrupts() {
	if (interruptFlags) {
		if (!(mcu->dataspace.sreg[DataSpace::Consts::SREG_I])) { // cancel if global interrupt flag is not set
			return;
		}

		for(int i = 0; i<43;i++) {
			if (interruptFlags & ((uint64_t)1 << i)) {
				interruptFlags &= ~((uint64_t)1 << i);

				directExecuteInterrupt(i);
				break;
			}
		}
	}
}
void A32u4::CPU::directExecuteInterrupt(uint8_t num) {
	insideInterrupt = true;
	static uint64_t count = 0;
	count++;
	//printf("cnt: %llu\n", count);

	if (CPU_sleep) {
		CPU_sleep = false;
		totalCycls += 5;
	}

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC);

	pc_t targetPC = num*2;
	mcu->debugger.pushPCOnCallStack(targetPC, mcu->cpu.PC);

	mcu->cpu.PC = targetPC;
}

uint64_t A32u4::CPU::cycsToNextTimerInt() {
#if 0
	const uint8_t& REF_TIMER0 = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::TCNT0);
	uint8_t ticksLeftT0 = 255 - REF_TIMER0;

	uint16_t prescCycsT0 = DataSpace::Timers::presc[mcu->dataspace.timers.timer0_presc_cache];
	return (uint64_t)ticksLeftT0 * prescCycsT0 + (prescCycsT0 - (totalCycls%prescCycsT0));
#else
	uint64_t amt = -1;
	{
		uint8_t timer0 = mcu->dataspace.data[DataSpace::Consts::TCNT0];
		uint64_t nextOverflow = mcu->dataspace.lastSet.Timer0Update + (256-timer0)*mcu->dataspace.getTimer0PrescDiv();
		amt = std::min(amt, nextOverflow - totalCycls);
	}
	return amt;

#endif
}

/*

	Status Register: SREG
        info about most recently  executed arithmetic Instruction
        updated after every ALU operation
        not automatically stored and restored when enterring interrupt routine
        Bits:
            7   6   5   4   3   2   1   0   Bit
            I   T   H   S   V   N   Z   C   Name
            RW  RW  RW  RW  RW  RW  RW  RW  Read/Write
            0   0   0   0   0   0   0   0   Initial Value

                I: 7 Global Interrupt Enable
                    must be set for interrupts to be enabled
                    Individual interrupt enable is done in seperate control registers
                    if cleared no Interrupts will happen regardless of individual interrupt enable
                    cleared after hardware Interrupt, set by RETI Instruction (Return from Interrupt)
                    can also be controlled by the application with SEI, CLI
                T: 6 Bit Copy Storage
                    used by bit copy Instructions BLD (Bit LoaD) and BST (Bit STore) as source/destination for operated bit
                    bit from register file can be copied to T by BST, bit from T can be copied into register file by BLD
                H: 5 Half Carry Flag
                    indicates Half Carry in some arithmetic operations, used in BCD arithmetic
                S: 4 Sign Bit (S = N xor V)
                    always xor between N flag and V flag
                V: 3 Two's Complement Overflow Flag
                    supports twos's arithmetic complements (nrswtm)
                N: 2 Negative Flag
                    Indicates negative result in arithmetic or logic operation
                Z: 1 Zero Flag
                    Indicates zero result in arithmetic or logic operation
                C: 0 Carry Flag
                    Indicates a carry in arithmetic or logic operation

*/

#if 0
void A32u4::CPU::setFlags_NZ(uint8_t res) {
#if FAST_FLAGSET
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = res & 0b10000000;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
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
	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11111001) | val;
#endif
}
void A32u4::CPU::setFlags_NZ(uint16_t res) {
#if FAST_FLAGSET
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = res & 0b1000000000000000;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
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
	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11111001) | val;
#endif
}

void A32u4::CPU::setFlags_HSVNZC_ADD(uint8_t a, uint8_t b, uint8_t c, uint8_t res) {
#if FAST_FLAGSET
	int8_t sum8 = (int8_t)a + (int8_t)b + c;
	int16_t sum16 = (int8_t)a + (int8_t)b + c;
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = sum8 != sum16;

	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = (res & 0b10000000) != 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;

	uint16_t usum16 = a + b + c;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSet(usum16, 8);

	uint8_t usum4 = (a & 0b1111) + (b & 0b1111) + c;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_H] = isBitSetNB(usum4, 4);
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

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11000000) | val;
#endif
}
void A32u4::CPU::setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t c, uint8_t res, bool Incl_Z) {
#if FAST_FLAGSET
	int16_t res16 = (int8_t)a - (int8_t)b - c;
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = (int8_t)res != res16;

	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = (res & 0b10000000) != 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	if (!Incl_Z) {
		mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
	} else {
		mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = (res == 0) && mcu->dataspace.sreg[DataSpace::Consts::SREG_Z];
	}

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = a < (uint16_t)b + c;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_H] = (b & 0b1111) + c > (a & 0b1111);
#else
	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);

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

void A32u4::CPU::setFlags_SVNZ(uint8_t res) {
#if FAST_FLAGSET
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = 0;

	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = (res & 0b10000000) != 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
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

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100001) | val;
#endif
}
void A32u4::CPU::setFlags_SVNZC(uint8_t res) {
#if FAST_FLAGSET
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = 0;

	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = (res & 0b10000000) != 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = 1;
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

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}

void A32u4::CPU::setFlags_SVNZC_ADD_16(uint16_t a, uint16_t b, uint16_t res) {
#if FAST_FLAGSET
	uint16_t sum16 = a + b;
	uint32_t sum32 = a + b;
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = sum16 != sum32;

	bool R15 = isBitSet(res, 15);
	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = R15;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;

	bool ah7 = isBitSet(a, 7 + 8);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = !R15 && ah7;
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

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}
void A32u4::CPU::setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res) {
#if FAST_FLAGSET
	int16_t sub16 = (int16_t)a - (int16_t)b;
	int32_t sub32 = (int16_t)a - (int16_t)b;
	bool V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V = sub16 != sub32;

	bool R15 = isBitSet(res, 15);
	bool N;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N = R15;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = b > a;
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

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG);
	reg = (reg & 0b11100000) | val;
#endif
}
#endif

pc_t& A32u4::CPU::getPCRef() {
	return PC;
}
pc_t A32u4::CPU::getPC() const {
	return PC;
}
addrmcu_t A32u4::CPU::getPCAddr() const {
	return getPC() * 2;
}
uint64_t A32u4::CPU::getTotalCycles() const {
	return totalCycls;
}
bool A32u4::CPU::isSleeping() const {
	return CPU_sleep;
}

void A32u4::CPU::getState(std::ostream& output){
	StreamUtils::write(output, PC);
	StreamUtils::write(output, totalCycls);
	StreamUtils::write(output, targetCycs);

	StreamUtils::write(output, interruptFlags);
	StreamUtils::write(output, insideInterrupt);

	StreamUtils::write(output, breakOutOfOptim);

	StreamUtils::write(output, CPU_sleep);
	StreamUtils::write(output, sleepCycsLeft);
}
void A32u4::CPU::setState(std::istream& input){
	StreamUtils::read(input, &PC);
	StreamUtils::read(input, &totalCycls);
	StreamUtils::read(input, &targetCycs);
	
	StreamUtils::read(input, &interruptFlags);
	StreamUtils::read(input, &insideInterrupt);

	StreamUtils::read(input, &breakOutOfOptim);

	StreamUtils::read(input, &CPU_sleep);
	StreamUtils::read(input, &sleepCycsLeft);
}

bool A32u4::CPU::operator==(const CPU& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(PC) && _CMP_(totalCycls) && _CMP_(targetCycs) &&
		_CMP_(interruptFlags) && _CMP_(insideInterrupt) &&
		_CMP_(breakOutOfOptim) &&
		_CMP_(CPU_sleep) && _CMP_(sleepCycsLeft);
#undef _CMP_
}

/*
void A32u4::CPU::setFlags_HVNZC_ADD(uint8_t a, uint8_t b, uint8_t res) {
#if FAST_FLAGSET
	bool a7 = isBitSet(a, 7);
	bool b7 = isBitSet(b, 7);
	bool r7 = isBitSet(res, 7);

	bool a3 = isBitSet(a, 3);
	bool b3 = isBitSet(b, 3);
	bool r3 = isBitSet(res, 3);

	bool V = (a7 && b7 && !r7) || (!a7 && !b7 && r7); //maybe try non branching stuff
	bool N = (res & 0b10000000) != 0;
	bool Z = res == 0;
	bool C = (a7 && b7) || (b7 && !r7) || (a7 && !r7);
	bool H = (a3 && b3) || (b3 && !r3) || (a3 && !r3);

	uint8_t val = 0;
	if (V) {
		val |= 1 << mcu->dataspace.SREG_V;
	}
	if (N) {
		val |= 1 << mcu->dataspace.SREG_N;
	}
	if (Z) {
		val |= 1 << mcu->dataspace.SREG_Z;
	}
	if (C) {
		val |= 1 << mcu->dataspace.SREG_C;
	}
	if (H) {
		val |= 1 << mcu->dataspace.SREG_H;
	}

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(mcu->dataspace.SREG);
	reg = reg & 0b11010000 | val;
#else
	setFlags_NZ(res);
	bool a7 = isBitSet(a, 7);
	bool b7 = isBitSet(b, 7);
	bool r7 = isBitSet(res, 7);

	bool a3 = isBitSet(a, 3);
	bool b3 = isBitSet(b, 3);
	bool r3 = isBitSet(res, 3);

	bool V = (a7 && b7 && !r7) || (!a7 && !b7 && r7);
	bool N = (res & 0b10000000) != 0;
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_V, V);
	//mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_S, V ^ N); //could be that this should not be set
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_C, (a7 && b7) || (b7 && !r7) || (a7 && !r7));

	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_H, (a3 && b3) || (b3 && !r3) || (a3 && !r3));
#endif
}

void A32u4::CPU::setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t res, bool Incl_Z) {
#if FAST_FLAGSET
	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(mcu->dataspace.SREG);
	bool a7 = isBitSet(a, 7);
	bool b7 = isBitSet(b, 7);
	bool r7 = isBitSet(res, 7);

	bool a3 = isBitSet(a, 3);
	bool b3 = isBitSet(b, 3);
	bool r3 = isBitSet(res, 3);

	bool V = (a7 && !b7 && !r7) || (!a7 && b7 && r7);
	bool N = (res & 0b10000000) != 0;
	bool Z;
	if (!Incl_Z) {
		Z = res == 0;
	} else {
		Z = (res == 0) && (reg &  (1 << mcu->dataspace.SREG_Z));
	}
	bool C = (!a7 && b7) || (b7 && r7) || (r7 && !a7);//(a7 && b7) || (b7 && !r7) || (a7 && !r7);
	bool H = (!a3 && b3) || (b3 && r3) || (r3 && !a3);//(a3 && b3) || (b3 && !r3) || (a3 && !r3);

	uint8_t val = 0;
	if (V) {
		val |= 1 << mcu->dataspace.SREG_V;
	}
	if (N) {
		val |= 1 << mcu->dataspace.SREG_N;
	}
	if (Z) {
		val |= 1 << mcu->dataspace.SREG_Z;
	}
	if (C) {
		val |= 1 << mcu->dataspace.SREG_C;
	}
	if (H) {
		val |= 1 << mcu->dataspace.SREG_H;
	}

	reg = (reg & 0b11010000) | val;

#else

	setFlags_NZ(res);
	bool a7 = isBitSet(a, 7);
	bool b7 = isBitSet(b, 7);
	bool r7 = isBitSet(res, 7);

	bool a3 = isBitSet(a, 3);
	bool b3 = isBitSet(b, 3);
	bool r3 = isBitSet(res, 3);

	bool V = (a7 && !b7 && !r7) || (!a7 && b7 && r7);
	bool N = (res & 0b10000000) != 0;
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_V, V);
	//mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_S, V ^ N); //could be that this should not be set
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_C, (!a7 && b7) || (b7 && r7) || (r7 && !a7));

	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_H, (!a3 && b3) || (b3 && r3) || (r3 && !a3));

	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_N, N);
	if (Incl_Z) {
		mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_Z, (res == 0) && mcu->dataspace.getRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_Z));
	}
	else {
		mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_Z, res == 0);
	}
#endif
}


void A32u4::CPU::setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res) {
#if FAST_FLAGSET
	bool ah7 = isBitSet(a, 7 + 8); //bit 7 of high byte of a word
	bool R15 = isBitSet(res, 15);

	bool V = ah7 && R15;
	bool N = R15;
	bool Z = res == 0;
	bool S = V ^ N;
	bool C = R15 && !ah7;

	uint8_t val = 0;

#if 1
	val |= V << mcu->dataspace.SREG_V;
	val |= N << mcu->dataspace.SREG_N;
	val |= Z << mcu->dataspace.SREG_Z;
	val |= S << mcu->dataspace.SREG_S;
	val |= C << mcu->dataspace.SREG_C;

#else
	if (V) {
		val |= 1 << mcu->dataspace.SREG_V;
	}
	if (N) {
		val |= 1 << mcu->dataspace.SREG_N;
	}
	if (Z) {
		val |= 1 << mcu->dataspace.SREG_Z;
	}
	if (S) {
		val |= 1 << mcu->dataspace.SREG_S;
	}
	if (C) {
		val |= 1 << mcu->dataspace.SREG_C;
	}
#endif

	uint8_t& reg = mcu->dataspace.getByteRefAtAddr(mcu->dataspace.SREG);
	reg = reg & 0b11100000 | val;
#else
	setFlags_NZ(res);

	bool ah7 = isBitSet(a, 7 + 8); //bit 7 of high byte of a word
	bool R15 = isBitSet(res, 15);

	bool V = ah7 && R15;
	bool N = R15;
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_V, V);
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_S, V ^ N);
	mcu->dataspace.setRegBit(mcu->dataspace.SREG, mcu->dataspace.SREG_C, R15 && !ah7);
#endif
}
*/