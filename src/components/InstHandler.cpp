#include "InstHandler.h"

#include "StringUtils.h"
#include "DataUtils.h"
#include "../utils/bitMacros.h"

#include "../config.h"

#include "../ATmega32u4.h"
#include "InstInds.h"
#include "../extras/Disassembler.h"

#define LU_MODULE "InstHandler"

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::callInstSwitch(uint8_t ind, ATmega32u4* mcu, uint16_t word){
	switch(ind) {
		case 0:
			return INST_STS(mcu, word);
		case 1:
			return INST_LDS(mcu, word);
		case 2:
			return INST_POP(mcu, word);
		case 3:
			return INST_PUSH(mcu, word);
		case 4:
			return INST_CALL(mcu, word);
		case 5:
			return INST_LD_X(mcu, word);
		case 6:
			return INST_LD_XpostInc(mcu, word);
		case 7:
			return INST_LD_XpreDec(mcu, word);
		case 8:
			return INST_LD_YpostInc(mcu, word);
		case 9:
			return INST_LD_YpreDec(mcu, word);
		case 10:
			return INST_LD_ZpostInc(mcu, word);
		case 11:
			return INST_LD_ZpreDec(mcu, word);
		case 12:
			return INST_JMP(mcu, word);
		case 13:
			return INST_RET(mcu, word);
		case 14:
			return INST_ADIW(mcu, word);
		case 15:
			return INST_OUT(mcu, word);
		case 16:
			return INST_IN(mcu, word);
		case 17:
			return INST_ROR(mcu, word);
		case 18:
			return INST_DEC(mcu, word);
		case 19:
			return INST_MUL(mcu, word);
		case 20:
			return INST_LPM_0(mcu, word);
		case 21:
			return INST_LPM_d(mcu, word);
		case 22:
			return INST_LPM_dpostInc(mcu, word);
		case 23:
			return INST_SBIW(mcu, word);
		case 24:
			return INST_LSR(mcu, word);
		case 25:
			return INST_ST_X(mcu, word);
		case 26:
			return INST_ST_XpostInc(mcu, word);
		case 27:
			return INST_ST_XpreDec(mcu, word);
		case 28:
			return INST_ST_YpostInc(mcu, word);
		case 29:
			return INST_ST_YpreDec(mcu, word);
		case 30:
			return INST_ST_ZpostInc(mcu, word);
		case 31:
			return INST_ST_ZpreDec(mcu, word);
		case 32:
			return INST_SBI(mcu, word);
		case 33:
			return INST_CBI(mcu, word);
		case 34:
			return INST_COM(mcu, word);
		case 35:
			return INST_CLI(mcu, word);
		case 36:
			return INST_INC(mcu, word);
		case 37:
			return INST_NEG(mcu, word);
		case 38:
			return INST_ASR(mcu, word);
		case 39:
			return INST_ICALL(mcu, word);
		case 40:
			return INST_SBIS(mcu, word);
		case 41:
			return INST_RETI(mcu, word);
		case 42:
			return INST_WDR(mcu, word);
		case 43:
			return INST_SLEEP(mcu, word);
		case 44:
			return INST_SBIC(mcu, word);
		case 45:
			return INST_CLT(mcu, word);
		case 46:
			return INST_SWAP(mcu, word);
		case 47:
			return INST_SEI(mcu, word);
		case 48:
			return INST_SET(mcu, word);
		case 49:
			return INST_SEC(mcu, word);
		case 50:
			return INST_IJMP(mcu, word);
		case 51:
			return INST_EIJMP(mcu, word);
		case 52:
			return INST_EICALL(mcu, word);
		case 53:
			return INST_BSET(mcu, word);
		case 54:
			return INST_BCLR(mcu, word);
		case 55:
			return INST_CLC(mcu, word);
		case 56:
			return INST_SEN(mcu, word);
		case 57:
			return INST_CLN(mcu, word);
		case 58:
			return INST_SEZ(mcu, word);
		case 59:
			return INST_CLZ(mcu, word);
		case 60:
			return INST_SES(mcu, word);
		case 61:
			return INST_CLS(mcu, word);
		case 62:
			return INST_SEV(mcu, word);
		case 63:
			return INST_CLV(mcu, word);
		case 64:
			return INST_SEH(mcu, word);
		case 65:
			return INST_CLH(mcu, word);
		case 66:
			return INST_ELPM_0(mcu, word);
		case 67:
			return INST_ELPM_d(mcu, word);
		case 68:
			return INST_ELPM_dpostInc(mcu, word);
		case 69:
			return INST_SPM(mcu, word);
		case 70:
			return INST_BREAK(mcu, word);
		case 71:
			return INST_LDI(mcu, word);
		case 72:
			return INST_RJMP(mcu, word);
		case 73:
			return INST_MOVW(mcu, word);
		case 74:
			return INST_MOV(mcu, word);
		case 75:
			return INST_ADD(mcu, word);
		case 76:
			return INST_CPC(mcu, word);
		case 77:
			return INST_AND(mcu, word);
		case 78:
			return INST_EOR(mcu, word);
		case 79:
			return INST_SBC(mcu, word);
		case 80:
			return INST_OR(mcu, word);
		case 81:
			return INST_NOP(mcu, word);
		case 82:
			return INST_MULSU(mcu, word);
		case 83:
			return INST_MULS(mcu, word);
		case 84:
			return INST_FMUL(mcu, word);
		case 85:
			return INST_FMULS(mcu, word);
		case 86:
			return INST_FMULSU(mcu, word);
		case 87:
			return INST_BRBS(mcu, word);
		case 88:
			return INST_BRBC(mcu, word);
		case 89:
			return INST_SBRS(mcu, word);
		case 90:
			return INST_SBRC(mcu, word);
		case 91:
			return INST_RCALL(mcu, word);
		case 92:
			return INST_BST(mcu, word);
		case 93:
			return INST_BLD(mcu, word);
		case 94:
			return INST_ADC(mcu, word);
		case 95:
			return INST_CPI(mcu, word);
		case 96:
			return INST_CP(mcu, word);
		case 97:
			return INST_CPSE(mcu, word);
		case 98:
			return INST_SUB(mcu, word);
		case 99:
			return INST_LDD_Y(mcu, word);
		case 100:
			return INST_LDD_Z(mcu, word);
		case 101:
			return INST_LD_Y(mcu, word);
		case 102:
			return INST_LD_Z(mcu, word);
		case 103:
			return INST_STD_Y(mcu, word);
		case 104:
			return INST_STD_Z(mcu, word);
		case 105:
			return INST_ST_Y(mcu, word);
		case 106:
			return INST_ST_Z(mcu, word);
		case 107:
			return INST_SUBI(mcu, word);
		case 108:
			return INST_ANDI(mcu, word);
		case 109:
			return INST_SBCI(mcu, word);
		case 110:
			return INST_ORI(mcu, word);
	}
#if MCU_INCLUDE_EXTRAS
	mcu->debugger.halt();
#else
	abort();
#endif

	return inst_effect_t(0,0);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::callInstSwitch2(ATmega32u4* mcu, uint16_t word) {
	switch (word & 0xf000) {
		case 0b0000 << 12:
			switch (word & 0b0000110000000000) {
				case 0b00 << 10: // xxxx 00xx xxxx xxxx
					switch (word & 0b0000001100000000) {
						case 0b00 << 8:
							return INST_NOP(mcu, word);
						case 0b01 << 8:
							return INST_MOVW(mcu, word);
						case 0b10 << 8:
							return INST_MULS(mcu, word);
						case 0b11 << 8:
							switch (word & 0b10001000) {
								case 0b00000000:
									return INST_MULSU(mcu, word);
								case 0b00001000:
									return INST_FMUL(mcu, word);
								case 0b10000000:
									return INST_FMULS(mcu, word);
								case 0b10001000:
									return INST_FMULSU(mcu, word);
							}
					}

				case 0b01 << 10: // xxxx 01xx xxxx xxxx
					return INST_CPC(mcu, word);
				case 0b10 << 10:
					return INST_SBC(mcu, word);
				case 0b11 << 10:
					return INST_ADD(mcu, word);
			}
		case 0b0001 << 12:
			switch (word & 0b0000110000000000) {
				case 0b00 << 10:
					return INST_CPSE(mcu, word);
				case 0b01 << 10:
					return INST_CP(mcu, word);
				case 0b10 << 10:
					return INST_SUB(mcu, word);
				case 0b11 << 10:
					return INST_ADC(mcu, word);
			}
		case 0b0010 << 12:
			switch (word & 0b0000110000000000) {
				case 0b00 << 10:
					return INST_AND(mcu, word);
				case 0b01 << 10:
					return INST_EOR(mcu, word);
				case 0b10 << 10:
					return INST_OR(mcu, word);
				case 0b11 << 10:
					return INST_MOV(mcu, word);
			}
		case 0b0011 << 12:
			return INST_CPI(mcu, word);
		case 0b0100 << 12:
			return INST_SBCI(mcu, word);
		case 0b0101 << 12:
			return INST_SUBI(mcu, word);
		case 0b0110 << 12:
			return INST_ORI(mcu, word);
		case 0b0111 << 12:
			return INST_ANDI(mcu, word);
		case 0b1000 << 12:
		case 0b1010 << 12:  // bc there is 1 bit of parameter q included
			switch (word & 0b0000001000000000) {
				case 0b0 << 9:
					return INST_LDD_anyYZ(mcu, word);
				case 0b1 << 9:
					return INST_STD_anyYZ(mcu, word);
			}
		case 0b1001 << 12:
			switch (word & 0x0f00) {
				case 0b0000 << 8:
				case 0b0001 << 8:
					switch (word & 0x000f) {
						case 0b0000:
							return INST_LDS(mcu, word);
						case 0b0001:
							return INST_LD_ZpostInc(mcu, word);
						case 0b0010:
							return INST_LD_ZpreDec(mcu, word);
						case 0b1001:
							return INST_LD_YpostInc(mcu, word);
						case 0b1010:
							return INST_LD_YpreDec(mcu, word);
						case 0b1100:
							return INST_LD_X(mcu, word);
						case 0b1101:
							return INST_LD_XpostInc(mcu, word);
						case 0b1110:
							return INST_LD_XpreDec(mcu, word);
						case 0b0100:
							return INST_LPM_d(mcu, word);
						case 0b0101:
							return INST_LPM_dpostInc(mcu, word);
						case 0b0110:
							return INST_ELPM_d(mcu, word);
						case 0b0111:
							return INST_ELPM_dpostInc(mcu, word);
						case 0b1111:
							return INST_POP(mcu, word);
						default:
							return inst_effect_t(0,1);  // TODO
					}
				case 0b0010 << 8:
				case 0b0011 << 8:
					switch (word & 0x000f) {
						case 0b0000:
							return INST_STS(mcu, word);
						case 0b0001:
							return INST_ST_ZpostInc(mcu, word);
						case 0b0010:
							return INST_ST_ZpreDec(mcu, word);
						case 0b1001:
							return INST_ST_YpostInc(mcu, word);
						case 0b1010:
							return INST_ST_YpreDec(mcu, word);
						case 0b1100:
							return INST_ST_X(mcu, word);
						case 0b1101:
							return INST_ST_XpostInc(mcu, word);
						case 0b1110:
							return INST_ST_XpreDec(mcu, word);
						case 0b1111:
							return INST_PUSH(mcu, word);
					}
				case 0b0100 << 8:
				case 0b0101 << 8:
					switch (word & 0x000f) {
						case 0b0000:
							return INST_COM(mcu, word);
						case 0b0001:
							return INST_NEG(mcu, word);
						case 0b0010:
							return INST_SWAP(mcu, word);
						case 0b0011:
							return INST_INC(mcu, word);
						case 0b0101:
							return INST_ASR(mcu, word);
						case 0b0110:
							return INST_LSR(mcu, word);
						case 0b0111:
							return INST_ROR(mcu, word);
						case 0b1000:
							switch (word & 0b0000000100000000) {
								case 0b0 << 8:
									switch (word & 0b0000000010000000) {
										case 0b0 << 7:
											return INST_BSET(mcu, word);
										case 0b1 << 7:
											return INST_BCLR(mcu, word);
									}
								case 0b1 << 8:
									switch (word & 0x00f0) {
										case 0b0000 << 4:
											return INST_RET(mcu, word);
										case 0b0001 << 4:
											return INST_RETI(mcu, word);
										case 0b1000 << 4:
											return INST_SLEEP(mcu, word);
										case 0b1001 << 4:
											return INST_BREAK(mcu, word);
										case 0b1010 << 4:
											return INST_WDR(mcu, word);
										case 0b1100 << 4:
											return INST_LPM_0(mcu, word);
										case 0b1101 << 4:
											return INST_ELPM_0(mcu, word);
										case 0b1110 << 4:
											return INST_SPM(mcu, word);
										default:
											abort();
									}
							}
						case 0b1001:
							switch (word & 0b0000000100010000) {
								case 0b00000 << 4:
									return INST_IJMP(mcu, word);
								case 0b00001 << 4:
									return INST_EIJMP(mcu, word);
								case 0b10000 << 4:
									return INST_ICALL(mcu, word);
								case 0b10001 << 4:
									return INST_EICALL(mcu, word);
							}
						case 0b1010:
							return INST_DEC(mcu, word);
						case 0b1100:
						case 0b1101: // 1 bit of k parameter
							return INST_JMP(mcu, word);
						case 0b1110:
						case 0b1111: // 1 bit of k parameter
							return INST_CALL(mcu, word);

					}
				case 0b0110 << 8:
					return INST_ADIW(mcu, word);
				case 0b0111 << 8:
					return INST_SBIW(mcu, word);
				case 0b1000 << 8:
					return INST_CBI(mcu, word);
				case 0b1001 << 8:
					return INST_SBIC(mcu, word);
				case 0b1010 << 8:
					return INST_SBI(mcu, word);
				case 0b1011 << 8:
					return INST_SBIS(mcu, word);
				default:  // case 0b11xx << 8
					return INST_MUL(mcu, word);
			}
		case 0b1011 << 12:
			switch (word & 0b0000100000000000) {
				case 0b0 << 11:
					return INST_IN(mcu, word);
				case 0b1 << 11:
					return INST_OUT(mcu, word);
			}
		case 0b1100 << 12:
			return INST_RJMP(mcu, word);
		case 0b1101 << 12:
			return INST_RCALL(mcu, word);
		case 0b1110 << 12:
			return INST_LDI(mcu, word);
		case 0b1111 << 12:
			switch (word & 0b0000111000000000) {
				case 0b000 << 9:
				case 0b001 << 9:
					return INST_BRBS(mcu, word);
				case 0b010 << 9:
				case 0b011 << 9:
					return INST_BRBC(mcu, word);
				case 0b100 << 9:
					return INST_BLD(mcu, word);
				case 0b101 << 9:
					return INST_BST(mcu, word);
				case 0b110 << 9:
					return INST_SBRC(mcu, word);
				case 0b111 << 9:
					return INST_SBRS(mcu, word);
				default: abort();
			}
		default: abort();
	}
}

uint8_t A32u4::InstHandler::getInstInd(uint16_t word) noexcept {
	return getInstInd3(word);
}
uint8_t A32u4::InstHandler::getInstInd3(uint16_t word) noexcept {
	static constexpr uint8_t startIndArr[] = { 73, 94, 109, 107, 99, 0, 71, 87 };
	uint8_t startInd = startIndArr[(word & 0b1100000000000000) >> 13 | ((word & 0b0001000000000000) != 0)];

	for (uint8_t i = startInd; i < instListLen; i++) {
		if ((word & instList[i].mask) == instList[i].res) {
			return i;
		}
	}

	//println("Unhaldled Inst: 0x" << std::hex << word); // << " At: 0x" << mcu->cpu.PC

	return 0xff;
}

#define convTo16BitInt(_bitlen_, _word_) _convTo16BitInt(_bitlen_, _word_)
//#define convTo16BitInt(_bitlen_, _word_) _convTo16BitIntT<_bitlen_>(_word_)

int16_t _convTo16BitInt(uint8_t bitLen, uint16_t word) noexcept { // convert a signed int of arbitrary bitLength to a int16_t
	return isBitSet(word, bitLen - 1) ? (((int16_t)-1 ^ ((1 << bitLen) - 1)) | word) : word;
}

// I had it be faster when not a template??
template<uint8_t bitLen>
int16_t _convTo16BitIntT(uint16_t word) noexcept { // convert a signed int of arbitrary bitLength to a int16_t
	return isBitSet(word, bitLen - 1) ? (((int16_t)-1 ^ ((1 << bitLen) - 1)) | word) : word;
}

//Parameters
uint8_t A32u4::InstHandler::getRd2_c_arr(uint16_t word) noexcept {//Rd 2Bit continous use as arr index
	// 24, 26, 28, 30
	return 24 + ((word & 0x0030) >> 4)*2;
}
uint8_t A32u4::InstHandler::getRd3_c_a16(uint16_t word) noexcept {//Rd 3Bit continous add 16
	return ((word & 0x0070) >> 4) + 16;
}
uint8_t A32u4::InstHandler::getRr3_c_a16(uint16_t word) noexcept {//Rr 3Bit continous add 16
	return (word & 0x0007) + 16;
}
uint8_t A32u4::InstHandler::getRd4_c(uint16_t word) noexcept {//Rd 4Bit continous add 16
	return ((word & 0x00F0) >> 4);
}
uint8_t A32u4::InstHandler::getRr4_c(uint16_t word) noexcept {//Rr 4Bit continous add 16
	return (word & 0x000F);
}
uint8_t A32u4::InstHandler::getRd4_c_a16(uint16_t word) noexcept {//Rd 4Bit continous add 16
	return getRd4_c(word) + 16;
}
uint8_t A32u4::InstHandler::getRr4_c_a16(uint16_t word) noexcept {//Rr 4Bit continous add 16
	return getRr4_c(word) + 16;
}
uint8_t A32u4::InstHandler::getRd4_c_m2(uint16_t word) noexcept {//Rd 4Bit continous add 16
	return getRd4_c(word) * 2;
}
uint8_t A32u4::InstHandler::getRr4_c_m2(uint16_t word) noexcept {//Rr 4Bit continous add 16
	return getRr4_c(word) * 2;
}
uint8_t A32u4::InstHandler::getRd5_c(uint16_t word) noexcept { //Rd 5Bit continous 
	return (word & 0b0000000111110000) >> 4;
}
uint8_t A32u4::InstHandler::getRr5_c(uint16_t word) noexcept { //Rr 5Bit continous 
	return ((word & 0b0000001000000000) >> 5) | (word & 0b0000000000001111);
}
uint8_t A32u4::InstHandler::getK6_d24(uint16_t word) noexcept {//K val 8Bit distributed in 4chunk 4chunk
	return ((word & 0x00C0) >> 2) | (word & 0x000F);
}
uint8_t A32u4::InstHandler::getK8_d44(uint16_t word) noexcept {//K val 8Bit distributed in 4chunk 4chunk
	return ((word & 0x0F00) >> 4) | (word & 0x000F);
}
uint8_t A32u4::InstHandler::getk7_c_sin(uint16_t word) noexcept {//k val 12Bit continuous signed
	return (int8_t)convTo16BitInt(7, (word & 0b1111111000) >> 3);
}
int16_t A32u4::InstHandler::getk12_c_sin(uint16_t word) noexcept {//k val 12Bit continuous signed
	return convTo16BitInt(12, word & 0xFFF);
}
uint8_t A32u4::InstHandler::getb3_c(uint16_t word) noexcept {//b val 3Bit continuous
	return word & 0b111;
}
uint8_t A32u4::InstHandler::getA5_c(uint16_t word) noexcept {//A val 5Bit continuous
	return (word & 0b11111000) >> 3;
}
uint8_t A32u4::InstHandler::gets3_c(uint16_t word) noexcept {//s val 3Bit continuous
	return (word & 0b1110000) >> 4;
}
uint8_t A32u4::InstHandler::getq6_d123(uint16_t word) noexcept {//q val 6Bit distributed in 1chunk 2chunk 3chunk
	return ((word & 0x2000) >> 8) | ((word & 0x0C00) >> 7) | (word & 0x0007);
}
uint8_t A32u4::InstHandler::getA6_d24(uint16_t word) noexcept {//A val 6Bit distributed in 2chunk 4chunk ____ _AA_ ____ AAAA
	return ((word & 0x0600) >> (4 + 1)) | (word & 0x000F);
}

uint32_t A32u4::InstHandler::getLongAddr(uint16_t word1, uint16_t word2) noexcept {
	return (((uint32_t)(word1 & 0b0000000111110000)) << 13) | (((uint32_t)(word1 & 0b0000000000000001)) << 16) | word2; //could just use word2, but idk
}

bool A32u4::InstHandler::is2WordInst(uint16_t word) noexcept {
	// sts  1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	// lds  1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	// call 1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	// jmp  1001 010k kkkk 110k kkkk kkkk kkkk kkkk

	CU_IF_LIKELY((word & 0b1111100000000000) != 0b1001000000000000) {
		return false;
	}
	
	if ((word & instList[IND_LDS].mask) == instList[IND_LDS].res) {
		return true;
	}
	if ((word & instList[IND_STS].mask) == instList[IND_STS].res) {
		return true;
	}
	if ((word & instList[IND_JMP].mask) == instList[IND_JMP].res) {
		return true;
	}
	if ((word & instList[IND_CALL].mask) == instList[IND_CALL].res) {
		return true;
	}

	return false;
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::setPC_Cycs_Skip(ATmega32u4* mcu, bool cond) noexcept {
	if (cond) {
		if (!is2WordInst(mcu->flash.getInst(mcu->cpu.PC + 1))) {
			return inst_effect_t(2,2);
		}
		else {
			return inst_effect_t(3,3);
		}
	}
	else {
		return inst_effect_t(1,1);
	}
}

#define FLAG_MODULE mcu->dataspace

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ADD(ATmega32u4* mcu, uint16_t word) noexcept { //0000 11rd dddd rrrr
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t Rd_res = Rd + Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_ADD(Rd, Rr, 0, Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ADC(ATmega32u4* mcu, uint16_t word) noexcept { //0001 11rd dddd rrrr
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	uint8_t C = mcu->dataspace.sreg[DataSpace::Consts::SREG_C] != 0;

	uint8_t Rd_res = Rd + Rr + C;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_ADD(Rd, Rr, C, Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ADIW(ATmega32u4* mcu, uint16_t word) noexcept { //1001 0110 KKdd KKKK
	const uint8_t d = getRd2_c_arr(word);
	const uint8_t K = getK6_d24(word);

	const uint16_t R16 = mcu->dataspace.getWordReg(d);
	const uint16_t R16_res = R16 + K;

	mcu->dataspace.setWordReg(d, R16_res);

	FLAG_MODULE.setFlags_SVNZC_ADD_16(R16,K,R16_res);
	return inst_effect_t(2,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SUB(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	uint8_t Rd_res = Rd - Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, Rr, 0, Rd_res,false);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SUBI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(d);

	const uint8_t Rd_res = Rd - K;

	mcu->dataspace.setGPReg_(d, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, K, 0, Rd_res,false);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t C = mcu->dataspace.sreg[DataSpace::Consts::SREG_C] != 0;

	const uint8_t Rd_res = Rd - (Rr + C); //Rd = Rd - Rr - C

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, Rr, C, Rd_res, true);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBCI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(d);

	const uint8_t C = mcu->dataspace.sreg[DataSpace::Consts::SREG_C] != 0;

	const uint8_t Rd_res = Rd - (K + C); //Rd = Rd - K - C

	mcu->dataspace.setGPReg_(d, Rd_res);

	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, K, C, Rd_res,true);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBIW(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd2_c_arr(word);
	const uint8_t K = getK6_d24(word);

	const uint16_t R16 = mcu->dataspace.getWordRegRam_(d);
	//uint16_t R16_copy = R16;
	//R16 -= K;
	const uint16_t R16_res = R16 - K;
	//mcu->dataspace.setWordReg(d, R16);
	mcu->dataspace.setWordRegRam_(d, R16_res);

	//FLAG_MODULE.setFlags_SVNZC_SUB_16(R16_copy, K, R16);
	FLAG_MODULE.setFlags_SVNZC_SUB_16(R16, K, R16_res);
	return inst_effect_t(2,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_AND(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t Rd_res = Rd & Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_SVNZ(Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ANDI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(d);

	const uint8_t Rd_res = Rd & K;

	mcu->dataspace.setGPReg_(d, Rd_res);

	FLAG_MODULE.setFlags_SVNZ(Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_OR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t Rd_res = Rd | Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_SVNZ(Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ORI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(d);

	const uint8_t Rd_res = Rd | K;

	mcu->dataspace.setGPReg_(d, Rd_res);

	FLAG_MODULE.setFlags_SVNZ(Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_EOR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t Rd_res = Rd ^ Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_SVNZ(Rd_res);
	return inst_effect_t(1,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_COM(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = 0xFF - Rd;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_SVNZC(Rd_res);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_NEG(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = 0x00 - Rd;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);


	FLAG_MODULE.setFlags_NZ(Rd_res);

	uint8_t res_h = 0x00 - (Rd & 0b1111);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_H] = isBitSetNB(res_h,4); // isBitSet(Rd,3) || !isBitSet(Rd_copy,3)

	const bool V = Rd_res == 0x80;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	const bool N = (Rd_res & 0b10000000) != 0;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = Rd_res != 0;

	return inst_effect_t(1,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_INC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = Rd + 1;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_NZ(Rd_res);
	bool V = Rd_res == 0x80;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	bool N = (Rd_res & 0b10000000) != 0;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_DEC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = Rd - 1;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	FLAG_MODULE.setFlags_NZ(Rd_res);
	bool V = (Rd_res ^ 0b10000000) == 0xFF;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	bool N = (Rd_res & 0b10000000) != 0;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = N ^ V;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SER(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31

	mcu->dataspace.setGPReg_(d, 0xFF);

	return inst_effect_t(1,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_MUL(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint16_t res = (uint16_t)Rd * (uint16_t)Rr;
	const uint8_t R0 = (uint8_t)res;
	const uint8_t R1 = (uint8_t)(res >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);
	
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSetNB(R1, 7);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_MULS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd4_c_a16(word);
	const uint8_t Rr_id = getRr4_c_a16(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const int16_t res = (int16_t)(int8_t)Rd * (int16_t)(int8_t)Rr;
	const uint8_t R0 = (uint8_t)res;
	const uint8_t R1 = (uint8_t)(res >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSetNB(R1, 7);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_MULSU(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd3_c_a16(word);
	const uint8_t Rr_id = getRr3_c_a16(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const int16_t res = (int16_t)(int8_t)Rd * (uint16_t)Rr;
	const uint8_t R0 = (uint8_t)res;
	const uint8_t R1 = (uint8_t)((uint16_t)res >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSetNB(R1, 7);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res == 0;
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_FMUL(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd3_c_a16(word);
	const uint8_t Rr_id = getRr3_c_a16(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint16_t res = (uint16_t)Rd * (uint16_t)Rr;
	const int16_t res_sh = res << 1;
	const uint8_t R0 = (uint8_t)res_sh;
	const uint8_t R1 = (uint8_t)((uint16_t)res_sh >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSet(res, 15);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res_sh == 0;
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_FMULS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd3_c_a16(word);
	const uint8_t Rr_id = getRr3_c_a16(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const int16_t res = (int16_t)(int8_t)Rd * (int16_t)(int8_t)Rr;
	const int16_t res_sh = res << 1;
	const uint8_t R0 = (uint8_t)res_sh;
	const uint8_t R1 = (uint8_t)(res_sh >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSet(res, 15);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res_sh == 0;
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_FMULSU(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd3_c_a16(word);
	const uint8_t Rr_id = getRr3_c_a16(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const int16_t res = (int16_t)(int8_t)Rd * (uint16_t)Rr;
	const int16_t res_sh = res << 1;
	const uint8_t R0 = (uint8_t)res_sh;
	const uint8_t R1 = (uint8_t)((uint16_t)res_sh >> 8);

	mcu->dataspace.setGPReg_(0, R0);
	mcu->dataspace.setGPReg_(1, R1);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = isBitSet(res, 15);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = res_sh == 0;
	return inst_effect_t(2,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_RJMP(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(mcu);
	const int16_t k = getk12_c_sin(word);
	return inst_effect_t(2,k+1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_IJMP(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->cpu.PC = mcu->dataspace.getZ();
	return inst_effect_t(2,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_EIJMP(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->cpu.PC = mcu->dataspace.getZ(); //PC should have bits 21:16 set to reg EIND but PC is only 16bit and EIND doesnt seem to exist, soooo
	return inst_effect_t(2,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_JMP(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint32_t k = getLongAddr(word,word2); 

	mcu->cpu.PC = k; 

	return inst_effect_t(3,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_RCALL(ATmega32u4* mcu, uint16_t word) noexcept {
	const int16_t k = getk12_c_sin(word);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC+1);

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.pushPCOnCallStack(mcu->cpu.PC+k+1, mcu->cpu.PC);
#endif
	
	return inst_effect_t(4,k+1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ICALL(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 1);

	const uint16_t addr = mcu->dataspace.getZ();

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.pushPCOnCallStack(addr, mcu->cpu.PC);
#endif

	mcu->cpu.PC = addr;

	return inst_effect_t(4,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_EICALL(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 1);

	const uint16_t addr = mcu->dataspace.getZ();

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.pushPCOnCallStack(addr, mcu->cpu.PC);
#endif

	mcu->cpu.PC = addr;

	return inst_effect_t(4,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CALL(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint32_t k = getLongAddr(word, word2);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 2);

#if MCU_INCLUDE_EXTRAS
	mcu->debugger.pushPCOnCallStack(k, mcu->cpu.PC);
#endif

	mcu->cpu.PC = k;

	return inst_effect_t(4,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_RET(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	const uint16_t addr = mcu->dataspace.popAddrFromStack();
	mcu->cpu.PC = addr;

	//     \/ is now handled automatically inside mcu->dataspace.popAddrFromStack();
	//mcu->debugger.popPCFromCallStack(); 

	return inst_effect_t(4,0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_RETI(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	const uint16_t addr = mcu->dataspace.popAddrFromStack();
	mcu->cpu.PC = addr;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_I] = 1;

	//mcu->debugger.popPCFromCallStack();

	mcu->cpu.insideInterrupt = false;

	return inst_effect_t(4,0);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CPSE(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	return setPC_Cycs_Skip(mcu, Rd == Rr);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CP(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	
	const uint8_t res = Rd - Rr;
	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, Rr, 0, res,false);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CPC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t C = mcu->dataspace.sreg[DataSpace::Consts::SREG_C] != 0;

	const uint8_t res = Rd - (Rr+C);
	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, Rr, C, res,true);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CPI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd = mcu->dataspace.getGPReg_(d);

	const uint8_t res = Rd - K;
	FLAG_MODULE.setFlags_HSVNZC_SUB(Rd, K, 0, res, false);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBRC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t b = getb3_c(word);

	return setPC_Cycs_Skip(mcu, (Rr & (1<<b)) == 0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBRS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t b = getb3_c(word);

	return setPC_Cycs_Skip(mcu, (Rr & (1 << b)) != 0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBIC(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	const uint8_t ioReg = mcu->dataspace.getIOAt(A);
	//uint8_t ioReg = mcu->dataspace.data[DataSpace::Consts::io_start + A];
	return setPC_Cycs_Skip(mcu, (ioReg & (1 << b)) == 0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBIS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	const uint8_t ioReg = mcu->dataspace.getIOAt(A);
	//uint8_t ioReg = mcu->dataspace.data[DataSpace::Consts::io_start + A];
	return setPC_Cycs_Skip(mcu, (ioReg & (1 << b)) != 0);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BRBS(ATmega32u4* mcu, uint16_t word) noexcept {
	const int8_t k = (int8_t)getk7_c_sin(word);
	const uint8_t s = getb3_c(word);

	if (mcu->dataspace.sreg[s]) {
		return inst_effect_t(2,k + 1);
	}else {
		return inst_effect_t(1,1);
	}
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BRBC(ATmega32u4* mcu, uint16_t word) noexcept {
	const int8_t k = (int8_t)getk7_c_sin(word);
	const uint8_t s = getb3_c(word);

	if (mcu->dataspace.sreg[s] == false) {
		return inst_effect_t(2,k + 1);
	}
	else {
		return inst_effect_t(1,1);
	}
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SBI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	uint8_t io = mcu->dataspace.getIOAt(A);
	io |= 1 << b;
	
	//mcu->dataspace.setRegBit(mcu->dataspace.data[DataSpace::Consts::io_start + A], b, true);
	mcu->dataspace.setIOAt(A, io);
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CBI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	//mcu->dataspace.setRegBit(mcu->dataspace.data[DataSpace::Consts::io_start + A], b, false);

	uint8_t io = mcu->dataspace.getIOAt(A);
	io &= ~(1 << b);

	mcu->dataspace.setIOAt(A, io);
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LSR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = Rd >> 1;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	bool C = Rd&0b1;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = C;
	bool N = 0;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N;
	bool V = N ^ C;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	bool S = N ^ V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = S;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = Rd_res == 0;
	
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ROR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = (Rd >> 1) | ((mcu->dataspace.sreg[DataSpace::Consts::SREG_C] != 0) << 7);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	bool C = Rd & 0b1;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = C;
	bool N = isBitSet(Rd_res,7);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N;
	bool V = N ^ C;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	bool S = N ^ V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = S;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = Rd_res == 0;

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ASR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = (int8_t)Rd >> 1; //hopyfully this works
	//Rd |= Rd_copy & 0b10000000;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	bool C = Rd & 0b1;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = C;
	bool N = isBitSet(Rd_res, 7);
	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = N;
	bool V = N ^ C;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = V;
	bool S = N ^ V;
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = S;

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = Rd_res == 0;

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SWAP(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);

	const uint8_t Rd_res = ((Rd&0xF0)>>4) | ((Rd&0xF)<<4);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BSET(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t s = gets3_c(word);

	mcu->dataspace.sreg[s] = 1;

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BCLR(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t s = gets3_c(word);

	mcu->dataspace.sreg[s] = 0;

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BST(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t b = getb3_c(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_T] = isBitSetNB(Rd,b);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BLD(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint8_t b = getb3_c(word);

	const bool T = mcu->dataspace.sreg[DataSpace::Consts::SREG_T];

	uint8_t Rd_res;
#if 0
	if (T) {
		Rd_res = Rd | (1 << b);
	}
	else {
		Rd_res = Rd & ~(1 << b);
	}
#else
	Rd_res = Rd ^ ((-(uint8_t)T ^ Rd) & (1 << b));
#endif
	
	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(1,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEC(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLC(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_C] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEN(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLN(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_N] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEZ(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLZ(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_Z] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEI(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_I] = 1;
	mcu->cpu.breakOutOfOptimisation(); // break out of optimisation to check for execution of interrupts (Global Interrupt Enable)
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLI(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_I] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SES(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLS(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);
	
	mcu->dataspace.sreg[DataSpace::Consts::SREG_S] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEV(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLV(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_V] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SET(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_T] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLT(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_T] = 0;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SEH(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_H] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CLH(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	mcu->dataspace.sreg[DataSpace::Consts::SREG_H] = 0;
	return inst_effect_t(1,1);
}

A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SE_(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	const uint8_t s = gets3_c(word);

	mcu->dataspace.sreg[s] = 1;
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_CL_(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);

	const uint8_t s = gets3_c(word);

	mcu->dataspace.sreg[s] = 0;
	return inst_effect_t(1,1);
}


A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_MOV(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	const uint8_t Rd_res = Rr;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_MOVW(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd4_c_m2(word);
	const uint8_t Rr_id = getRr4_c_m2(word);

	mcu->dataspace.setGPReg_(Rd_id, mcu->dataspace.getGPReg_(Rr_id));
	mcu->dataspace.setGPReg_(Rd_id+1, mcu->dataspace.getGPReg_(Rr_id+1));

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LDI(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd4_c_a16(word);
	const uint8_t K = getK8_d44(word);

	const uint8_t Rd_res = K;

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_X(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getX();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_XpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getX();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setX(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_XpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getX();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setX(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_Y(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getY();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_YpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getY();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setY(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_YpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getY();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setY(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LDD_Y(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getY() + q;

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_Z(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_ZpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setZ(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LD_ZpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	mcu->dataspace.setZ(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LDD_Z(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getZ() + q;

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LDD_anyYZ(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t q = getq6_d123(word);
	const bool isYNotZ = word & 0b1000;
	const uint16_t Addr = (isYNotZ ? mcu->dataspace.getY() : mcu->dataspace.getZ()) + q;

	const uint8_t Rd_res = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LDS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint8_t Rd_id = getRd5_c(word);
	const uint16_t k = word2;

	const uint8_t Rd_res = mcu->dataspace.getByteAt(k);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,2);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_X(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();
	
	mcu->dataspace.setByteAt(Addr, Rr);
	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_XpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();

	mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setX(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_XpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();

	mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setX(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_Y(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	mcu->dataspace.setByteAt(Addr, Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_YpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setY(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_YpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setY(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_STD_Y(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getY() + q;

	mcu->dataspace.setByteAt(Addr, Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_Z(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	mcu->dataspace.setByteAt(Addr, Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_ZpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setZ(Addr + 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ST_ZpreDec(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setZ(Addr - 1);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_STD_Z(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getZ() + q;

	mcu->dataspace.setByteAt(Addr, Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_STD_anyYZ(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t q = getq6_d123(word);
	const bool isYNotZ = word & 0b1000;
	const uint16_t Addr = (isYNotZ ? mcu->dataspace.getY() : mcu->dataspace.getZ()) + q;

	mcu->dataspace.setByteAt(Addr, Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_STS(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rd = mcu->dataspace.getGPReg_(Rd_id);
	const uint16_t k = word2;

	mcu->dataspace.setByteAt(k, Rd);

	return inst_effect_t(2,2);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LPM_0(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t R0 = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(0, R0);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LPM_d(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t Rr_res = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(Rr_id, Rr_res);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_LPM_dpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint16_t Addr = mcu->dataspace.getZ();

	const uint8_t Rr_res = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(Rr_id, Rr_res);

	mcu->dataspace.setZ(Addr + 1);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ELPM_0(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);
	const uint32_t Addr = mcu->dataspace.getExtendedZ();

	const uint8_t R0 = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(0, R0);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ELPM_d(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint32_t Addr = mcu->dataspace.getExtendedZ();

	const uint8_t Rr_res = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(Rr_id, Rr_res);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_ELPM_dpostInc(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint32_t Addr = mcu->dataspace.getExtendedZ();

	const uint8_t Rr_res = mcu->flash.getByte(Addr);

	mcu->dataspace.setGPReg_(Rr_id, Rr_res);

	mcu->dataspace.setZ(Addr + 1);

	return inst_effect_t(3,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SPM(ATmega32u4* mcu, uint16_t word) noexcept {
	//TODO
	CU_UNUSED(mcu);
	CU_UNUSED(word);

	return inst_effect_t(0,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_IN(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t A = getA6_d24(word);

	const uint8_t Rd_res = mcu->dataspace.getIOAt(A);

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_OUT(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);
	const uint8_t A = getA6_d24(word);

	mcu->dataspace.setIOAt(A, Rr);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_PUSH(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t Rr = mcu->dataspace.getGPReg_(Rr_id);

	mcu->dataspace.pushByteToStack(Rr);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_POP(ATmega32u4* mcu, uint16_t word) noexcept {
	const uint8_t Rd_id = getRd5_c(word);

	const uint8_t Rd_res = mcu->dataspace.popByteFromStack();

	mcu->dataspace.setGPReg_(Rd_id, Rd_res);

	return inst_effect_t(2,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_NOP(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(mcu);
	CU_UNUSED(word);
	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_SLEEP(ATmega32u4* mcu, uint16_t word) noexcept {
	CU_UNUSED(word);
	
	//TODO
	uint8_t SMCR_val = mcu->dataspace.getByteAt(DataSpace::Consts::SMCR);
	if (SMCR_val & 0b1) { //if SE (sleep enable) is set
		switch ((SMCR_val >> 1) & 0b111) {
		case 0: //idle
			mcu->cpu.CPU_sleep = true;
			mcu->cpu.breakOutOfOptimisation();
			break;
		default:
			break;
		}
	}
	

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_WDR(ATmega32u4* mcu, uint16_t word) noexcept {
	//TODO
	CU_UNUSED(mcu);
	CU_UNUSED(word);

	return inst_effect_t(1,1);
}
A32u4::InstHandler::inst_effect_t A32u4::InstHandler::INST_BREAK(ATmega32u4* mcu, uint16_t word) noexcept {
	//TODO
	CU_UNUSED(word);
#if MCU_INCLUDE_EXTRAS
	mcu->debugger.halt();
#endif

	return inst_effect_t(1,1);
}
