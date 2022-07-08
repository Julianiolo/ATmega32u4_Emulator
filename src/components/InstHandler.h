#ifndef _A32u4_INSTHANDLER
#define _A32u4_INSTHANDLER

#include <stdint.h>


namespace A32u4 {
	class ATmega32u4;

	class InstHandler {
	private:
		friend class CPU;
		friend class Disassembler;

		uint8_t cycs;
		int16_t PC_add;
		ATmega32u4* const mcu;
		InstHandler(ATmega32u4* mcu);

		void handleInst(uint8_t& CYCL_ADD_Ref, int16_t& PC_ADD_Ref);
public:
		struct inst_effect_t{
			uint8_t addCycs;
			int16_t addPC;

			inst_effect_t(uint8_t addCycs, int16_t addPC) : addCycs(addCycs), addPC(addPC) {}
		};

		template<bool debug,bool analyse>
		inst_effect_t handleInstRawT(uint16_t word);

	private:
		template<bool debug,bool analyse>
		inst_effect_t handleCurrentInstT();

		template<bool debug,bool analyse>
		inst_effect_t handleInstT(uint16_t word);

		

		static constexpr uint8_t startIndArr2[] = { 73, 94, 109, 107, 99, 0, 71, 87 };
		
		static uint8_t getInstInd3(uint16_t word);

		//static void getRegsDirect2(uint16_t word, uint8_t& Rd, uint8_t& Rr);
		static uint8_t getRd2_c_arr(uint16_t word);
		static uint8_t getRd3_c_a16(uint16_t word);
		static uint8_t getRr3_c_a16(uint16_t word);
		static uint8_t getRd4_c(uint16_t word);
		static uint8_t getRr4_c(uint16_t word);
		static uint8_t getRd4_c_a16(uint16_t word);
		static uint8_t getRr4_c_a16(uint16_t word);
		static uint8_t getRd4_c_m2(uint16_t word);
		static uint8_t getRr4_c_m2(uint16_t word);
		static uint8_t getRd5_c(uint16_t word);
		static uint8_t getRr5_c(uint16_t word);
		static uint8_t getK6_d24(uint16_t word);
		static uint8_t getK8_d44(uint16_t word);
		static uint8_t getk7_c_sin(uint16_t word);
		static int16_t getk12_c_sin(uint16_t word);
		static uint8_t getb3_c(uint16_t word);
		static uint8_t getA5_c(uint16_t word);
		static uint8_t getA6_d24(uint16_t word);
		static uint8_t gets3_c(uint16_t word);
		static uint8_t getq6_d123(uint16_t word);
		//static int16_t conv12to16BitInt(uint16_t word);
		static uint32_t getLongAddr(uint16_t word1, uint16_t word2);
		
		void setPC_Cycs_Skip(bool cond);
		uint32_t getExtendedZ();

		//############## Arithmetic and Logic Instructions ##############
		void INST_ADD(uint16_t word);
		void INST_ADC(uint16_t word);
		void INST_ADIW(uint16_t word);

		void INST_SUB(uint16_t word);
		void INST_SUBI(uint16_t word);
		void INST_SBC(uint16_t word);
		void INST_SBCI(uint16_t word);
		void INST_SBIW(uint16_t word);

		void INST_AND(uint16_t word);
		void INST_ANDI(uint16_t word);
		void INST_OR(uint16_t word);
		void INST_ORI(uint16_t word);
		void INST_EOR(uint16_t word);

		void INST_COM(uint16_t word);
		void INST_NEG(uint16_t word);

		//void INST_SBR(uint16_t word);
		//void INST_CBR(uint16_t word);

		void INST_INC(uint16_t word);
		void INST_DEC(uint16_t word);

		//void INST_TST(uint16_t word);
		//void INST_CLR(uint16_t word);
		void INST_SER(uint16_t word);

		void INST_MUL(uint16_t word);
		void INST_MULS(uint16_t word);
		void INST_MULSU(uint16_t word);
		void INST_FMUL(uint16_t word); //i dont know if any of the FMUL instructions do what they are supposed to http://www.kjit.bme.hu/images/stories/targyak/jarmufedelzeti_rendszerek/avr_hw_multiplier.pdf
		void INST_FMULS(uint16_t word);
		void INST_FMULSU(uint16_t word);

