#ifndef _A32u4_INSTHANDLER
#define _A32u4_INSTHANDLER

#include <stdint.h>
#include <array>

#include "InstInds.h"

namespace A32u4 {
	class ATmega32u4;

	class InstHandler {
	private:
		friend class CPU;
		friend class Disassembler;
	public:
		struct inst_effect_t{
			uint8_t addToCycs;
			int16_t addToPC;

			inst_effect_t(uint8_t addToCycs, int16_t addToPC) : addToCycs(addToCycs), addToPC(addToPC) {}
		};

		template<bool debug>
		static inst_effect_t handleInstRawT(ATmega32u4* mcu, uint16_t word) noexcept;

	private:
		template<bool debug>
		static inst_effect_t handleCurrentInstT(ATmega32u4* mcu) noexcept;

		template<bool debug>
		static inst_effect_t handleInstT(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t callInstSwitch(uint8_t ind,ATmega32u4* mcu, uint16_t word);

		static uint8_t getInstInd3(uint16_t word) noexcept;

		//static void getRegsDirect2(uint16_t word, uint8_t& Rd, uint8_t& Rr);
		static uint8_t getRd2_c_arr(uint16_t word) noexcept;
		static uint8_t getRd3_c_a16(uint16_t word) noexcept;
		static uint8_t getRr3_c_a16(uint16_t word) noexcept;
		static uint8_t getRd4_c(uint16_t word) noexcept;
		static uint8_t getRr4_c(uint16_t word) noexcept;
		static uint8_t getRd4_c_a16(uint16_t word) noexcept;
		static uint8_t getRr4_c_a16(uint16_t word) noexcept;
		static uint8_t getRd4_c_m2(uint16_t word) noexcept;
		static uint8_t getRr4_c_m2(uint16_t word) noexcept;
		static uint8_t getRd5_c(uint16_t word) noexcept;
		static uint8_t getRr5_c(uint16_t word) noexcept;
		static uint8_t getK6_d24(uint16_t word) noexcept;
		static uint8_t getK8_d44(uint16_t word) noexcept;
		static uint8_t getk7_c_sin(uint16_t word) noexcept;
		static int16_t getk12_c_sin(uint16_t word) noexcept;
		static uint8_t getb3_c(uint16_t word) noexcept;
		static uint8_t getA5_c(uint16_t word) noexcept;
		static uint8_t getA6_d24(uint16_t word) noexcept;
		static uint8_t gets3_c(uint16_t word) noexcept;
		static uint8_t getq6_d123(uint16_t word) noexcept;
		//static int16_t conv12to16BitInt(uint16_t word);
		static uint32_t getLongAddr(uint16_t word1, uint16_t word2) noexcept;
		
		static inst_effect_t setPC_Cycs_Skip(ATmega32u4* mcu, bool cond) noexcept;

		//############## Arithmetic and Logic Instructions ##############
		static inst_effect_t INST_ADD(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ADC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ADIW(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_SUB(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SUBI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBCI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBIW(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_AND(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ANDI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_OR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ORI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_EOR(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_COM(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_NEG(ATmega32u4* mcu, uint16_t word) noexcept;

		//void INST_SBR(ATmega32u4* mcu, uint16_t word);
		//void INST_CBR(ATmega32u4* mcu, uint16_t word);

		static inst_effect_t INST_INC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_DEC(ATmega32u4* mcu, uint16_t word) noexcept;

		//void INST_TST(ATmega32u4* mcu, uint16_t word);
		//void INST_CLR(ATmega32u4* mcu, uint16_t word);
		static inst_effect_t INST_SER(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_MUL(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_MULS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_MULSU(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_FMUL(ATmega32u4* mcu, uint16_t word) noexcept; //i dont know if any of the FMUL instructions do what they are supposed to http://www.kjit.bme.hu/images/stories/targyak/jarmufedelzeti_rendszerek/avr_hw_multiplier.pdf
		static inst_effect_t INST_FMULS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_FMULSU(ATmega32u4* mcu, uint16_t word) noexcept;

		//############## Branch Instructions ##############
		static inst_effect_t INST_RJMP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_IJMP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_EIJMP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_JMP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_RCALL(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ICALL(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_EICALL(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CALL(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_RET(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_RETI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CPSE(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CPC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CPI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBRC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBRS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBIC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SBIS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BRBS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BRBC(ATmega32u4* mcu, uint16_t word) noexcept;
		
		//############## Bit and Bit-Test Instructions ##############
		static inst_effect_t INST_SBI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CBI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LSR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ROR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ASR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SWAP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BSET(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BCLR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BST(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BLD(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_SEC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLC(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SEN(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLN(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SEZ(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLZ(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SEI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SES(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SEV(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLV(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SET(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLT(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SEH(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CLH(ATmega32u4* mcu, uint16_t word) noexcept;

		static inst_effect_t INST_SE_(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_CL_(ATmega32u4* mcu, uint16_t word) noexcept;

		//############## Data Tranfer Instructions ##############
		static inst_effect_t INST_MOV(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_MOVW(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LDI(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_X(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_XpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_XpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_Y(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_YpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_YpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LDD_Y(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_Z(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_ZpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LD_ZpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LDD_Z(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LDS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_X(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_XpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_XpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_Y(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_YpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_YpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_STD_Y(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_Z(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_ZpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ST_ZpreDec(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_STD_Z(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_STS(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LPM_0(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LPM_d(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_LPM_dpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ELPM_0(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ELPM_d(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_ELPM_dpostInc(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SPM(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_IN(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_OUT(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_PUSH(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_POP(ATmega32u4* mcu, uint16_t word) noexcept;

		//############## MCU Control Instructions ##############
		static inst_effect_t INST_NOP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_SLEEP(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_WDR(ATmega32u4* mcu, uint16_t word) noexcept;
		static inst_effect_t INST_BREAK(ATmega32u4* mcu, uint16_t word) noexcept;

	public:
		static bool is2WordInst(uint16_t word) noexcept;
		static uint8_t getInstInd(uint16_t word) noexcept;

		struct Inst_ELEM {
			inst_effect_t (*func)(ATmega32u4* mcu, uint16_t word) noexcept;
			const uint16_t mask;
			const uint16_t res;
			const char* const name;

			const uint8_t parTypes;

			uint8_t(*par1)(uint16_t word) = nullptr;
			uint8_t(*par2)(uint16_t word) = nullptr;
#if 0
			Inst_ELEM(inst_effect_t (*func)(ATmega32u4* mcu, uint16_t word) noexcept, 
				const uint16_t mask, const uint16_t res, const char* const name,
				const uint8_t parTypes,
				uint8_t(*par1)(uint16_t word), uint8_t(*par2)(uint16_t word)) : func(func), mask(mask), res(res), name(name), parTypes(parTypes), par1(par1), par2(par2) {}
#endif
		};

		static constexpr uint8_t instListLen = 111;

		static constexpr inst_effect_t (*instOnlyList[])(ATmega32u4* mcu, uint16_t word) = {
			INST_STS,
			INST_LDS,
			INST_POP,
			INST_PUSH,
			INST_CALL,
			INST_LD_X,
			INST_LD_XpostInc,
			INST_LD_XpreDec,
			INST_LD_YpostInc,
			INST_LD_YpreDec,
			INST_LD_ZpostInc,
			INST_LD_ZpreDec,
			INST_JMP,
			INST_RET,
			INST_ADIW,
			INST_OUT,
			INST_IN,
			INST_ROR,
			INST_DEC,
			INST_MUL,
			INST_LPM_0,
			INST_LPM_d,
			INST_LPM_dpostInc,
			INST_SBIW,
			INST_LSR,
			INST_ST_X,
			INST_ST_XpostInc,
			INST_ST_XpreDec,
			INST_ST_YpostInc,
			INST_ST_YpreDec,
			INST_ST_ZpostInc,
			INST_ST_ZpreDec,
			INST_SBI,
			INST_CBI,
			INST_COM,
			INST_CLI,
			INST_INC,
			INST_NEG,
			INST_ASR,
			INST_ICALL,
			INST_SBIS,
			INST_RETI,
			INST_WDR,
			INST_SLEEP,
			INST_SBIC,
			INST_CLT,
			INST_SWAP,
			INST_SEI,
			INST_SET,
			INST_SEC,
			INST_IJMP,
			INST_EIJMP,
			INST_EICALL,
			INST_BSET,
			INST_BCLR,
			INST_CLC,
			INST_SEN,
			INST_CLN,
			INST_SEZ,
			INST_CLZ,
			INST_SES,
			INST_CLS,
			INST_SEV,
			INST_CLV,
			INST_SEH,
			INST_CLH,
			INST_ELPM_0,
			INST_ELPM_d,
			INST_ELPM_dpostInc,
			INST_SPM,
			INST_BREAK,
			INST_LDI,
			INST_RJMP,
			INST_MOVW,
			INST_MOV,
			INST_ADD,
			INST_CPC,
			INST_AND,
			INST_EOR,
			INST_SBC,
			INST_OR,
			INST_NOP,
			INST_MULSU,
			INST_MULS,
			INST_FMUL,
			INST_FMULS,
			INST_FMULSU,
			INST_BRBS,
			INST_BRBC,
			INST_SBRS,
			INST_SBRC,
			INST_RCALL,
			INST_BST,
			INST_BLD,
			INST_ADC,
			INST_CPI,
			INST_CP,
			INST_CPSE,
			INST_SUB,
			INST_LDD_Y,
			INST_LDD_Z,
			INST_LD_Y,
			INST_LD_Z,
			INST_STD_Y,
			INST_STD_Z,
			INST_ST_Y,
			INST_ST_Z,
			INST_SUBI,
			INST_ANDI,
			INST_SBCI,
			INST_ORI,
		};

		static constexpr std::array<Inst_ELEM,IND_COUNT_> instList = {
		//            func                 mask                res
		//         				            1   2   3   4       1   2   3   4
			Inst_ELEM{INST_STS          , 0b1111111000001111, 0b1001001000000000, "sts"    , 0x32,getRd5_c,0},                         //1001 001d dddd 0000 kkkk kkkk kkkk kkkk
			Inst_ELEM{INST_LDS          , 0b1111111000001111, 0b1001000000000000, "lds"    , 0x32,getRd5_c,0},                         //1001 000d dddd 0000 kkkk kkkk kkkk kkkk
			Inst_ELEM{INST_POP          , 0b1111111000001111, 0b1001000000001111, "pop"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 1111
			Inst_ELEM{INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "push"   ,  0x2,getRd5_c,0},                         //1001 001d dddd 1111
			Inst_ELEM{INST_CALL         , 0b1111111000001110, 0b1001010000001110, "call"   ,  0x3,0,0},                                  //1001 010k kkkk 111k kkkk kkkk kkkk kkkk
			Inst_ELEM{INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1100
			Inst_ELEM{INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1101
			Inst_ELEM{INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1110
			Inst_ELEM{INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1001
			Inst_ELEM{INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1010
			Inst_ELEM{INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 0001
			Inst_ELEM{INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 0010
			Inst_ELEM{INST_JMP          , 0b1111111000001110, 0b1001010000001100, "jmp"    ,  0x3,0,0},                                  //1001 010k kkkk 110k kkkk kkkk kkkk kkkk
			Inst_ELEM{INST_RET          , 0b1111111111111111, 0b1001010100001000, "ret"    ,  0x0,0,0},                                  //1001 0101 0000 1000
			Inst_ELEM{INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "adiw"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0110 KKdd KKKK
			Inst_ELEM{INST_OUT          , 0b1111100000000000, 0b1011100000000000, "out"    , 0x20,getA6_d24,getRd5_c},              //1011 1AAr rrrr AAAA
			Inst_ELEM{INST_IN           , 0b1111100000000000, 0b1011000000000000, "in"     ,  0x2,getRd5_c,getA6_d24},              //1011 0AAd dddd AAAA
			Inst_ELEM{INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ror"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0111
			Inst_ELEM{INST_DEC          , 0b1111111000001111, 0b1001010000001010, "dec"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 1010
			Inst_ELEM{INST_MUL          , 0b1111110000000000, 0b1001110000000000, "mul"    , 0x22,getRd5_c,getRr5_c},               //1001 11rd dddd rrrr
			Inst_ELEM{INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "lpm"    ,  0x0,0,0},                                  //1001 0101 1100 1000
			Inst_ELEM{INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "lpm"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 0100
			Inst_ELEM{INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "lpm"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 0101
			Inst_ELEM{INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "sbiw"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0111 KKdd KKKK
			Inst_ELEM{INST_LSR          , 0b1111111000001111, 0b1001010000000110, "lsr"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0110
			Inst_ELEM{INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1100
			Inst_ELEM{INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1101
			Inst_ELEM{INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1110
			Inst_ELEM{INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1001
			Inst_ELEM{INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1010
			Inst_ELEM{INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 0001
			Inst_ELEM{INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 0010
			Inst_ELEM{INST_SBI          , 0b1111111100000000, 0b1001101000000000, "sbi"    , 0x10,getA5_c,getb3_c},                 //1001 1010 AAAA Abbb
			Inst_ELEM{INST_CBI          , 0b1111111100000000, 0b1001100000000000, "cbi"    , 0x10,getA5_c,getb3_c},                 //1001 1000 AAAA Abbb
			Inst_ELEM{INST_COM          , 0b1111111000001111, 0b1001010000000000, "com"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0000
			Inst_ELEM{INST_CLI          , 0b1111111111111111, 0b1001010011111000, "cli"    ,  0x0,0,0},                                  //1001 0100 1111 1000
			Inst_ELEM{INST_INC          , 0b1111111000001111, 0b1001010000000011, "inc"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0011
			Inst_ELEM{INST_NEG          , 0b1111111000001111, 0b1001010000000001, "neg"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0001
			Inst_ELEM{INST_ASR          , 0b1111111000001111, 0b1001010000000101, "asr"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0101
			Inst_ELEM{INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "icall"  ,  0x0,0,0},                                  //1001 0101 0000 1001
			Inst_ELEM{INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "sbis"   , 0x10,getA5_c,getb3_c},                 //1001 1011 AAAA Abbb
			Inst_ELEM{INST_RETI         , 0b1111111111111111, 0b1001010100011000, "reti"   ,  0x0,0,0},                                  //1001 0101 0001 1000
			Inst_ELEM{INST_WDR          , 0b1111111111111111, 0b1001010110101000, "wdr"    ,  0x0,0,0},                                  //1001 0101 1010 1000
			Inst_ELEM{INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "sleep"  ,  0x0,0,0},                                  //1001 0101 1000 1000
			Inst_ELEM{INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "sbic"   , 0x10,getA5_c,getb3_c},                 //1001 1001 AAAA Abbb
			Inst_ELEM{INST_CLT          , 0b1111111111111111, 0b1001010011101000, "clt"    ,  0x0,0,0},                                  //1001 0100 1110 1000
			Inst_ELEM{INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "swap"   ,  0x2,getRd5_c,0},                         //1001 010d dddd 0010
			Inst_ELEM{INST_SEI          , 0b1111111111111111, 0b1001010001111000, "sei"    ,  0x0,0,0},                                  //1001 0100 0111 1000
			Inst_ELEM{INST_SET          , 0b1111111111111111, 0b1001010001101000, "set"    ,  0x0,0,0},                                  //1001 0100 0110 1000
			Inst_ELEM{INST_SEC          , 0b1111111111111111, 0b1001010000001000, "sec"    ,  0x0,0,0},                                  //1001 0100 0000 1000
			Inst_ELEM{INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "ijmp"   ,  0x0,0,0},                                  //1001 0100 0000 1001
			Inst_ELEM{INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "eijmp"  ,  0x0,0,0},                                  //1001 0100 0001 1001
			Inst_ELEM{INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "eicall" ,  0x0,0,0},                                  //1001 0101 0001 1001
			Inst_ELEM{INST_BSET         , 0b1111111110001111, 0b1001010000001000, "bset"   ,  0x1,gets3_c,0},                          //1001 0100 0sss 1000
			Inst_ELEM{INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "bclr"   ,  0x1,gets3_c,0},                          //1001 0100 1sss 1000
			Inst_ELEM{INST_CLC          , 0b1111111111111111, 0b1001010010001000, "clc"    ,  0x0,0,0},                                  //1001 0100 1000 1000
			Inst_ELEM{INST_SEN          , 0b1111111111111111, 0b1001010000101000, "sen"    ,  0x0,0,0},                                  //1001 0100 0010 1000
			Inst_ELEM{INST_CLN          , 0b1111111111111111, 0b1001010010101000, "cln"    ,  0x0,0,0},                                  //1001 0100 1010 1000
			Inst_ELEM{INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "sez"    ,  0x0,0,0},                                  //1001 0100 0001 1000
			Inst_ELEM{INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "clz"    ,  0x0,0,0},                                  //1001 0100 1001 1000
			Inst_ELEM{INST_SES          , 0b1111111111111111, 0b1001010001001000, "ses"    ,  0x0,0,0},                                  //1001 0100 0100 1000
			Inst_ELEM{INST_CLS          , 0b1111111111111111, 0b1001010011001000, "cls"    ,  0x0,0,0},                                  //1001 0100 1100 1000
			Inst_ELEM{INST_SEV          , 0b1111111111111111, 0b1001010000111000, "sev"    ,  0x0,0,0},                                  //1001 0100 0011 1000
			Inst_ELEM{INST_CLV          , 0b1111111111111111, 0b1001010010111000, "clv"    ,  0x0,0,0},                                  //1001 0100 1011 1000
			Inst_ELEM{INST_SEH          , 0b1111111111111111, 0b1001010001011000, "seh"    ,  0x0,0,0},                                  //1001 0100 0101 1000
			Inst_ELEM{INST_CLH          , 0b1111111111111111, 0b1001010011011000, "clh"    ,  0x0,0,0},                                  //1001 0100 1101 1000
			Inst_ELEM{INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "elpm"   ,  0x0,0,0},                                  //1001 0101 1101 1000
			Inst_ELEM{INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "elpm"   ,  0x2,getRd5_c,0},                         //1001 000d dddd 0110
			Inst_ELEM{INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "elpm"   ,  0x2,getRd5_c,0},                         //1001 000d dddd 0111
			Inst_ELEM{INST_SPM          , 0b1111111111111111, 0b1001010111101000, "spm"    ,  0x0,0,0},                                  //1001 0101 1110 1000
			Inst_ELEM{INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "break"  ,  0x0,0,0},                                  //1001 0101 1001 1000
			Inst_ELEM{INST_LDI          , 0b1111000000000000, 0b1110000000000000, "ldi"    ,  0x2,getRd4_c_a16,getK8_d44},          //1110 KKKK dddd KKKK
			Inst_ELEM{INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "rjmp"   ,  0x5,0,0/*&getk12_c_sin*/},                     //1100 kkkk kkkk kkkk
			Inst_ELEM{INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "movw"   , 0x22,getRd4_c_m2,getRr4_c_m2},         //0000 0001 dddd rrrr
			Inst_ELEM{INST_MOV          , 0b1111110000000000, 0b0010110000000000, "mov"    , 0x22,getRd5_c,getRr5_c},               //0010 11rd dddd rrrr
			Inst_ELEM{INST_ADD          , 0b1111110000000000, 0b0000110000000000, "add"    , 0x22,getRd5_c,getRr5_c},               //0000 11rd dddd rrrr
			Inst_ELEM{INST_CPC          , 0b1111110000000000, 0b0000010000000000, "cpc"    , 0x22,getRd5_c,getRr5_c},               //0000 01rd dddd rrrr
			Inst_ELEM{INST_AND          , 0b1111110000000000, 0b0010000000000000, "and"    , 0x22,getRd5_c,getRr5_c},               //0010 00rd dddd rrrr
			Inst_ELEM{INST_EOR          , 0b1111110000000000, 0b0010010000000000, "eor"    , 0x22,getRd5_c,getRr5_c},               //0010 01rd dddd rrrr
			Inst_ELEM{INST_SBC          , 0b1111110000000000, 0b0000100000000000, "sbc"    , 0x22,getRd5_c,getRr5_c},               //0000 10rd dddd rrrr
			Inst_ELEM{INST_OR           , 0b1111110000000000, 0b0010100000000000, "or"     , 0x22,getRd5_c,getRr5_c},               //0010 10rd dddd rrrr
			Inst_ELEM{INST_NOP          , 0b1111111111111111, 0b0000000000000000, "nop"    ,  0x0,0,0},                                  //0000 0000 0000 0000
			Inst_ELEM{INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "mulsu"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 0rrr
			Inst_ELEM{INST_MULS         , 0b1111111100000000, 0b0000001000000000, "muls"   , 0x22,getRd4_c_a16,getRr4_c_a16},       //0000 0010 dddd rrrr
			Inst_ELEM{INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "fmul"   , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 1rrr
			Inst_ELEM{INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "fmuls"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 0rrr
			Inst_ELEM{INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "fmulsu" , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 1rrr
			Inst_ELEM{INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "brbs"   , 0x41,getb3_c,getk7_c_sin},             //1111 00kk kkkk ksss
			Inst_ELEM{INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "brbc"   , 0x41,getb3_c,getk7_c_sin},             //1111 01kk kkkk ksss
			Inst_ELEM{INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "sbrs"   , 0x12,getRd5_c,getb3_c},                //1111 111r rrrr 0bbb
			Inst_ELEM{INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "sbrc"   , 0x12,getRd5_c,getb3_c},                //1111 110r rrrr 0bbb
			Inst_ELEM{INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "rcall"  ,  0x5,0,0/*&getk12_c_sin*/},                     //1101 kkkk kkkk kkkk
			Inst_ELEM{INST_BST          , 0b1111111000001000, 0b1111101000000000, "bst"    , 0x12,getRd5_c,getb3_c},                //1111 101d dddd 0bbb
			Inst_ELEM{INST_BLD          , 0b1111111000001000, 0b1111100000000000, "bld"    , 0x12,getRd5_c,getb3_c},                //1111 100d dddd 0bbb
			Inst_ELEM{INST_ADC          , 0b1111110000000000, 0b0001110000000000, "adc"    , 0x22,getRd5_c,getRr5_c},               //0001 11rd dddd rrrr
			Inst_ELEM{INST_CPI          , 0b1111000000000000, 0b0011000000000000, "cpi"    ,  0x2,getRd4_c_a16,getK8_d44},          //0011 KKKK dddd KKKK
			Inst_ELEM{INST_CP           , 0b1111110000000000, 0b0001010000000000, "cp"     , 0x22,getRd5_c,getRr5_c},               //0001 01rd dddd rrrr
			Inst_ELEM{INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "cpse"   , 0x22,getRd5_c,getRr5_c},               //0001 00rd dddd rrrr
			Inst_ELEM{INST_SUB          , 0b1111110000000000, 0b0001100000000000, "sub"    , 0x22,getRd5_c,getRr5_c},               //0001 10rd dddd rrrr
			Inst_ELEM{INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "ldd"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 1qqq
			Inst_ELEM{INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "ldd"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 0qqq
			Inst_ELEM{INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "ld"     ,  0x2,getRd5_c,0},                         //1000 000d dddd 1000
			Inst_ELEM{INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "ld"     ,  0x2,getRd5_c,0},                         //1000 000d dddd 0000
			Inst_ELEM{INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "std"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 1qqq
			Inst_ELEM{INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "std"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 0qqq
			Inst_ELEM{INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "st"     ,  0x2,getRd5_c,0},                         //1000 001r rrrr 1000
			Inst_ELEM{INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "st"     ,  0x2,getRd5_c,0},                         //1000 001r rrrr 0000
			Inst_ELEM{INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "subi"   ,  0x2,getRd4_c_a16,getK8_d44},          //0101 KKKK dddd KKKK
			Inst_ELEM{INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "andi"   ,  0x2,getRd4_c_a16,getK8_d44},          //0111 KKKK dddd KKKK
			Inst_ELEM{INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "sbci"   ,  0x2,getRd4_c_a16,getK8_d44},          //0100 KKKK dddd KKKK
			Inst_ELEM{INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ori"    ,  0x2,getRd4_c_a16,getK8_d44}           //0110 KKKK dddd KKKK
		};
	};
}

#endif

/*

void executeInstSwitch(uint16_t word);

*/