		//############## Branch Instructions ##############
		void INST_RJMP(uint16_t word);
		void INST_IJMP(uint16_t word);
		void INST_EIJMP(uint16_t word);
		void INST_JMP(uint16_t word);
		void INST_RCALL(uint16_t word);
		void INST_ICALL(uint16_t word);
		void INST_EICALL(uint16_t word);
		void INST_CALL(uint16_t word);
		void INST_RET(uint16_t word);
		void INST_RETI(uint16_t word);
		void INST_CPSE(uint16_t word);
		void INST_CP(uint16_t word);
		void INST_CPC(uint16_t word);
		void INST_CPI(uint16_t word);
		void INST_SBRC(uint16_t word);
		void INST_SBRS(uint16_t word);
		void INST_SBIC(uint16_t word);
		void INST_SBIS(uint16_t word);
		void INST_BRBS(uint16_t word);
		void INST_BRBC(uint16_t word);
		
		//############## Bit and Bit-Test Instructions ##############
		void INST_SBI(uint16_t word);
		void INST_CBI(uint16_t word);
		void INST_LSR(uint16_t word);
		void INST_ROR(uint16_t word);
		void INST_ASR(uint16_t word);
		void INST_SWAP(uint16_t word);
		void INST_BSET(uint16_t word);
		void INST_BCLR(uint16_t word);
		void INST_BST(uint16_t word);
		void INST_BLD(uint16_t word);

		void INST_SEC(uint16_t word);
		void INST_CLC(uint16_t word);
		void INST_SEN(uint16_t word);
		void INST_CLN(uint16_t word);
		void INST_SEZ(uint16_t word);
		void INST_CLZ(uint16_t word);
		void INST_SEI(uint16_t word);
		void INST_CLI(uint16_t word);
		void INST_SES(uint16_t word);
		void INST_CLS(uint16_t word);
		void INST_SEV(uint16_t word);
		void INST_CLV(uint16_t word);
		void INST_SET(uint16_t word);
		void INST_CLT(uint16_t word);
		void INST_SEH(uint16_t word);
		void INST_CLH(uint16_t word);

		//############## Data Tranfer Instructions ##############
		void INST_MOV(uint16_t word);
		void INST_MOVW(uint16_t word);
		void INST_LDI(uint16_t word);
		void INST_LD_X(uint16_t word);
		void INST_LD_XpostInc(uint16_t word);
		void INST_LD_XpreDec(uint16_t word);
		void INST_LD_Y(uint16_t word);
		void INST_LD_YpostInc(uint16_t word);
		void INST_LD_YpreDec(uint16_t word);
		void INST_LDD_Y(uint16_t word);
		void INST_LD_Z(uint16_t word);
		void INST_LD_ZpostInc(uint16_t word);
		void INST_LD_ZpreDec(uint16_t word);
		void INST_LDD_Z(uint16_t word);
		void INST_LDS(uint16_t word);
		void INST_ST_X(uint16_t word);
		void INST_ST_XpostInc(uint16_t word);
		void INST_ST_XpreDec(uint16_t word);
		void INST_ST_Y(uint16_t word);
		void INST_ST_YpostInc(uint16_t word);
		void INST_ST_YpreDec(uint16_t word);
		void INST_STD_Y(uint16_t word);
		void INST_ST_Z(uint16_t word);
		void INST_ST_ZpostInc(uint16_t word);
		void INST_ST_ZpreDec(uint16_t word);
		void INST_STD_Z(uint16_t word);
		void INST_STS(uint16_t word);
		void INST_LPM_0(uint16_t word);
		void INST_LPM_d(uint16_t word);
		void INST_LPM_dpostInc(uint16_t word);
		void INST_ELPM_0(uint16_t word);
		void INST_ELPM_d(uint16_t word);
		void INST_ELPM_dpostInc(uint16_t word);
		void INST_SPM(uint16_t word);
		void INST_IN(uint16_t word);
		void INST_OUT(uint16_t word);
		void INST_PUSH(uint16_t word);
		void INST_POP(uint16_t word);

		//############## MCU Control Instructions ##############
		void INST_NOP(uint16_t word);
		void INST_SLEEP(uint16_t word);
		void INST_WDR(uint16_t word);
		void INST_BREAK(uint16_t word);

	public:
		static bool is2WordInst(uint16_t word);
		static uint8_t getInstInd(uint16_t word);

		struct Inst_ELEM {
			void (InstHandler::*func)(uint16_t word);
			const uint16_t mask;
			const uint16_t res;
			const char* const name;

			const uint8_t parTypes;

			uint8_t(*par1)(uint16_t word) = nullptr;
			uint8_t(*par2)(uint16_t word) = nullptr;
		};

		static constexpr uint8_t instListLen = 111;

		static constexpr Inst_ELEM instList[] = {
		//   func                 mask                res
		//				            1   2   3   4       1   2   3   4
			{&A32u4::InstHandler::INST_STS          , 0b1111111000001111, 0b1001001000000000, "sts"    , 0x32,getRd5_c,0},                         //1001 001d dddd 0000 kkkk kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_LDS          , 0b1111111000001111, 0b1001000000000000, "lds"    , 0x32,getRd5_c,0},                         //1001 000d dddd 0000 kkkk kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_POP          , 0b1111111000001111, 0b1001000000001111, "pop"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 1111
			{&A32u4::InstHandler::INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "push"   ,  0x2,getRd5_c,0},                         //1001 001d dddd 1111
			{&A32u4::InstHandler::INST_CALL         , 0b1111111000001110, 0b1001010000001110, "call"   ,  0x3,0,0},                                  //1001 010k kkkk 111k kkkk kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1100
			{&A32u4::InstHandler::INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1101
			{&A32u4::InstHandler::INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1110
			{&A32u4::InstHandler::INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1001
			{&A32u4::InstHandler::INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 1010
			{&A32u4::InstHandler::INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 0001
			{&A32u4::InstHandler::INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "ld"     ,  0x2,getRd5_c,0},                         //1001 000d dddd 0010
			{&A32u4::InstHandler::INST_JMP          , 0b1111111000001110, 0b1001010000001100, "jmp"    ,  0x3,0,0},                                  //1001 010k kkkk 110k kkkk kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_RET          , 0b1111111111111111, 0b1001010100001000, "ret"    ,  0x0,0,0},                                  //1001 0101 0000 1000
			{&A32u4::InstHandler::INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "adiw"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0110 KKdd KKKK
			{&A32u4::InstHandler::INST_OUT          , 0b1111100000000000, 0b1011100000000000, "out"    , 0x20,getA6_d24,getRd5_c},              //1011 1AAr rrrr AAAA
			{&A32u4::InstHandler::INST_IN           , 0b1111100000000000, 0b1011000000000000, "in"     ,  0x2,getRd5_c,getA6_d24},              //1011 0AAd dddd AAAA
			{&A32u4::InstHandler::INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ror"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0111
			{&A32u4::InstHandler::INST_DEC          , 0b1111111000001111, 0b1001010000001010, "dec"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 1010
			{&A32u4::InstHandler::INST_MUL          , 0b1111110000000000, 0b1001110000000000, "mul"    , 0x22,getRd5_c,getRr5_c},               //1001 11rd dddd rrrr
			{&A32u4::InstHandler::INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "lpm"    ,  0x0,0,0},                                  //1001 0101 1100 1000
			{&A32u4::InstHandler::INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "lpm"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 0100
			{&A32u4::InstHandler::INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "lpm"    ,  0x2,getRd5_c,0},                         //1001 000d dddd 0101
			{&A32u4::InstHandler::INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "sbiw"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0111 KKdd KKKK
			{&A32u4::InstHandler::INST_LSR          , 0b1111111000001111, 0b1001010000000110, "lsr"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0110
			{&A32u4::InstHandler::INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1100
			{&A32u4::InstHandler::INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1101
			{&A32u4::InstHandler::INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1110
			{&A32u4::InstHandler::INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1001
			{&A32u4::InstHandler::INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 1010
			{&A32u4::InstHandler::INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 0001
			{&A32u4::InstHandler::INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "st"     ,  0x2,getRd5_c,0},                         //1001 001r rrrr 0010
			{&A32u4::InstHandler::INST_SBI          , 0b1111111100000000, 0b1001101000000000, "sbi"    , 0x10,getA5_c,getb3_c},                 //1001 1010 AAAA Abbb
			{&A32u4::InstHandler::INST_CBI          , 0b1111111100000000, 0b1001100000000000, "cbi"    , 0x10,getA5_c,getb3_c},                 //1001 1000 AAAA Abbb
			{&A32u4::InstHandler::INST_COM          , 0b1111111000001111, 0b1001010000000000, "com"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0000
			{&A32u4::InstHandler::INST_CLI          , 0b1111111111111111, 0b1001010011111000, "cli"    ,  0x0,0,0},                                  //1001 0100 1111 1000
			{&A32u4::InstHandler::INST_INC          , 0b1111111000001111, 0b1001010000000011, "inc"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0011
			{&A32u4::InstHandler::INST_NEG          , 0b1111111000001111, 0b1001010000000001, "neg"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0001
			{&A32u4::InstHandler::INST_ASR          , 0b1111111000001111, 0b1001010000000101, "asr"    ,  0x2,getRd5_c,0},                         //1001 010d dddd 0101
			{&A32u4::InstHandler::INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "icall"  ,  0x0,0,0},                                  //1001 0101 0000 1001
			{&A32u4::InstHandler::INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "sbis"   , 0x10,getA5_c,getb3_c},                 //1001 1011 AAAA Abbb
			{&A32u4::InstHandler::INST_RETI         , 0b1111111111111111, 0b1001010100011000, "reti"   ,  0x0,0,0},                                  //1001 0101 0001 1000
			{&A32u4::InstHandler::INST_WDR          , 0b1111111111111111, 0b1001010110101000, "wdr"    ,  0x0,0,0},                                  //1001 0101 1010 1000
			{&A32u4::InstHandler::INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "sleep"  ,  0x0,0,0},                                  //1001 0101 1000 1000
			{&A32u4::InstHandler::INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "sbic"   , 0x10,getA5_c,getb3_c},                 //1001 1001 AAAA Abbb
			{&A32u4::InstHandler::INST_CLT          , 0b1111111111111111, 0b1001010011101000, "clt"    ,  0x0,0,0},                                  //1001 0100 1110 1000
			{&A32u4::InstHandler::INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "swap"   ,  0x2,getRd5_c,0},                         //1001 010d dddd 0010
			{&A32u4::InstHandler::INST_SEI          , 0b1111111111111111, 0b1001010001111000, "sei"    ,  0x0,0,0},                                  //1001 0100 0111 1000
			{&A32u4::InstHandler::INST_SET          , 0b1111111111111111, 0b1001010001101000, "set"    ,  0x0,0,0},                                  //1001 0100 0110 1000
			{&A32u4::InstHandler::INST_SEC          , 0b1111111111111111, 0b1001010000001000, "sec"    ,  0x0,0,0},                                  //1001 0100 0000 1000
			{&A32u4::InstHandler::INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "ijmp"   ,  0x0,0,0},                                  //1001 0100 0000 1001
			{&A32u4::InstHandler::INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "eijmp"  ,  0x0,0,0},                                  //1001 0100 0001 1001
			{&A32u4::InstHandler::INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "eicall" ,  0x0,0,0},                                  //1001 0101 0001 1001
			{&A32u4::InstHandler::INST_BSET         , 0b1111111110001111, 0b1001010000001000, "bset"   ,  0x1,gets3_c,0},                          //1001 0100 0sss 1000
			{&A32u4::InstHandler::INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "bclr"   ,  0x1,gets3_c,0},                          //1001 0100 1sss 1000
			{&A32u4::InstHandler::INST_CLC          , 0b1111111111111111, 0b1001010010001000, "clc"    ,  0x0,0,0},                                  //1001 0100 1000 1000
			{&A32u4::InstHandler::INST_SEN          , 0b1111111111111111, 0b1001010000101000, "sen"    ,  0x0,0,0},                                  //1001 0100 0010 1000
			{&A32u4::InstHandler::INST_CLN          , 0b1111111111111111, 0b1001010010101000, "cln"    ,  0x0,0,0},                                  //1001 0100 1010 1000
			{&A32u4::InstHandler::INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "sez"    ,  0x0,0,0},                                  //1001 0100 0001 1000
			{&A32u4::InstHandler::INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "clz"    ,  0x0,0,0},                                  //1001 0100 1001 1000
			{&A32u4::InstHandler::INST_SES          , 0b1111111111111111, 0b1001010001001000, "ses"    ,  0x0,0,0},                                  //1001 0100 0100 1000
			{&A32u4::InstHandler::INST_CLS          , 0b1111111111111111, 0b1001010011001000, "cls"    ,  0x0,0,0},                                  //1001 0100 1100 1000
			{&A32u4::InstHandler::INST_SEV          , 0b1111111111111111, 0b1001010000111000, "sev"    ,  0x0,0,0},                                  //1001 0100 0011 1000
			{&A32u4::InstHandler::INST_CLV          , 0b1111111111111111, 0b1001010010111000, "clv"    ,  0x0,0,0},                                  //1001 0100 1011 1000
			{&A32u4::InstHandler::INST_SEH          , 0b1111111111111111, 0b1001010001011000, "seh"    ,  0x0,0,0},                                  //1001 0100 0101 1000
			{&A32u4::InstHandler::INST_CLH          , 0b1111111111111111, 0b1001010011011000, "clh"    ,  0x0,0,0},                                  //1001 0100 1101 1000
			{&A32u4::InstHandler::INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "elpm"   ,  0x0,0,0},                                  //1001 0101 1101 1000
			{&A32u4::InstHandler::INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "elpm"   ,  0x2,getRd5_c,0},                         //1001 000d dddd 0110
			{&A32u4::InstHandler::INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "elpm"   ,  0x2,getRd5_c,0},                         //1001 000d dddd 0111
			{&A32u4::InstHandler::INST_SPM          , 0b1111111111111111, 0b1001010111101000, "spm"    ,  0x0,0,0},                                  //1001 0101 1110 1000
			{&A32u4::InstHandler::INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "break"  ,  0x0,0,0},                                  //1001 0101 1001 1000
			{&A32u4::InstHandler::INST_LDI          , 0b1111000000000000, 0b1110000000000000, "ldi"    ,  0x2,getRd4_c_a16,getK8_d44},          //1110 KKKK dddd KKKK
			{&A32u4::InstHandler::INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "rjmp"   ,  0x5,0,0/*&getk12_c_sin*/},                     //1100 kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "movw"   , 0x22,getRd4_c_m2,getRr4_c_m2},         //0000 0001 dddd rrrr
			{&A32u4::InstHandler::INST_MOV          , 0b1111110000000000, 0b0010110000000000, "mov"    , 0x22,getRd5_c,getRr5_c},               //0010 11rd dddd rrrr
			{&A32u4::InstHandler::INST_ADD          , 0b1111110000000000, 0b0000110000000000, "add"    , 0x22,getRd5_c,getRr5_c},               //0000 11rd dddd rrrr
			{&A32u4::InstHandler::INST_CPC          , 0b1111110000000000, 0b0000010000000000, "cpc"    , 0x22,getRd5_c,getRr5_c},               //0000 01rd dddd rrrr
			{&A32u4::InstHandler::INST_AND          , 0b1111110000000000, 0b0010000000000000, "and"    , 0x22,getRd5_c,getRr5_c},               //0010 00rd dddd rrrr
			{&A32u4::InstHandler::INST_EOR          , 0b1111110000000000, 0b0010010000000000, "eor"    , 0x22,getRd5_c,getRr5_c},               //0010 01rd dddd rrrr
			{&A32u4::InstHandler::INST_SBC          , 0b1111110000000000, 0b0000100000000000, "sbc"    , 0x22,getRd5_c,getRr5_c},               //0000 10rd dddd rrrr
			{&A32u4::InstHandler::INST_OR           , 0b1111110000000000, 0b0010100000000000, "or"     , 0x22,getRd5_c,getRr5_c},               //0010 10rd dddd rrrr
			{&A32u4::InstHandler::INST_NOP          , 0b1111111111111111, 0b0000000000000000, "nop"    ,  0x0,0,0},                                  //0000 0000 0000 0000
			{&A32u4::InstHandler::INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "mulsu"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 0rrr
			{&A32u4::InstHandler::INST_MULS         , 0b1111111100000000, 0b0000001000000000, "muls"   , 0x22,getRd4_c_a16,getRr4_c_a16},       //0000 0010 dddd rrrr
			{&A32u4::InstHandler::INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "fmul"   , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 1rrr
			{&A32u4::InstHandler::INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "fmuls"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 0rrr
			{&A32u4::InstHandler::INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "fmulsu" , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 1rrr
			{&A32u4::InstHandler::INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "brbs"   , 0x41,getb3_c,getk7_c_sin},             //1111 00kk kkkk ksss
			{&A32u4::InstHandler::INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "brbc"   , 0x41,getb3_c,getk7_c_sin},             //1111 01kk kkkk ksss
			{&A32u4::InstHandler::INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "sbrs"   , 0x12,getRd5_c,getb3_c},                //1111 111r rrrr 0bbb
			{&A32u4::InstHandler::INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "sbrc"   , 0x12,getRd5_c,getb3_c},                //1111 110r rrrr 0bbb
			{&A32u4::InstHandler::INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "rcall"  ,  0x5,0,0/*&getk12_c_sin*/},                     //1101 kkkk kkkk kkkk
			{&A32u4::InstHandler::INST_BST          , 0b1111111000001000, 0b1111101000000000, "bst"    , 0x12,getRd5_c,getb3_c},                //1111 101d dddd 0bbb
			{&A32u4::InstHandler::INST_BLD          , 0b1111111000001000, 0b1111100000000000, "bld"    , 0x12,getRd5_c,getb3_c},                //1111 100d dddd 0bbb
			{&A32u4::InstHandler::INST_ADC          , 0b1111110000000000, 0b0001110000000000, "adc"    , 0x22,getRd5_c,getRr5_c},               //0001 11rd dddd rrrr
			{&A32u4::InstHandler::INST_CPI          , 0b1111000000000000, 0b0011000000000000, "cpi"    ,  0x2,getRd4_c_a16,getK8_d44},          //0011 KKKK dddd KKKK
			{&A32u4::InstHandler::INST_CP           , 0b1111110000000000, 0b0001010000000000, "cp"     , 0x22,getRd5_c,getRr5_c},               //0001 01rd dddd rrrr
			{&A32u4::InstHandler::INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "cpse"   , 0x22,getRd5_c,getRr5_c},               //0001 00rd dddd rrrr
			{&A32u4::InstHandler::INST_SUB          , 0b1111110000000000, 0b0001100000000000, "sub"    , 0x22,getRd5_c,getRr5_c},               //0001 10rd dddd rrrr
			{&A32u4::InstHandler::INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "ldd"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 1qqq
			{&A32u4::InstHandler::INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "ldd"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 0qqq
			{&A32u4::InstHandler::INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "ld"     ,  0x2,getRd5_c,0},                         //1000 000d dddd 1000
			{&A32u4::InstHandler::INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "ld"     ,  0x2,getRd5_c,0},                         //1000 000d dddd 0000
			{&A32u4::InstHandler::INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "std"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 1qqq
			{&A32u4::InstHandler::INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "std"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 0qqq
			{&A32u4::InstHandler::INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "st"     ,  0x2,getRd5_c,0},                         //1000 001r rrrr 1000
			{&A32u4::InstHandler::INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "st"     ,  0x2,getRd5_c,0},                         //1000 001r rrrr 0000
			{&A32u4::InstHandler::INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "subi"   ,  0x2,getRd4_c_a16,getK8_d44},          //0101 KKKK dddd KKKK
			{&A32u4::InstHandler::INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "andi"   ,  0x2,getRd4_c_a16,getK8_d44},          //0111 KKKK dddd KKKK
			{&A32u4::InstHandler::INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "sbci"   ,  0x2,getRd4_c_a16,getK8_d44},          //0100 KKKK dddd KKKK
			{&A32u4::InstHandler::INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ori"    ,  0x2,getRd4_c_a16,getK8_d44}           //0110 KKKK dddd KKKK
		};
	};
}

#endif

/*

void executeInstSwitch(uint16_t word);

*/