#include "InstHandler.h"
#include "../ATmega32u4.h"

#include "config.h"

#include "InstInds.h"
#include "Disassembler.h"

#include "../utils/StringUtils.h"
#include "../utils/bitMacros.h"

const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"    , 0x32,getRd5_c},                         //1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"    , 0x32,getRd5_c},                         //1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"    ,  0x2,getRd5_c},                         //1001 000d dddd 1111
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"   ,  0x2,getRd5_c},                         //1001 001d dddd 1111
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"   ,  0x3,},                                  //1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 1110
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 1010
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD"     ,  0x2,getRd5_c},                         //1001 000d dddd 0010
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"    ,  0x3,},                                  //1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"    ,  0x0,},                                  //1001 0101 0000 1000
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0110 KKdd KKKK
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"    , 0x20,getA6_d24,getRd5_c},              //1011 1AAr rrrr AAAA
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"     ,  0x2,getRd5_c,getA6_d24},              //1011 0AAd dddd AAAA
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"    ,  0x2,getRd5_c},                         //1001 010d dddd 0111
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"    ,  0x2,getRd5_c},                         //1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"    , 0x22,getRd5_c,getRr5_c},               //1001 11rd dddd rrrr
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM"    ,  0x0,},                                  //1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM"    ,  0x2,getRd5_c},                         //1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM"    ,  0x2,getRd5_c},                         //1001 000d dddd 0101
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"   ,  0x2,getRd2_c_arr,getK6_d24},          //1001 0111 KKdd KKKK
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"    ,  0x2,getRd5_c},                         //1001 010d dddd 0110
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 1110
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 1010
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST"     ,  0x2,getRd5_c},                         //1001 001r rrrr 0010
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"    , 0x10,getA5_c,getb3_c},                 //1001 1010 AAAA Abbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"    , 0x10,getA5_c,getb3_c},                 //1001 1000 AAAA Abbb
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"    ,  0x2,getRd5_c},                         //1001 010d dddd 0000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"    ,  0x0,},                                  //1001 0100 1111 1000
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"    ,  0x2,getRd5_c},                         //1001 010d dddd 0011
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"    ,  0x2,getRd5_c},                         //1001 010d dddd 0001
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"    ,  0x2,getRd5_c},                         //1001 010d dddd 0101
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"  ,  0x0,},                                  //1001 0101 0000 1001
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"   , 0x10,getA5_c,getb3_c},                 //1001 1011 AAAA Abbb
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"   ,  0x0,},                                  //1001 0101 0001 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"    ,  0x0,},                                  //1001 0101 1010 1000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"  ,  0x0,},                                  //1001 0101 1000 1000
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"   , 0x10,getA5_c,getb3_c},                 //1001 1001 AAAA Abbb
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"    ,  0x0,},                                  //1001 0100 1110 1000
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"   ,  0x2,getRd5_c},                         //1001 010d dddd 0010
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"    ,  0x0,},                                  //1001 0100 0111 1000
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"    ,  0x0,},                                  //1001 0100 0110 1000
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"    ,  0x0,},                                  //1001 0100 0000 1000
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"   ,  0x0,},                                  //1001 0100 0000 1001
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"  ,  0x0,},                                  //1001 0100 0001 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL" ,  0x0,},                                  //1001 0101 0001 1001
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"   ,  0x1,gets3_c},                          //1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"   ,  0x1,gets3_c},                          //1001 0100 1sss 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"    ,  0x0,},                                  //1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"    ,  0x0,},                                  //1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"    ,  0x0,},                                  //1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"    ,  0x0,},                                  //1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"    ,  0x0,},                                  //1001 0100 1001 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"    ,  0x0,},                                  //1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"    ,  0x0,},                                  //1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"    ,  0x0,},                                  //1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"    ,  0x0,},                                  //1001 0100 1011 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"    ,  0x0,},                                  //1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"    ,  0x0,},                                  //1001 0100 1101 1000
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM"   ,  0x0,},                                  //1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM"   ,  0x2,getRd5_c},                         //1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM"   ,  0x2,getRd5_c},                         //1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"    ,  0x0,},                                  //1001 0101 1110 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"  ,  0x0,},                                  //1001 0101 1001 1000
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"    ,  0x2,getRd4_c_a16,getK8_d44},          //1110 KKKK dddd KKKK
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"   ,  0x5,/*&getk12_c_sin*/},                     //1100 kkkk kkkk kkkk
	{&INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"   , 0x22,getRd4_c_m2,getRr4_c_m2},         //0000 0001 dddd rrrr
	{&INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"    , 0x22,getRd5_c,getRr5_c},               //0010 11rd dddd rrrr
	{&INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"    , 0x22,getRd5_c,getRr5_c},               //0000 11rd dddd rrrr
	{&INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"    , 0x22,getRd5_c,getRr5_c},               //0000 01rd dddd rrrr
	{&INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"    , 0x22,getRd5_c,getRr5_c},               //0010 00rd dddd rrrr
	{&INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"    , 0x22,getRd5_c,getRr5_c},               //0010 01rd dddd rrrr
	{&INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"    , 0x22,getRd5_c,getRr5_c},               //0000 10rd dddd rrrr
	{&INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"     , 0x22,getRd5_c,getRr5_c},               //0010 10rd dddd rrrr
	{&INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"    ,  0x0,},                                  //0000 0000 0000 0000
	{&INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 0rrr
	{&INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"   , 0x22,getRd4_c_a16,getRr4_c_a16},       //0000 0010 dddd rrrr
	{&INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"   , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 0ddd 1rrr
	{&INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"  , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 0rrr
	{&INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU" , 0x22,getRd3_c_a16,getRr3_c_a16},       //0000 0011 1ddd 1rrr
	{&INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"   , 0x41,getb3_c,getk7_c_sin},             //1111 00kk kkkk ksss
	{&INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"   , 0x41,getb3_c,getk7_c_sin},             //1111 01kk kkkk ksss
	{&INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"   , 0x12,getRd5_c,getb3_c},                //1111 111r rrrr 0bbb
	{&INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"   , 0x12,getRd5_c,getb3_c},                //1111 110r rrrr 0bbb
	{&INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"  ,  0x5,/*&getk12_c_sin*/},                     //1101 kkkk kkkk kkkk
	{&INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"    , 0x12,getRd5_c,getb3_c},                //1111 101d dddd 0bbb
	{&INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"    , 0x12,getRd5_c,getb3_c},                //1111 100d dddd 0bbb
	{&INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"    , 0x22,getRd5_c,getRr5_c},               //0001 11rd dddd rrrr
	{&INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"    ,  0x2,getRd4_c_a16,getK8_d44},          //0011 KKKK dddd KKKK
	{&INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"     , 0x22,getRd5_c,getRr5_c},               //0001 01rd dddd rrrr
	{&INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"   , 0x22,getRd5_c,getRr5_c},               //0001 00rd dddd rrrr
	{&INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"    , 0x22,getRd5_c,getRr5_c},               //0001 10rd dddd rrrr
	{&INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 1qqq
	{&INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq0d dddd 0qqq
	{&INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD"     ,  0x2,getRd5_c},                         //1000 000d dddd 1000
	{&INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD"     ,  0x2,getRd5_c},                         //1000 000d dddd 0000
	{&INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 1qqq
	{&INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD"    , 0x62,getRd5_c,getq6_d123},             //10q0 qq1r rrrr 0qqq
	{&INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST"     ,  0x2,getRd5_c},                         //1000 001r rrrr 1000
	{&INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST"     ,  0x2,getRd5_c},                         //1000 001r rrrr 0000
	{&INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"   ,  0x2,getRd4_c_a16,getK8_d44},          //0101 KKKK dddd KKKK
	{&INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"   ,  0x2,getRd4_c_a16,getK8_d44},          //0111 KKKK dddd KKKK
	{&INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"   ,  0x2,getRd4_c_a16,getK8_d44},          //0100 KKKK dddd KKKK
	{&INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"    ,  0x2,getRd4_c_a16,getK8_d44},          //0110 KKKK dddd KKKK
};

A32u4::InstHandler::InstHandler(ATmega32u4* mcu) : mcu(mcu), 
cycs(0), PC_add(0) {

}

void A32u4::InstHandler::handleInst(uint8_t& CYCL_ADD_Ref, int16_t& PC_ADD_Ref) { //returns how many cycles the Inst took
	cycs = 0;
	PC_add = 0;
	
	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);

#if	DEBUGGING_F
	mcu->debugger.checkBreakpoints();

	if (mcu->debugger.printDisassembly) {
#endif
		//println(Disassembler::disassemble(word, word2, mcu->cpu.PC));
#if	DEBUGGING_F
	}
#endif

	uint8_t ind;
#if !USE_INSTCACHE
	ind = getInstInd(word);
#else
	ind = mcu->flash.getInstIndCache(mcu->cpu.PC);
#endif

#if	ANALYSE_F
	mcu->analytics.addData(ind, mcu->cpu.PC);
#endif

	//std::invoke(instList[ind].func, this, word);
	(this->*instList[ind].func)(word);
	
#if 0
	indAdd *= (ind+1) * (ind+3);
	indAdd += ind;
	indAdd /= (ind * ind * ind) + 1;
	indAdd ^= 20874;
	indAdd ^= 32874*ind;
#endif
	indAdd++;

	PC_ADD_Ref = PC_add;
	CYCL_ADD_Ref = cycs;
}

void A32u4::InstHandler::executeInstSwitch(uint16_t word) {
	switch ((word & 0xD208) >> 12) {
	case 0b1000000000000000:
		INST_LDD_Z(word); goto break_all;
	case 0b1000000000001000:
		INST_LDD_Y(word); goto break_all;
	case 0b1000001000000000:
		INST_STD_Z(word); goto break_all;
	case 0b1000001000001000:
		INST_STD_Y(word); goto break_all;
	}

	switch ((word & 0xF000) >> 12) {
	case 0b0000:
		switch ((word & 0x0C00) >> (8 + 2)) {
		case 0b01:
			INST_CPC(word); goto break_all;
		case 0b10:
			INST_SBC(word); goto break_all;
		case 0b11:
			INST_ADD(word); goto break_all;
		}

		switch ((word & 0x0F00) >> (8)) {
		case 0b0000:
			switch (word & 0x00FF) {
			case 0b00000000:
				INST_NOP(word); goto break_all;
			}
		case 0b0001:
			INST_MOVW(word); goto break_all;
		case 0b0010:
			INST_MULS(word); goto break_all;
		case 0b0011:
			switch ((word & 0x0088) >> (8)) {
			case 0b00000000:
				INST_MULSU(word); goto break_all;
			case 0b00001000:
				INST_FMUL(word); goto break_all;
			case 0b10000000:
				INST_FMULS(word); goto break_all;
			case 0b10001000:
				INST_FMULSU(word); goto break_all;
			}
			break;
		}
		break;

	case 0b0001:
		switch ((word & 0x0C00) >> (8 + 2)) {
		case 0b00:
			INST_CPSE(word); goto break_all;
		case 0b01:
			INST_CP(word); goto break_all;
		case 0b10:
			INST_SUB(word); goto break_all;
		case 0b11:
			INST_ADC(word); goto break_all;
		}
		break;

	case 0b0010:
		switch ((word & 0x0C00) >> (8 + 2)) {
		case 0b00:
			INST_AND(word); goto break_all;
		case 0b01:
			INST_EOR(word); goto break_all;
		case 0b10:
			INST_OR(word); goto break_all;
		case 0b11:
			INST_MOV(word); goto break_all;
		}
		break;

	case 0b0011:
		INST_CPI(word); goto break_all;

	case 0b0100:
		INST_SBCI(word); goto break_all;
	case 0b0101:
		INST_SUBI(word); goto break_all;

	case 0b0110:
		INST_ORI(word); goto break_all; //also SBR

	case 0b0111:
		INST_ANDI(word); goto break_all; //also CBR (but with complement of K, but i dont have to worry about that)

	case 0b1000:
		switch ((word & 0x0E00) >> (8 + 1)) {
		case 0b000:
			switch (word & 0x000F) {
			case 0b0000:
				INST_LD_Z(word); goto break_all;
			case 0b1000:
				INST_LD_Y(word); goto break_all;
			}
			break;
		case 0b001:
			switch (word & 0x000F) {
			case 0b0000:
				INST_ST_Z(word); goto break_all;
			case 0b1000:
				INST_ST_Y(word); goto break_all;
			}
			break;
		}
		break;

	case 0b1001:
		switch ((word & 0x0C00) >> (8 + 2)) {
		case 0b11:
			INST_MUL(word); goto break_all;
		}

		switch ((word & 0x0E00) >> (8 + 1)) {
		case 0b000:
			switch (word & 0x000F) {
			case 0b0000:
				INST_LDS(word); goto break_all;
			case 0b0001:
				INST_LD_ZpostInc(word); goto break_all;
			case 0b0010:
				INST_LD_ZpreDec(word); goto break_all;
			case 0b0100:
				INST_LPM_d(word); goto break_all;
			case 0b0101:
				INST_LPM_dpostInc(word); goto break_all;
			case 0b0110:
				INST_ELPM_d(word); goto break_all;
			case 0b0111:
				INST_ELPM_dpostInc(word); goto break_all;
			case 0b1001:
				INST_LD_YpostInc(word); goto break_all;
			case 0b1010:
				INST_LD_YpreDec(word); goto break_all;
			case 0b1111:
				INST_POP(word); goto break_all;
			}

		case 0b001:
			switch (word & 0x000F) {
			case 0b0000:
				INST_STS(word); goto break_all;
			case 0b0001:
				INST_ST_ZpostInc(word); goto break_all;
			case 0b0010:
				INST_ST_ZpreDec(word); goto break_all;
			case 0b1001:
				INST_ST_YpostInc(word); goto break_all;
			case 0b1010:
				INST_ST_YpreDec(word); goto break_all;
			case 0b1100:
				INST_ST_X(word); goto break_all;
			case 0b1101:
				INST_ST_XpostInc(word); goto break_all;
			case 0b1110:
				INST_ST_XpreDec(word); goto break_all;
			case 0b1111:
				INST_PUSH(word); goto break_all;
			}

		case 0b010:
		{
			switch ((word & 0x000E) >> 1) {
			case 0b110:
				INST_JMP(word);  goto break_all;
			case 0b111:
				INST_CALL(word);  goto break_all;
			}
		}

		switch (word & 0x000F) {
		case 0b0000:
			INST_COM(word); goto break_all;
		case 0b0001:
			INST_NEG(word); goto break_all;
		case 0b0010:
			INST_SWAP(word); goto break_all;
		case 0b0011:
			INST_INC(word); goto break_all;
		case 0b0101:
			INST_ASR(word); goto break_all;
		case 0b0110:
			INST_LSR(word); goto break_all;
		case 0b0111:
			INST_ROR(word); goto break_all;
		case 0b1010:
			INST_DEC(word); goto break_all;
		case 0b1100:
			INST_LD_X(word); goto break_all;
		case 0b1101:
			INST_LD_XpostInc(word); goto break_all;
		case 0b1110:
			INST_LD_XpreDec(word); goto break_all;
		}
		break;
		}

		switch ((word & 0x0F00) >> 8) {
		case 0b0100:
			switch (word & 0x00FF) {
			case 0b00001000:
				INST_SEC(word); goto break_all;
			case 0b10001000:
				INST_CLC(word); goto break_all;
			case 0b00011000:
				INST_SEZ(word); goto break_all;
			case 0b10011000:
				INST_CLZ(word); goto break_all;
			case 0b00101000:
				INST_SEN(word); goto break_all;
			case 0b10101000:
				INST_CLN(word); goto break_all;
			case 0b00111000:
				INST_SEV(word); goto break_all;
			case 0b10111000:
				INST_CLV(word); goto break_all;
			case 0b01001000:
				INST_SES(word); goto break_all;
			case 0b11001000:
				INST_CLS(word); goto break_all;
			case 0b01011000:
				INST_SEH(word); goto break_all;
			case 0b11011000:
				INST_CLH(word); goto break_all;
			case 0b01101000:
				INST_SET(word); goto break_all;
			case 0b11101000:
				INST_CLT(word); goto break_all;
			case 0b01111000:
				INST_SEI(word); goto break_all;
			case 0b11111000:
				INST_CLI(word); goto break_all;

			case 0b00001001:
				INST_IJMP(word); goto break_all;
			case 0b00011001:
				INST_EIJMP(word); goto break_all;
			}
			break;
		case 0b0101:
			switch (word & 0b10001111) {
			case 0b00001000:
				INST_BSET(word); goto break_all;
			}
			switch (word & 0x00FF) {
			case 0b00001000:
				INST_RET(word); goto break_all;
			case 0b00011000:
				INST_RETI(word); goto break_all;
			case 0b00001001:
				INST_ICALL(word); goto break_all;
			case 0b00011001:
				INST_EICALL(word); goto break_all;
			case 0b10001000:
				INST_SLEEP(word); goto break_all;
			case 0b10011000:
				INST_BREAK(word); goto break_all;
			case 0b10101000:
				INST_WDR(word); goto break_all;
			case 0b11001000:
				INST_LPM_0(word); goto break_all;
			case 0b11011000:
				INST_ELPM_0(word); goto break_all;
			case 0b11101000:
				INST_SPM(word); goto break_all;
			}
			break;
		case 0b0110:
			INST_ADIW(word); goto break_all;
		case 0b0111:
			INST_SBIW(word); goto break_all;
		case 0b1000:
			INST_CBI(word); goto break_all;
		case 0b1001:
			INST_SBIC(word); goto break_all;
		case 0b1010:
			INST_SBI(word); goto break_all;
		case 0b1011:
			INST_SBIS(word); goto break_all;
		}
		break;

	case 0b1011:
		switch ((word & 0x0800) >> (8 + 3)) {
		case 0:
			INST_IN(word); goto break_all;
		case 1:
			INST_OUT(word); goto break_all;
		}
		break;

	case 0b1100:
		INST_RJMP(word); goto break_all;

	case 0b1101:
		INST_RCALL(word); goto break_all;

	case 0b1110:
		INST_LDI(word); goto break_all;

	case 0b1111:
		switch ((word & 0x0C00) >> (8 + 2)) {
		case 0b00:
			INST_BRBS(word); goto break_all;
		case 0b01:
			INST_BRBC(word); goto break_all;
		}

		switch (word & 0x0E09) {
		case 0b100000000000:
			INST_BLD(word); goto break_all;
		case 0b101000000000:
			INST_BST(word); goto break_all;
		case 0b110000000000:
			INST_SBRC(word); goto break_all;
		case 0b111000000000:
			INST_SBRS(word); goto break_all;
		}
	}
	mcu->logf(ATmega32u4::LogLevel_Error, "unhandled Inst p1: 0x%04x", word);
break_all:
	return;
}

uint8_t A32u4::InstHandler::getInstInd(uint16_t word) {
	return getInstInd3(word);
}
uint8_t A32u4::InstHandler::getInstInd3(uint16_t word) {
	uint8_t startInd = startIndArr2[(word & 0b1100000000000000) >> 13 | ((word & 0b0001000000000000) != 0)];

	for (uint8_t i = startInd; i < instListLen; i++) {
		if ((word & instList[i].mask) == instList[i].res) {
			return i;
		}
	}

	//println("Unhaldled Inst: 0x" << std::hex << word); // << " At: 0x" << mcu->cpu.PC

	return 0xff;
}

int16_t convTo16BitInt(uint16_t word, uint8_t bitCnt) {
	return isBitSet(word, bitCnt - 1) ? (((int16_t)-1 ^ ((1 << bitCnt) - 1)) | word) : word;
}
//Parameters
uint8_t A32u4::InstHandler::getRd2_c_arr(uint16_t word) {//Rd 3Bit continous add 16
	const uint8_t arr[] = { 24,26,28,30 };
	return arr[((word & 0x0030) >> 4)];
}
uint8_t A32u4::InstHandler::getRd3_c_a16(uint16_t word) {//Rd 3Bit continous add 16
	return ((word & 0x0070) >> 4) + 16;
}
uint8_t A32u4::InstHandler::getRr3_c_a16(uint16_t word) {//Rr 3Bit continous add 16
	return (word & 0x0007) + 16;
}
uint8_t A32u4::InstHandler::getRd4_c(uint16_t word) {//Rd 4Bit continous add 16
	return ((word & 0x00F0) >> 4);
}
uint8_t A32u4::InstHandler::getRr4_c(uint16_t word) {//Rr 4Bit continous add 16
	return (word & 0x000F);
}
uint8_t A32u4::InstHandler::getRd4_c_a16(uint16_t word) {//Rd 4Bit continous add 16
	return getRd4_c(word) + 16;
}
uint8_t A32u4::InstHandler::getRr4_c_a16(uint16_t word) {//Rr 4Bit continous add 16
	return getRr4_c(word) + 16;
}
uint8_t A32u4::InstHandler::getRd4_c_m2(uint16_t word) {//Rd 4Bit continous add 16
	return getRd4_c(word) * 2;
}
uint8_t A32u4::InstHandler::getRr4_c_m2(uint16_t word) {//Rr 4Bit continous add 16
	return getRr4_c(word) * 2;
}
uint8_t A32u4::InstHandler::getRd5_c(uint16_t word) { //Rd 5Bit continous 
	return (word & 0b0000000111110000) >> 4;
}
uint8_t A32u4::InstHandler::getRr5_c(uint16_t word) { //Rr 5Bit continous 
	return ((word & 0b0000001000000000) >> 5) | (word & 0b0000000000001111);
}
uint8_t A32u4::InstHandler::getK6_d24(uint16_t word) {//K val 8Bit distributed in 4chunk 4chunk
	return ((word & 0x00C0) >> 2) | (word & 0x000F);
}
uint8_t A32u4::InstHandler::getK8_d44(uint16_t word) {//K val 8Bit distributed in 4chunk 4chunk
	return ((word & 0x0F00) >> 4) | (word & 0x000F);
}
uint8_t A32u4::InstHandler::getk7_c_sin(uint16_t word) {//k val 12Bit continuous signed
	return (int8_t)convTo16BitInt((word & 0b1111111000) >> 3, 7);
}
int16_t A32u4::InstHandler::getk12_c_sin(uint16_t word) {//k val 12Bit continuous signed
	return convTo16BitInt(word & 0xFFF, 12);
}
uint8_t A32u4::InstHandler::getb3_c(uint16_t word) {//b val 3Bit continuous
	return word & 0b111;
}
uint8_t A32u4::InstHandler::getA5_c(uint16_t word) {//A val 5Bit continuous
	return (word & 0b11111000) >> 3;
}
uint8_t A32u4::InstHandler::gets3_c(uint16_t word) {//s val 3Bit continuous
	return (word & 0b1110000) >> 4;
}
uint8_t A32u4::InstHandler::getq6_d123(uint16_t word) {//q val 6Bit distributed in 1chunk 2chunk 3chunk
	return ((word & 0x2000) >> 5) | ((word & 0x0C00) >> 7) | (word & 0x0007);
}
uint8_t A32u4::InstHandler::getA6_d24(uint16_t word) {//A val 6Bit distributed in 2chunk 4chunk ____ _AA_ ____ AAAA
	return ((word & 0x0600) >> (4 + 1)) | word & 0x000F;
}

uint32_t A32u4::InstHandler::getLongAddr(uint16_t word1, uint16_t word2) {
	return (((uint32_t)(word1 & 0b0000000111110000)) << 13) | (((uint32_t)(word1 & 0b0000000000000001)) << 16) | word2; //could just use word2, but idk
}
uint32_t A32u4::InstHandler::getExtendedZ() {
	return ((uint32_t)mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::RAMPZ) << 16) | mcu->dataspace.getZ();
}

bool A32u4::InstHandler::is2WordInst(uint16_t word) {
	return is2WordInstNew(word);
}
bool A32u4::InstHandler::is2WordInstOld(uint16_t word) {
	uint8_t inds[] = { IND_LDS, IND_STS, IND_JMP, IND_CALL }; //{28,32,85,97};

	for (uint8_t i = 0; i < 4; i++) {
		uint8_t ind = inds[i];
		if ((word & instList[ind].mask) == instList[ind].res) {
			return true;
		}
	}

	return false;
}
bool A32u4::InstHandler::is2WordInstNew(uint16_t word) {
	const uint16_t all = instList[IND_LDS].mask & instList[IND_STS].mask & instList[IND_JMP].mask & instList[IND_CALL].mask;

	if ((word & all) != all) {
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

void A32u4::InstHandler::setPC_Cycs_Skip(bool cond) {
	if (cond) {
		if (!is2WordInst(mcu->flash.getInst(mcu->cpu.PC + 1))) {
			cycs = 2; PC_add = 2;
		}
		else {
			cycs = 3; PC_add = 3;
		}
	}
	else {
		cycs = 1; PC_add = 1;
	}
}

void A32u4::InstHandler::INST_ADD(uint16_t word) { //0000 11rd dddd rrrr
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t Rd_copy = Rd;
	const uint8_t Rr_copy = Rr;

	Rd += Rr;
	mcu->cpu.setFlags_HSVNZC_ADD(Rd_copy, Rr_copy, 0, Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ADC(uint16_t word) { //0001 11rd dddd rrrr
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t Rd_copy = Rd;
	const uint8_t Rr_copy = Rr;

	uint8_t C = mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C);

	Rd += Rr + C;
	mcu->cpu.setFlags_HSVNZC_ADD(Rd_copy, Rr_copy, C, Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ADIW(uint16_t word) { //1001 0110 KKdd KKKK
	const uint8_t d = getRd2_c_arr(word);
	const uint8_t K = getK6_d24(word);

	uint16_t R16 = mcu->dataspace.getWordReg(d);
	const uint16_t R16_copy = R16;
	R16 += K;
	mcu->dataspace.setWordReg(d, R16);

	mcu->cpu.setFlags_SVNZC_ADD_16(R16_copy,K,R16);
	cycs = 2; PC_add = 1;
}

void A32u4::InstHandler::INST_SUB(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t Rd_copy = Rd;
	const uint8_t Rr_copy = Rr;

	Rd -= Rr;
	mcu->cpu.setFlags_HSVNZC_SUB(Rd_copy, Rr_copy, 0, Rd,false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SUBI(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(d);
	const uint8_t Rd_copy = Rd;

	Rd -= K;
	mcu->cpu.setFlags_HSVNZC_SUB(Rd_copy, K, 0, Rd,false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SBC(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t Rd_copy = Rd;
	const uint8_t Rr_copy = Rr;

	const uint8_t C = mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C);

	Rd -= (Rr + C); //Rd = Rd - Rr - C
	mcu->cpu.setFlags_HSVNZC_SUB(Rd_copy, Rr_copy, C, Rd, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SBCI(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(d);
	const uint8_t Rd_copy = Rd;

	const uint8_t C = mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C);

	Rd -= (K + C); //Rd = Rd - K - C
	mcu->cpu.setFlags_HSVNZC_SUB(Rd_copy, K, C, Rd,true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SBIW(uint16_t word) {
	const uint8_t d = getRd2_c_arr(word);
	const uint8_t K = getK6_d24(word);

	const uint16_t R16 = mcu->dataspace.getWordReg(d);
	//uint16_t R16_copy = R16;
	//R16 -= K;
	const uint16_t R16_res = R16 - K;
	//mcu->dataspace.setWordReg(d, R16);
	mcu->dataspace.setWordReg(d, R16_res);

	//mcu->cpu.setFlags_SVNZC_SUB_16(R16_copy, K, R16);
	mcu->cpu.setFlags_SVNZC_SUB_16(R16, K, R16_res);
	cycs = 2; PC_add = 1;
}

void A32u4::InstHandler::INST_AND(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	Rd &= Rr;
	mcu->cpu.setFlags_SVNZ(Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ANDI(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(d);

	Rd &= K;
	mcu->cpu.setFlags_SVNZ(Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_OR(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	Rd |= Rr;
	mcu->cpu.setFlags_SVNZ(Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ORI(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(d);

	Rd |= K;
	mcu->cpu.setFlags_SVNZ(Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_EOR(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	Rd ^= Rr;
	mcu->cpu.setFlags_SVNZ(Rd);
	cycs = 1; PC_add = 1;
}

void A32u4::InstHandler::INST_COM(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);

	Rd = 0xFF - Rd;
	mcu->cpu.setFlags_SVNZC(Rd);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_NEG(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t Rd_copy = Rd;

	Rd = 0x00 - Rd;
	mcu->cpu.setFlags_NZ(Rd);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_H, isBitSet(Rd,3) || !isBitSet(Rd_copy,3));
	const bool V = Rd == 0x80;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	const bool N = (Rd & 0b10000000) != 0;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, N ^ V);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, (Rd != 0));

	cycs = 1; PC_add = 1;
}

void A32u4::InstHandler::INST_INC(uint16_t word) {
	uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);

	Rd++;

	mcu->cpu.setFlags_NZ(Rd);
	bool V = Rd == 0x80;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	bool N = (Rd & 0b10000000) != 0;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, N ^ V);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_DEC(uint16_t word) {
	uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);

	Rd--;

	mcu->cpu.setFlags_NZ(Rd);
	bool V = (Rd ^ 0b10000000) == 0xFF;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	bool N = (Rd & 0b10000000) != 0;
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, N ^ V);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SER(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	uint8_t& Rd = mcu->dataspace.getGPRegRef(d);

	Rd = 0xFF;

	cycs = 1; PC_add = 1;
}

void A32u4::InstHandler::INST_MUL(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	const uint16_t res = (uint16_t)Rd * (uint16_t)Rr;
	R0 = (uint8_t)res;
	R1 = (uint8_t)(res >> 8);
	
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1,7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 == 0) && (R1 == 0));
	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_MULS(uint16_t word) {
	uint8_t Rd_id = getRd4_c_a16(word);
	uint8_t Rr_id = getRr4_c_a16(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	int16_t res = (int16_t)Rd * (int16_t)Rr;
	R0 = (uint8_t)res;
	R1 = (uint8_t)(res >> 8);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1, 7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 == 0) && (R1 == 0));
	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_MULSU(uint16_t word) {
	uint8_t Rd_id = getRd3_c_a16(word);
	uint8_t Rr_id = getRr3_c_a16(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	int16_t res = (int16_t)Rd * (uint16_t)Rr;
	R0 = (uint8_t)res;
	R1 = (uint8_t)((uint16_t)res >> 8);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1, 7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 == 0) && (R1 == 0));
	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_FMUL(uint16_t word) {
	uint8_t Rd_id = getRd3_c_a16(word);
	uint8_t Rr_id = getRr3_c_a16(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	uint16_t res = (uint16_t)Rd * (uint16_t)Rr;
	res <<= 1;
	R0 = (uint8_t)res;
	R1 = (uint8_t)((uint16_t)res >> 8);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1, 7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 == 0) && (R1 == 0));
	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_FMULS(uint16_t word) {
	uint8_t Rd_id = getRd4_c_a16(word);
	uint8_t Rr_id = getRr4_c_a16(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	int16_t res = (int16_t)Rd * (int16_t)Rr;
	res <<= 1;
	R0 = (uint8_t)res;
	R1 = (uint8_t)(res >> 8);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1, 7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 != 0) && (R1 != 0));
	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_FMULSU(uint16_t word) {
	uint8_t Rd_id = getRd3_c_a16(word);
	uint8_t Rr_id = getRr3_c_a16(word);

	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	uint8_t& R1 = mcu->dataspace.getGPRegRef(1);

	int16_t res = (int16_t)Rd * (uint16_t)Rr;
	res <<= 1;
	R0 = (uint8_t)res;
	R1 = (uint8_t)((uint16_t)res >> 8);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, isBitSet(R1, 7));
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, (R0 == 0) && (R1 == 0));
	cycs = 2; PC_add = 1;
}

void A32u4::InstHandler::INST_RJMP(uint16_t word) {
	int16_t k = getk12_c_sin(word);
	cycs = 2; PC_add = k+1;
}
void A32u4::InstHandler::INST_IJMP(uint16_t word) {
	mcu->cpu.PC = mcu->dataspace.getZ();
	cycs = 2; PC_add = 0;
}
void A32u4::InstHandler::INST_EIJMP(uint16_t word) {
	mcu->cpu.PC = mcu->dataspace.getZ(); //PC should have bits 21:16 set to reg EIND but PC is only 16bit and EIND doesnt seem to exist, soooo
	cycs = 2; PC_add = 0;
}
void A32u4::InstHandler::INST_JMP(uint16_t word) {
	uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	uint32_t k = getLongAddr(word,word2); 

	mcu->cpu.PC = k; 

	cycs = 3; PC_add = 0;
}
void A32u4::InstHandler::INST_RCALL(uint16_t word) {
	int16_t k = getk12_c_sin(word);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC+1);

	mcu->debugger.pushAddrOnAddressStack(mcu->cpu.PC+k+1, mcu->cpu.PC);
	
	cycs = 4; PC_add = k+1;
}
void A32u4::InstHandler::INST_ICALL(uint16_t word) {
	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 1);

	uint16_t addr = mcu->dataspace.getZ();

	mcu->debugger.pushAddrOnAddressStack(addr, mcu->cpu.PC);

	mcu->cpu.PC = addr;

	cycs = 4; PC_add = 0;
}
void A32u4::InstHandler::INST_EICALL(uint16_t word) {
	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 1);

	uint16_t addr = mcu->dataspace.getZ();

	mcu->debugger.pushAddrOnAddressStack(addr, mcu->cpu.PC);

	mcu->cpu.PC = addr;

	cycs = 4; PC_add = 0;
}
void A32u4::InstHandler::INST_CALL(uint16_t word) {
	uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	uint32_t k = getLongAddr(word, word2);

	mcu->dataspace.pushAddrToStack(mcu->cpu.PC + 2);

	mcu->debugger.pushAddrOnAddressStack(k, mcu->cpu.PC);

	mcu->cpu.PC = k;

	cycs = 5; PC_add = 0;
}
void A32u4::InstHandler::INST_RET(uint16_t word) {
	uint16_t addr = mcu->dataspace.popAddrFromStack();
	mcu->cpu.PC = addr;

	mcu->debugger.popAddrFromAddressStack();

	cycs = 5; PC_add = 0;
}
void A32u4::InstHandler::INST_RETI(uint16_t word) {
	uint16_t addr = mcu->dataspace.popAddrFromStack();
	mcu->cpu.PC = addr;

	mcu->dataspace.getByteRefAtAddr(DataSpace::Consts::SREG) |= (1 << DataSpace::Consts::SREG_I);//mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_I, true);

	mcu->debugger.popAddrFromAddressStack();

	mcu->cpu.insideInterrupt = false;

	cycs = 5; PC_add = 0;
}

void A32u4::InstHandler::INST_CPSE(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	setPC_Cycs_Skip(Rd == Rr);
}
void A32u4::InstHandler::INST_CP(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	
	const uint8_t res = Rd - Rr;
	mcu->cpu.setFlags_HSVNZC_SUB(Rd, Rr, 0, res,false);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CPC(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	const uint8_t C = mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C);

	const uint8_t res = Rd - (Rr+C);
	mcu->cpu.setFlags_HSVNZC_SUB(Rd, Rr, C, res,true);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CPI(uint16_t word) {
	const uint8_t d = getRd4_c_a16(word); //R16 to R31
	const uint8_t K = getK8_d44(word);

	const uint8_t& Rd = mcu->dataspace.getGPRegRef(d);

	const uint8_t res = Rd - K;
	mcu->cpu.setFlags_HSVNZC_SUB(Rd, K, 0, res, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SBRC(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t b = getb3_c(word);

	setPC_Cycs_Skip((Rr & (1<<b)) == 0);
}
void A32u4::InstHandler::INST_SBRS(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t b = getb3_c(word);

	setPC_Cycs_Skip((Rr & (1 << b)) != 0);
}
void A32u4::InstHandler::INST_SBIC(uint16_t word) {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	const uint8_t ioReg = mcu->dataspace.getIOAt(A);
	//uint8_t ioReg = mcu->dataspace.data[DataSpace::Consts::io_start + A];
	setPC_Cycs_Skip((ioReg & (1 << b)) == 0);
}
void A32u4::InstHandler::INST_SBIS(uint16_t word) {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	const uint8_t ioReg = mcu->dataspace.getIOAt(A);
	//uint8_t ioReg = mcu->dataspace.data[DataSpace::Consts::io_start + A];
	setPC_Cycs_Skip((ioReg & (1 << b)) != 0);
}
void A32u4::InstHandler::INST_BRBS(uint16_t word) {
	int8_t k = (int8_t)getk7_c_sin(word);
	uint8_t s = getb3_c(word);

	if (mcu->dataspace.getRegBit(DataSpace::Consts::SREG, s)) {
		cycs = 2; PC_add = k + 1;
	}else {
		cycs = 1; PC_add = 1;
	}
}
void A32u4::InstHandler::INST_BRBC(uint16_t word) {
	int8_t k = (int8_t)getk7_c_sin(word);
	uint8_t s = getb3_c(word);

	if (mcu->dataspace.getRegBit(DataSpace::Consts::SREG, s) == false) {
		cycs = 2; PC_add = k + 1;
	}
	else {
		cycs = 1; PC_add = 1;
	}
}

void A32u4::InstHandler::INST_SBI(uint16_t word) {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	uint8_t io = mcu->dataspace.getIOAt(A);
	io |= 1 << b;
	
	//mcu->dataspace.setRegBit(mcu->dataspace.data[DataSpace::Consts::io_start + A], b, true);
	cycs = 2 + mcu->dataspace.setIOAt(A, io);
	PC_add = 1;
}
void A32u4::InstHandler::INST_CBI(uint16_t word) {
	const uint8_t A = getA5_c(word);
	const uint8_t b = getb3_c(word);

	//mcu->dataspace.setRegBit(mcu->dataspace.data[DataSpace::Consts::io_start + A], b, false);

	uint8_t io = mcu->dataspace.getIOAt(A);
	io &= ~(1 << b);

	cycs = 2 + mcu->dataspace.setIOAt(A, io);
	PC_add = 1;
}
void A32u4::InstHandler::INST_LSR(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t Rd_copy = Rd;

	Rd >>= 1;

	bool C = Rd_copy&0b1;
	bool N = 0;
	bool V = N ^ C;
	bool S = N ^ V;

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, S);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_N, N);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, Rd == 0);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, C);
	
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ROR(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t Rd_copy = Rd;

	Rd >>= 1;
	Rd |= mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C) << 7;

	bool C = Rd_copy & 0b1;
	bool N = isBitSet(Rd,7);
	bool V = N ^ C;
	bool S = N ^ V;

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, S);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_N, N);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, Rd == 0);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, C);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_ASR(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t Rd_copy = Rd;

	Rd = (int8_t)Rd >> 1; //hopyfully this works
	//Rd |= Rd_copy & 0b10000000;

	bool C = Rd_copy & 0b1;
	bool N = isBitSet(Rd, 7);
	bool V = N ^ C;
	bool S = N ^ V;

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, S);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, V);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_N, N);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, Rd == 0);
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, C);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SWAP(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t Rd_copy = Rd;

	Rd = ((Rd_copy&0xF0)>>4) | ((Rd_copy&0xF)<<4);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_BSET(uint16_t word) {
	const uint8_t s = gets3_c(word);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, s, true);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_BCLR(uint16_t word) {
	const uint8_t s = gets3_c(word);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, s, false);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_BST(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t b = getb3_c(word);

	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_T, isBitSet(Rd,b));

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_BLD(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t b = getb3_c(word);

	const bool T = mcu->dataspace.getRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_T);
	if (T) {
		Rd |= 1 << b;
	}
	else {
		Rd &= ~(1 << b);
	}
	

	cycs = 1; PC_add = 1;
}

void A32u4::InstHandler::INST_SEC(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLC(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_C, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SEN(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_N, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLN(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_N, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SEZ(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLZ(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_Z, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SEI(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_I, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLI(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_I, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SES(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLS(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_S, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SEV(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLV(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_V, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SET(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_T, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLT(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_T, false);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SEH(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_H, true);
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_CLH(uint16_t word) {
	mcu->dataspace.setRegBit(DataSpace::Consts::SREG, DataSpace::Consts::SREG_H, false);
	cycs = 1; PC_add = 1;
}


void A32u4::InstHandler::INST_MOV(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t Rr_id = getRr5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	Rd = Rr;

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_MOVW(uint16_t word) {
	const uint8_t Rd_id = getRd4_c_m2(word);
	const uint8_t Rr_id = getRr4_c_m2(word);

	mcu->dataspace.getGPRegRef(Rd_id) = mcu->dataspace.getGPRegRef(Rr_id);
	mcu->dataspace.getGPRegRef(Rd_id+1) = mcu->dataspace.getGPRegRef(Rr_id+1);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_LDI(uint16_t word) {
	const uint8_t Rd_id = getRd4_c_a16(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t K = getK8_d44(word);

	Rd = K;

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_X(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getX();

	Rd = mcu->dataspace.getByteAt(Addr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_XpostInc(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getX();

	Rd = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setX(Addr + 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_XpreDec(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getX();

	Rd = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setX(Addr - 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_Y(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getY();

	Rd = mcu->dataspace.getByteAt(Addr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_YpostInc(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getY();

	Rd = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setY(Addr + 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_YpreDec(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getY();

	Rd = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setY(Addr - 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LDD_Y(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getY() + q;

	Rd = mcu->dataspace.getByteAt(Addr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_Z(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	Rd = mcu->dataspace.getByteAt(Addr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_ZpostInc(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	Rd = mcu->dataspace.getByteAt(Addr);

	mcu->dataspace.setZ(Addr + 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LD_ZpreDec(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	Rd = mcu->dataspace.getByteAt(Addr - 1);

	mcu->dataspace.setZ(Addr - 1);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LDD_Z(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getZ() + q;

	Rd = mcu->dataspace.getByteAt(Addr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_LDS(uint16_t word) {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t k = word2;

	Rd = mcu->dataspace.getByteAt(k);

	cycs = 2; PC_add = 2;
}
void A32u4::InstHandler::INST_ST_X(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();
	
	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);
	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_XpostInc(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setX(Addr + 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_XpreDec(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getX();

	cycs = 2 + mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setX(Addr - 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_Y(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_YpostInc(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setY(Addr + 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_YpreDec(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getY();

	cycs = 2 + mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setY(Addr - 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_STD_Y(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getY() + q;

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_Z(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_ZpostInc(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	mcu->dataspace.setZ(Addr + 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_ST_ZpreDec(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	cycs = 2 + mcu->dataspace.setByteAt(Addr - 1, Rr);

	mcu->dataspace.setZ(Addr - 1);

	PC_add = 1;
}
void A32u4::InstHandler::INST_STD_Z(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t q = getq6_d123(word);
	const uint16_t Addr = mcu->dataspace.getZ() + q;

	cycs = 2 + mcu->dataspace.setByteAt(Addr, Rr);

	PC_add = 1;
}
void A32u4::InstHandler::INST_STS(uint16_t word) {
	const uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);
	const uint8_t Rd_id = getRd5_c(word);
	const uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint16_t k = word2;

	cycs = 2 + mcu->dataspace.setByteAt(k, Rd);

	PC_add = 2;
}
void A32u4::InstHandler::INST_LPM_0(uint16_t word) {
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	const uint16_t Addr = mcu->dataspace.getZ();

	R0 = mcu->flash.getByte(Addr);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_LPM_d(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	Rr = mcu->flash.getByte(Addr);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_LPM_dpostInc(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint16_t Addr = mcu->dataspace.getZ();

	Rr = mcu->flash.getByte(Addr);

	mcu->dataspace.setZ(Addr + 1);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_ELPM_0(uint16_t word) {
	uint8_t& R0 = mcu->dataspace.getGPRegRef(0);
	const uint32_t Addr = getExtendedZ();

	R0 = mcu->flash.getByte(Addr);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_ELPM_d(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint32_t Addr = getExtendedZ();

	Rr = mcu->flash.getByte(Addr);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_ELPM_dpostInc(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint32_t Addr = getExtendedZ();

	Rr = mcu->flash.getByte(Addr);

	mcu->dataspace.setZ(Addr + 1);

	cycs = 3; PC_add = 1;
}
void A32u4::InstHandler::INST_SPM(uint16_t word) {
	//TODO

	cycs = 0; PC_add = 1;
}
void A32u4::InstHandler::INST_IN(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);
	const uint8_t A = getA6_d24(word);

	Rd = mcu->dataspace.getIOAt(A);

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_OUT(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);
	const uint8_t A = getA6_d24(word);

	cycs = 1 + mcu->dataspace.setIOAt(A, Rr);
	PC_add = 1;
}
void A32u4::InstHandler::INST_PUSH(uint16_t word) {
	const uint8_t Rr_id = getRd5_c(word);
	const uint8_t& Rr = mcu->dataspace.getGPRegRef(Rr_id);

	mcu->dataspace.pushByteToStack(Rr);

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_POP(uint16_t word) {
	const uint8_t Rd_id = getRd5_c(word);
	uint8_t& Rd = mcu->dataspace.getGPRegRef(Rd_id);

	Rd = mcu->dataspace.popByteFromStack();

	cycs = 2; PC_add = 1;
}
void A32u4::InstHandler::INST_NOP(uint16_t word) {
	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_SLEEP(uint16_t word) {
	//TODO
	uint8_t SMCR_val = mcu->dataspace.getByteAt(DataSpace::Consts::SMCR);
	if (SMCR_val & 0b1) { //if SE (sleep enable) is set
		switch ((SMCR_val >> 1) & 0b111) {
		case 0: //idle
			mcu->cpu.CPU_sleep = true;
			mcu->cpu.breakOutOfOptim = true;
			break;
		default:
			break;
		}
	}
	

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_WDR(uint16_t word) {
	//TODO

	cycs = 1; PC_add = 1;
}
void A32u4::InstHandler::INST_BREAK(uint16_t word) {
	//TODO
	mcu->debugger.halt();

	cycs = 1; PC_add = 1;
}

/*

const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"          },//0000 11rd dddd rrrr
	{&INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"          },//0001 11rd dddd rrrr
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"         },//1001 0110 KKdd KKKK
	{&INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"          },//0001 10rd dddd rrrr
	{&INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"         },//0101 KKKK dddd KKKK
	{&INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"          },//0000 10rd dddd rrrr
	{&INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"         },//0100 KKKK dddd KKKK
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"         },//1001 0111 KKdd KKKK
	{&INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"          },//0010 00rd dddd rrrr
	{&INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"         },//0111 KKKK dddd KKKK
	{&INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"           },//0010 10rd dddd rrrr
	{&INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"          },//0110 KKKK dddd KKKK
	{&INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"          },//0010 01rd dddd rrrr
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"          },//1001 010d dddd 0000
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"          },//1001 010d dddd 0001
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"          },//1001 010d dddd 0011
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"          },//1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"          },//1001 11rd dddd rrrr
	{&INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"         },//0000 0010 dddd rrrr
	{&INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"        },//0000 0011 0ddd 0rrr
	{&INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"         },//0000 0011 0ddd 1rrr
	{&INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"        },//0000 0011 1ddd 0rrr
	{&INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU"       },//0000 0011 1ddd 1rrr
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"         },//1100 kkkk kkkk kkkk
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"         },//1001 0100 0000 1001
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"        },//1001 0100 0001 1001
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"          },//1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"        },//1101 kkkk kkkk kkkk
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"        },//1001 0101 0000 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL"       },//1001 0101 0001 1001
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"         },//1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"          },//1001 0101 0000 1000
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"         },//1001 0101 0001 1000
	{&INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"         },//0001 00rd dddd rrrr
	{&INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"           },//0001 01rd dddd rrrr
	{&INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"          },//0000 01rd dddd rrrr
	{&INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"          },//0011 KKKK dddd KKKK
	{&INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"         },//1111 110r rrrr 0bbb
	{&INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"         },//1111 111r rrrr 0bbb
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"         },//1001 1001 AAAA Abbb
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"         },//1001 1011 AAAA Abbb
	{&INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"         },//1111 00kk kkkk ksss
	{&INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"         },//1111 01kk kkkk ksss
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"          },//1001 1010 AAAA Abbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"          },//1001 1000 AAAA Abbb
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"          },//1001 010d dddd 0110
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"          },//1001 010d dddd 0111
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"          },//1001 010d dddd 0101
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"         },//1001 010d dddd 0010
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"         },//1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"         },//1001 0100 1sss 1000
	{&INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"          },//1111 101d dddd 0bbb
	{&INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"          },//1111 100d dddd 0bbb
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"          },//1001 0100 0000 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"          },//1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"          },//1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"          },//1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"          },//1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"          },//1001 0100 1001 1000
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"          },//1001 0100 0111 1000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"          },//1001 0100 1111 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"          },//1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"          },//1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"          },//1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"          },//1001 0100 1011 1000
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"          },//1001 0100 0110 1000
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"          },//1001 0100 1110 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"          },//1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"          },//1001 0100 1101 1000
	{&INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"          },//0010 11rd dddd rrrr
	{&INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"         },//0000 0001 dddd rrrr
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"          },//1110 KKKK dddd KKKK
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD_X"         },//1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD_XpostInc"  },//1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD_XpreDec"   },//1001 000d dddd 1110
	{&INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD_Y"         },//1000 000d dddd 1000
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD_YpostInc"  },//1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD_YpreDec"   },//1001 000d dddd 1010
	{&INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD_Y"        },//10q0 qq0d dddd 1qqq
	{&INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD_Z"         },//1000 000d dddd 0000
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD_ZpostInc"  },//1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD_ZpreDec"   },//1001 000d dddd 0010
	{&INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD_Z"        },//10q0 qq0d dddd 0qqq
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"          },//1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST_X"         },//1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST_XpostInc"  },//1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST_XpreDec"   },//1001 001r rrrr 1110
	{&INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST_Y"         },//1000 001r rrrr 1000
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST_YpostInc"  },//1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST_YpreDec"   },//1001 001r rrrr 1010
	{&INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD_Y"        },//10q0 qq1r rrrr 1qqq
	{&INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST_Z"         },//1000 001r rrrr 0000
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST_ZpostInc"  },//1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST_ZpreDec"   },//1001 001r rrrr 0010
	{&INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD_Z"        },//10q0 qq1r rrrr 0qqq
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"          },//1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM_0"        },//1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM_d"        },//1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM_dpostInc" },//1001 000d dddd 0101
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM_0"       },//1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM_d"       },//1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM_dpostInc"},//1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"          },//1001 0101 1110 1000
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"           },//1011 0AAd dddd AAAA
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"          },//1011 1AAr rrrr AAAA
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"         },//1001 001d dddd 1111
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"          },//1001 000d dddd 1111
	{&INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"          },//0000 0000 0000 0000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"        },//1001 0101 1000 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"          },//1001 0101 1010 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"        },//1001 0101 1001 1000

};







const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"          },//1110 KKKK dddd KKKK
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"          },//1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"         },//1100 kkkk kkkk kkkk
	{&INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"         },//0000 0001 dddd rrrr
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"          },//1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"          },//1001 000d dddd 1111
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"         },//1001 001d dddd 1111
	{&INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"          },//0010 11rd dddd rrrr
	{&INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"         },//1111 00kk kkkk ksss
	{&INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"         },//1111 01kk kkkk ksss
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"         },//1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"          },//0000 11rd dddd rrrr
	{&INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"          },//0001 11rd dddd rrrr
	{&INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"          },//0011 KKKK dddd KKKK
	{&INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"         },//0101 KKKK dddd KKKK
	{&INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD_Y"        },//10q0 qq0d dddd 1qqq
	{&INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD_Z"        },//10q0 qq0d dddd 0qqq
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD_X"         },//1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD_XpostInc"  },//1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD_XpreDec"   },//1001 000d dddd 1110
	{&INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD_Y"         },//1000 000d dddd 1000
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD_YpostInc"  },//1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD_YpreDec"   },//1001 000d dddd 1010
	{&INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD_Z"         },//1000 000d dddd 0000
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD_ZpostInc"  },//1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD_ZpreDec"   },//1001 000d dddd 0010
	{&INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"          },//0000 01rd dddd rrrr
	{&INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"          },//0010 00rd dddd rrrr
	{&INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"          },//0010 01rd dddd rrrr
	{&INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"         },//0100 KKKK dddd KKKK
	{&INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD_Y"        },//10q0 qq1r rrrr 1qqq
	{&INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD_Z"        },//10q0 qq1r rrrr 0qqq
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"          },//1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"          },//1001 0101 0000 1000
	{&INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"           },//0001 01rd dddd rrrr
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"         },//1001 0110 KKdd KKKK
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"          },//1011 1AAr rrrr AAAA
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"           },//1011 0AAd dddd AAAA
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"          },//1001 010d dddd 0111
	{&INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"          },//0110 KKKK dddd KKKK
	{&INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"         },//0001 00rd dddd rrrr
	{&INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"          },//0000 10rd dddd rrrr
	{&INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"         },//0111 KKKK dddd KKKK
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"          },//1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"          },//1001 11rd dddd rrrr
	{&INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"          },//0001 10rd dddd rrrr
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM_0"        },//1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM_d"        },//1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM_dpostInc" },//1001 000d dddd 0101
	{&INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"         },//1111 111r rrrr 0bbb
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"         },//1001 0111 KKdd KKKK
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"          },//1001 010d dddd 0110
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST_X"         },//1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST_XpostInc"  },//1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST_XpreDec"   },//1001 001r rrrr 1110
	{&INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST_Y"         },//1000 001r rrrr 1000
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST_YpostInc"  },//1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST_YpreDec"   },//1001 001r rrrr 1010
	{&INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST_Z"         },//1000 001r rrrr 0000
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST_ZpostInc"  },//1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST_ZpreDec"   },//1001 001r rrrr 0010
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"          },//1001 1010 AAAA Abbb
	{&INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"         },//1111 110r rrrr 0bbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"          },//1001 1000 AAAA Abbb
	{&INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"           },//0010 10rd dddd rrrr
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"          },//1001 010d dddd 0000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"          },//1001 0100 1111 1000
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"          },//1001 010d dddd 0011
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"          },//1001 010d dddd 0001
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"          },//1001 010d dddd 0101
	{&INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"        },//1101 kkkk kkkk kkkk
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"        },//1001 0101 0000 1001
	{&INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"          },//1111 101d dddd 0bbb
	{&INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"          },//1111 100d dddd 0bbb
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"         },//1001 1011 AAAA Abbb
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"         },//1001 0101 0001 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"          },//1001 0101 1010 1000
	{&INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"          },//0000 0000 0000 0000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"        },//1001 0101 1000 1000
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"         },//1001 1001 AAAA Abbb
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"          },//1001 0100 1110 1000
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"         },//1001 010d dddd 0010
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"          },//1001 0100 0111 1000
	{&INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"        },//0000 0011 0ddd 0rrr
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"          },//1001 0100 0110 1000
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"          },//1001 0100 0000 1000
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"         },//1001 0100 0000 1001
	{&INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"         },//0000 0010 dddd rrrr
	{&INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"         },//0000 0011 0ddd 1rrr
	{&INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"        },//0000 0011 1ddd 0rrr
	{&INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU"       },//0000 0011 1ddd 1rrr
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"        },//1001 0100 0001 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL"       },//1001 0101 0001 1001
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"         },//1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"         },//1001 0100 1sss 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"          },//1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"          },//1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"          },//1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"          },//1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"          },//1001 0100 1001 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"          },//1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"          },//1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"          },//1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"          },//1001 0100 1011 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"          },//1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"          },//1001 0100 1101 1000
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM_0"       },//1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM_d"       },//1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM_dpostInc"},//1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"          },//1001 0101 1110 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"        },//1001 0101 1001 1000

};




const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"          },//1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"          },//1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"          },//1001 000d dddd 1111
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"         },//1001 001d dddd 1111
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"         },//1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD_Y"        },//10q0 qq0d dddd 1qqq
	{&INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD_Z"        },//10q0 qq0d dddd 0qqq
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD_X"         },//1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD_XpostInc"  },//1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD_XpreDec"   },//1001 000d dddd 1110
	{&INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD_Y"         },//1000 000d dddd 1000
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD_YpostInc"  },//1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD_YpreDec"   },//1001 000d dddd 1010
	{&INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD_Z"         },//1000 000d dddd 0000
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD_ZpostInc"  },//1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD_ZpreDec"   },//1001 000d dddd 0010
	{&INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD_Y"        },//10q0 qq1r rrrr 1qqq
	{&INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD_Z"        },//10q0 qq1r rrrr 0qqq
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"          },//1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"          },//1001 0101 0000 1000
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"         },//1001 0110 KKdd KKKK
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"          },//1011 1AAr rrrr AAAA
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"           },//1011 0AAd dddd AAAA
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"          },//1001 010d dddd 0111
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"          },//1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"          },//1001 11rd dddd rrrr
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM_0"        },//1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM_d"        },//1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM_dpostInc" },//1001 000d dddd 0101
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"         },//1001 0111 KKdd KKKK
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"          },//1001 010d dddd 0110
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST_X"         },//1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST_XpostInc"  },//1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST_XpreDec"   },//1001 001r rrrr 1110
	{&INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST_Y"         },//1000 001r rrrr 1000
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST_YpostInc"  },//1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST_YpreDec"   },//1001 001r rrrr 1010
	{&INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST_Z"         },//1000 001r rrrr 0000
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST_ZpostInc"  },//1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST_ZpreDec"   },//1001 001r rrrr 0010
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"          },//1001 1010 AAAA Abbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"          },//1001 1000 AAAA Abbb
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"          },//1001 010d dddd 0000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"          },//1001 0100 1111 1000
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"          },//1001 010d dddd 0011
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"          },//1001 010d dddd 0001
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"          },//1001 010d dddd 0101
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"        },//1001 0101 0000 1001
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"         },//1001 1011 AAAA Abbb
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"         },//1001 0101 0001 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"          },//1001 0101 1010 1000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"        },//1001 0101 1000 1000
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"         },//1001 1001 AAAA Abbb
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"          },//1001 0100 1110 1000
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"         },//1001 010d dddd 0010
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"          },//1001 0100 0111 1000
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"          },//1001 0100 0110 1000
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"          },//1001 0100 0000 1000
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"         },//1001 0100 0000 1001
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"        },//1001 0100 0001 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL"       },//1001 0101 0001 1001
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"         },//1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"         },//1001 0100 1sss 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"          },//1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"          },//1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"          },//1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"          },//1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"          },//1001 0100 1001 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"          },//1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"          },//1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"          },//1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"          },//1001 0100 1011 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"          },//1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"          },//1001 0100 1101 1000
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM_0"       },//1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM_d"       },//1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM_dpostInc"},//1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"          },//1001 0101 1110 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"        },//1001 0101 1001 1000
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"          },//1110 KKKK dddd KKKK
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"         },//1100 kkkk kkkk kkkk
	{&INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"         },//1111 00kk kkkk ksss
	{&INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"         },//1111 01kk kkkk ksss
	{&INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"         },//1111 111r rrrr 0bbb
	{&INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"         },//1111 110r rrrr 0bbb
	{&INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"        },//1101 kkkk kkkk kkkk
	{&INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"          },//1111 101d dddd 0bbb
	{&INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"          },//1111 100d dddd 0bbb
	{&INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"         },//0000 0001 dddd rrrr
	{&INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"          },//0010 11rd dddd rrrr
	{&INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"          },//0000 11rd dddd rrrr
	{&INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"          },//0001 11rd dddd rrrr
	{&INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"          },//0011 KKKK dddd KKKK
	{&INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"          },//0000 01rd dddd rrrr
	{&INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"          },//0010 00rd dddd rrrr
	{&INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"          },//0010 01rd dddd rrrr
	{&INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"           },//0001 01rd dddd rrrr
	{&INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"         },//0001 00rd dddd rrrr
	{&INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"          },//0000 10rd dddd rrrr
	{&INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"          },//0001 10rd dddd rrrr
	{&INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"           },//0010 10rd dddd rrrr
	{&INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"          },//0000 0000 0000 0000
	{&INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"        },//0000 0011 0ddd 0rrr
	{&INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"         },//0000 0010 dddd rrrr
	{&INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"         },//0000 0011 0ddd 1rrr
	{&INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"        },//0000 0011 1ddd 0rrr
	{&INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU"       },//0000 0011 1ddd 1rrr
	{&INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"         },//0101 KKKK dddd KKKK
	{&INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"         },//0100 KKKK dddd KKKK
	{&INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"          },//0110 KKKK dddd KKKK
	{&INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"         },//0111 KKKK dddd KKKK
};

const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"          },//1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"          },//1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"          },//1001 000d dddd 1111
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"         },//1001 001d dddd 1111
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"         },//1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD_X"         },//1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD_XpostInc"  },//1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD_XpreDec"   },//1001 000d dddd 1110
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD_YpostInc"  },//1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD_YpreDec"   },//1001 000d dddd 1010
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD_ZpostInc"  },//1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD_ZpreDec"   },//1001 000d dddd 0010
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"          },//1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"          },//1001 0101 0000 1000
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"         },//1001 0110 KKdd KKKK
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"          },//1011 1AAr rrrr AAAA
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"           },//1011 0AAd dddd AAAA
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"          },//1001 010d dddd 0111
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"          },//1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"          },//1001 11rd dddd rrrr
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM_0"        },//1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM_d"        },//1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM_dpostInc" },//1001 000d dddd 0101
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"         },//1001 0111 KKdd KKKK
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"          },//1001 010d dddd 0110
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST_X"         },//1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST_XpostInc"  },//1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST_XpreDec"   },//1001 001r rrrr 1110
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST_YpostInc"  },//1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST_YpreDec"   },//1001 001r rrrr 1010
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST_ZpostInc"  },//1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST_ZpreDec"   },//1001 001r rrrr 0010
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"          },//1001 1010 AAAA Abbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"          },//1001 1000 AAAA Abbb
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"          },//1001 010d dddd 0000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"          },//1001 0100 1111 1000
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"          },//1001 010d dddd 0011
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"          },//1001 010d dddd 0001
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"          },//1001 010d dddd 0101
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"        },//1001 0101 0000 1001
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"         },//1001 1011 AAAA Abbb
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"         },//1001 0101 0001 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"          },//1001 0101 1010 1000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"        },//1001 0101 1000 1000
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"         },//1001 1001 AAAA Abbb
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"          },//1001 0100 1110 1000
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"         },//1001 010d dddd 0010
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"          },//1001 0100 0111 1000
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"          },//1001 0100 0110 1000
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"          },//1001 0100 0000 1000
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"         },//1001 0100 0000 1001
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"        },//1001 0100 0001 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL"       },//1001 0101 0001 1001
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"         },//1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"         },//1001 0100 1sss 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"          },//1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"          },//1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"          },//1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"          },//1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"          },//1001 0100 1001 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"          },//1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"          },//1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"          },//1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"          },//1001 0100 1011 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"          },//1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"          },//1001 0100 1101 1000
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM_0"       },//1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM_d"       },//1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM_dpostInc"},//1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"          },//1001 0101 1110 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"        },//1001 0101 1001 1000
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"          },//1110 KKKK dddd KKKK
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"         },//1100 kkkk kkkk kkkk
	{&INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"         },//0000 0001 dddd rrrr
	{&INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"          },//0010 11rd dddd rrrr
	{&INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"          },//0000 11rd dddd rrrr
	{&INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"          },//0000 01rd dddd rrrr
	{&INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"          },//0010 00rd dddd rrrr
	{&INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"          },//0010 01rd dddd rrrr
	{&INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"          },//0000 10rd dddd rrrr
	{&INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"           },//0010 10rd dddd rrrr
	{&INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"          },//0000 0000 0000 0000
	{&INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"        },//0000 0011 0ddd 0rrr
	{&INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"         },//0000 0010 dddd rrrr
	{&INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"         },//0000 0011 0ddd 1rrr
	{&INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"        },//0000 0011 1ddd 0rrr
	{&INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU"       },//0000 0011 1ddd 1rrr
	{&INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"         },//1111 00kk kkkk ksss
	{&INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"         },//1111 01kk kkkk ksss
	{&INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"         },//1111 111r rrrr 0bbb
	{&INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"         },//1111 110r rrrr 0bbb
	{&INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"        },//1101 kkkk kkkk kkkk
	{&INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"          },//1111 101d dddd 0bbb
	{&INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"          },//1111 100d dddd 0bbb
	{&INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"          },//0001 11rd dddd rrrr
	{&INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"          },//0011 KKKK dddd KKKK
	{&INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"           },//0001 01rd dddd rrrr
	{&INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"         },//0001 00rd dddd rrrr
	{&INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"          },//0001 10rd dddd rrrr
	{&INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD_Y"        },//10q0 qq0d dddd 1qqq
	{&INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD_Z"        },//10q0 qq0d dddd 0qqq
	{&INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD_Y"         },//1000 000d dddd 1000
	{&INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD_Z"         },//1000 000d dddd 0000
	{&INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD_Y"        },//10q0 qq1r rrrr 1qqq
	{&INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD_Z"        },//10q0 qq1r rrrr 0qqq
	{&INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST_Y"         },//1000 001r rrrr 1000
	{&INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST_Z"         },//1000 001r rrrr 0000
	{&INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"         },//0101 KKKK dddd KKKK
	{&INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"         },//0111 KKKK dddd KKKK
	{&INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"         },//0100 KKKK dddd KKKK
	{&INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"          },//0110 KKKK dddd KKKK
};


const A32u4::InstHandler::Inst_ELEM A32u4::InstHandler::instList[] = {
//   func                 mask                res
//				            1   2   3   4       1   2   3   4
	{&INST_STS          , 0b1111111000001111, 0b1001001000000000, "STS"          ,&getRd5_c},                   //1001 001d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_LDS          , 0b1111111000001111, 0b1001000000000000, "LDS"          ,&getRd5_c},                   //1001 000d dddd 0000 kkkk kkkk kkkk kkkk
	{&INST_POP          , 0b1111111000001111, 0b1001000000001111, "POP"          ,&getRd5_c},                   //1001 000d dddd 1111
	{&INST_PUSH         , 0b1111111000001111, 0b1001001000001111, "PUSH"         ,&getRd5_c},                   //1001 001d dddd 1111
	{&INST_CALL         , 0b1111111000001110, 0b1001010000001110, "CALL"         ,},                            //1001 010k kkkk 111k kkkk kkkk kkkk kkkk
	{&INST_LD_X         , 0b1111111000001111, 0b1001000000001100, "LD_X"         ,&getRd5_c},                   //1001 000d dddd 1100
	{&INST_LD_XpostInc  , 0b1111111000001111, 0b1001000000001101, "LD_XpostInc"  ,&getRd5_c},                   //1001 000d dddd 1101
	{&INST_LD_XpreDec   , 0b1111111000001111, 0b1001000000001110, "LD_XpreDec"   ,&getRd5_c},                   //1001 000d dddd 1110
	{&INST_LD_YpostInc  , 0b1111111000001111, 0b1001000000001001, "LD_YpostInc"  ,&getRd5_c},                   //1001 000d dddd 1001
	{&INST_LD_YpreDec   , 0b1111111000001111, 0b1001000000001010, "LD_YpreDec"   ,&getRd5_c},                   //1001 000d dddd 1010
	{&INST_LD_ZpostInc  , 0b1111111000001111, 0b1001000000000001, "LD_ZpostInc"  ,&getRd5_c},                   //1001 000d dddd 0001
	{&INST_LD_ZpreDec   , 0b1111111000001111, 0b1001000000000010, "LD_ZpreDec"   ,&getRd5_c},                   //1001 000d dddd 0010
	{&INST_JMP          , 0b1111111000001110, 0b1001010000001100, "JMP"          ,},                            //1001 010k kkkk 110k kkkk kkkk kkkk kkkk
	{&INST_RET          , 0b1111111111111111, 0b1001010100001000, "RET"          ,},                            //1001 0101 0000 1000
	{&INST_ADIW         , 0b1111111100000000, 0b1001011000000000, "ADIW"         ,&getRd2_c_arr,&getK6_d24},    //1001 0110 KKdd KKKK
	{&INST_OUT          , 0b1111100000000000, 0b1011100000000000, "OUT"          ,&getA6_d24,&getRd5_c},        //1011 1AAr rrrr AAAA
	{&INST_IN           , 0b1111100000000000, 0b1011000000000000, "IN"           ,&getA6_d24,&getRd5_c},        //1011 0AAd dddd AAAA
	{&INST_ROR          , 0b1111111000001111, 0b1001010000000111, "ROR"          ,&getRd5_c},                   //1001 010d dddd 0111
	{&INST_DEC          , 0b1111111000001111, 0b1001010000001010, "DEC"          ,&getRd5_c},                   //1001 010d dddd 1010
	{&INST_MUL          , 0b1111110000000000, 0b1001110000000000, "MUL"          ,&getRd5_c,&getRr5_c},         //1001 11rd dddd rrrr
	{&INST_LPM_0        , 0b1111111111111111, 0b1001010111001000, "LPM_0"        ,},                            //1001 0101 1100 1000
	{&INST_LPM_d        , 0b1111111000001111, 0b1001000000000100, "LPM_d"        ,&getRd5_c},                   //1001 000d dddd 0100
	{&INST_LPM_dpostInc , 0b1111111000001111, 0b1001000000000101, "LPM_dpostInc" ,&getRd5_c},                   //1001 000d dddd 0101
	{&INST_SBIW         , 0b1111111100000000, 0b1001011100000000, "SBIW"         ,&getRd2_c_arr,&getK6_d24},    //1001 0111 KKdd KKKK
	{&INST_LSR          , 0b1111111000001111, 0b1001010000000110, "LSR"          ,&getRd5_c},                   //1001 010d dddd 0110
	{&INST_ST_X         , 0b1111111000001111, 0b1001001000001100, "ST_X"         ,&getRd5_c},                   //1001 001r rrrr 1100
	{&INST_ST_XpostInc  , 0b1111111000001111, 0b1001001000001101, "ST_XpostInc"  ,&getRd5_c},                   //1001 001r rrrr 1101
	{&INST_ST_XpreDec   , 0b1111111000001111, 0b1001001000001110, "ST_XpreDec"   ,&getRd5_c},                   //1001 001r rrrr 1110
	{&INST_ST_YpostInc  , 0b1111111000001111, 0b1001001000001001, "ST_YpostInc"  ,&getRd5_c},                   //1001 001r rrrr 1001
	{&INST_ST_YpreDec   , 0b1111111000001111, 0b1001001000001010, "ST_YpreDec"   ,&getRd5_c},                   //1001 001r rrrr 1010
	{&INST_ST_ZpostInc  , 0b1111111000001111, 0b1001001000000001, "ST_ZpostInc"  ,&getRd5_c},                   //1001 001r rrrr 0001
	{&INST_ST_ZpreDec   , 0b1111111000001111, 0b1001001000000010, "ST_ZpreDec"   ,&getRd5_c},                   //1001 001r rrrr 0010
	{&INST_SBI          , 0b1111111100000000, 0b1001101000000000, "SBI"          ,&getA5_c,&getb3_c},           //1001 1010 AAAA Abbb
	{&INST_CBI          , 0b1111111100000000, 0b1001100000000000, "CBI"          ,&getA5_c,&getb3_c},           //1001 1000 AAAA Abbb
	{&INST_COM          , 0b1111111000001111, 0b1001010000000000, "COM"          ,&getRd5_c},                   //1001 010d dddd 0000
	{&INST_CLI          , 0b1111111111111111, 0b1001010011111000, "CLI"          ,},                            //1001 0100 1111 1000
	{&INST_INC          , 0b1111111000001111, 0b1001010000000011, "INC"          ,&getRd5_c},                   //1001 010d dddd 0011
	{&INST_NEG          , 0b1111111000001111, 0b1001010000000001, "NEG"          ,&getRd5_c},                   //1001 010d dddd 0001
	{&INST_ASR          , 0b1111111000001111, 0b1001010000000101, "ASR"          ,&getRd5_c},                   //1001 010d dddd 0101
	{&INST_ICALL        , 0b1111111111111111, 0b1001010100001001, "ICALL"        ,},                            //1001 0101 0000 1001
	{&INST_SBIS         , 0b1111111100000000, 0b1001101100000000, "SBIS"         ,&getA5_c,&getb3_c},           //1001 1011 AAAA Abbb
	{&INST_RETI         , 0b1111111111111111, 0b1001010100011000, "RETI"         ,},                            //1001 0101 0001 1000
	{&INST_WDR          , 0b1111111111111111, 0b1001010110101000, "WDR"          ,},                            //1001 0101 1010 1000
	{&INST_SLEEP        , 0b1111111111111111, 0b1001010110001000, "SLEEP"        ,},                            //1001 0101 1000 1000
	{&INST_SBIC         , 0b1111111100000000, 0b1001100100000000, "SBIC"         ,&getA5_c,&getb3_c},           //1001 1001 AAAA Abbb
	{&INST_CLT          , 0b1111111111111111, 0b1001010011101000, "CLT"          ,},                            //1001 0100 1110 1000
	{&INST_SWAP         , 0b1111111000001111, 0b1001010000000010, "SWAP"         ,&getRd5_c},                   //1001 010d dddd 0010
	{&INST_SEI          , 0b1111111111111111, 0b1001010001111000, "SEI"          ,},                            //1001 0100 0111 1000
	{&INST_SET          , 0b1111111111111111, 0b1001010001101000, "SET"          ,},                            //1001 0100 0110 1000
	{&INST_SEC          , 0b1111111111111111, 0b1001010000001000, "SEC"          ,},                            //1001 0100 0000 1000
	{&INST_IJMP         , 0b1111111111111111, 0b1001010000001001, "IJMP"         ,},                            //1001 0100 0000 1001
	{&INST_EIJMP        , 0b1111111111111111, 0b1001010000011001, "EIJMP"        ,},                            //1001 0100 0001 1001
	{&INST_EICALL       , 0b1111111111111111, 0b1001010100011001, "EICALL"       ,},                            //1001 0101 0001 1001
	{&INST_BSET         , 0b1111111110001111, 0b1001010000001000, "BSET"         ,&gets3_c},                    //1001 0100 0sss 1000
	{&INST_BCLR         , 0b1111111110001111, 0b1001010010001000, "BCLR"         ,&gets3_c},                    //1001 0100 1sss 1000
	{&INST_CLC          , 0b1111111111111111, 0b1001010010001000, "CLC"          ,},                            //1001 0100 1000 1000
	{&INST_SEN          , 0b1111111111111111, 0b1001010000101000, "SEN"          ,},                            //1001 0100 0010 1000
	{&INST_CLN          , 0b1111111111111111, 0b1001010010101000, "CLN"          ,},                            //1001 0100 1010 1000
	{&INST_SEZ          , 0b1111111111111111, 0b1001010000011000, "SEZ"          ,},                            //1001 0100 0001 1000
	{&INST_CLZ          , 0b1111111111111111, 0b1001010010011000, "CLZ"          ,},                            //1001 0100 1001 1000
	{&INST_SES          , 0b1111111111111111, 0b1001010001001000, "SES"          ,},                            //1001 0100 0100 1000
	{&INST_CLS          , 0b1111111111111111, 0b1001010011001000, "CLS"          ,},                            //1001 0100 1100 1000
	{&INST_SEV          , 0b1111111111111111, 0b1001010000111000, "SEV"          ,},                            //1001 0100 0011 1000
	{&INST_CLV          , 0b1111111111111111, 0b1001010010111000, "CLV"          ,},                            //1001 0100 1011 1000
	{&INST_SEH          , 0b1111111111111111, 0b1001010001011000, "SEH"          ,},                            //1001 0100 0101 1000
	{&INST_CLH          , 0b1111111111111111, 0b1001010011011000, "CLH"          ,},                            //1001 0100 1101 1000
	{&INST_ELPM_0       , 0b1111111111111111, 0b1001010111011000, "ELPM_0"       ,},                            //1001 0101 1101 1000
	{&INST_ELPM_d       , 0b1111111000001111, 0b1001000000000110, "ELPM_d"       ,&getRd5_c},                   //1001 000d dddd 0110
	{&INST_ELPM_dpostInc, 0b1111111000001111, 0b1001000000000111, "ELPM_dpostInc",&getRd5_c},                   //1001 000d dddd 0111
	{&INST_SPM          , 0b1111111111111111, 0b1001010111101000, "SPM"          ,},                            //1001 0101 1110 1000
	{&INST_BREAK        , 0b1111111111111111, 0b1001010110011000, "BREAK"        ,},                            //1001 0101 1001 1000
	{&INST_LDI          , 0b1111000000000000, 0b1110000000000000, "LDI"          ,&getRd4_c_a16,&getK8_d44},    //1110 KKKK dddd KKKK
	{&INST_RJMP         , 0b1111000000000000, 0b1100000000000000, "RJMP"         ,&getk12_c_sin},               //1100 kkkk kkkk kkkk
	{ &INST_MOVW         , 0b1111111100000000, 0b0000000100000000, "MOVW"         ,&getRd4_c_m2,&getRr4_c_m2 },   //0000 0001 dddd rrrr
	{ &INST_MOV          , 0b1111110000000000, 0b0010110000000000, "MOV"          ,&getRd5_c,&getRr5_c },         //0010 11rd dddd rrrr
	{ &INST_ADD          , 0b1111110000000000, 0b0000110000000000, "ADD"          ,&getRd5_c,&getRr5_c },         //0000 11rd dddd rrrr
	{ &INST_CPC          , 0b1111110000000000, 0b0000010000000000, "CPC"          ,&getRd5_c,&getRr5_c },         //0000 01rd dddd rrrr
	{ &INST_AND          , 0b1111110000000000, 0b0010000000000000, "AND"          ,&getRd5_c,&getRr5_c },         //0010 00rd dddd rrrr
	{ &INST_EOR          , 0b1111110000000000, 0b0010010000000000, "EOR"          ,&getRd5_c,&getRr5_c },         //0010 01rd dddd rrrr
	{ &INST_SBC          , 0b1111110000000000, 0b0000100000000000, "SBC"          ,&getRd5_c,&getRr5_c },         //0000 10rd dddd rrrr
	{ &INST_OR           , 0b1111110000000000, 0b0010100000000000, "OR"           ,&getRd5_c,&getRr5_c },         //0010 10rd dddd rrrr
	{ &INST_NOP          , 0b1111111111111111, 0b0000000000000000, "NOP"          , },                            //0000 0000 0000 0000
	{ &INST_MULSU        , 0b1111111110001000, 0b0000001100000000, "MULSU"        ,&getRd3_c_a16,&getRr3_c_a16 }, //0000 0011 0ddd 0rrr
	{ &INST_MULS         , 0b1111111100000000, 0b0000001000000000, "MULS"         ,&getRd4_c_a16,&getRr4_c_a16 }, //0000 0010 dddd rrrr
	{ &INST_FMUL         , 0b1111111110001000, 0b0000001100001000, "FMUL"         ,&getRd3_c_a16,&getRr3_c_a16 }, //0000 0011 0ddd 1rrr
	{ &INST_FMULS        , 0b1111111110001000, 0b0000001110000000, "FMULS"        ,&getRd3_c_a16,&getRr3_c_a16 }, //0000 0011 1ddd 0rrr
	{ &INST_FMULSU       , 0b1111111110001000, 0b0000001110001000, "FMULSU"       ,&getRd3_c_a16,&getRr3_c_a16 }, //0000 0011 1ddd 1rrr
	{ &INST_BRBS         , 0b1111110000000000, 0b1111000000000000, "BRBS"         ,&getb3_c,&getk7_c_sin },       //1111 00kk kkkk ksss
	{ &INST_BRBC         , 0b1111110000000000, 0b1111010000000000, "BRBC"         ,&getb3_c,&getk7_c_sin },       //1111 01kk kkkk ksss
	{ &INST_SBRS         , 0b1111111000001000, 0b1111111000000000, "SBRS"         ,&getRd5_c,&getb3_c },          //1111 111r rrrr 0bbb
	{ &INST_SBRC         , 0b1111111000001000, 0b1111110000000000, "SBRC"         ,&getRd5_c,&getb3_c },          //1111 110r rrrr 0bbb
	{ &INST_RCALL        , 0b1111000000000000, 0b1101000000000000, "RCALL"        ,&getk12_c_sin },               //1101 kkkk kkkk kkkk
	{ &INST_BST          , 0b1111111000001000, 0b1111101000000000, "BST"          ,&getRd5_c,&getb3_c },          //1111 101d dddd 0bbb
	{ &INST_BLD          , 0b1111111000001000, 0b1111100000000000, "BLD"          ,&getRd5_c,&getb3_c },          //1111 100d dddd 0bbb
	{ &INST_ADC          , 0b1111110000000000, 0b0001110000000000, "ADC"          ,&getRd5_c,&getRr5_c },         //0001 11rd dddd rrrr
	{ &INST_CPI          , 0b1111000000000000, 0b0011000000000000, "CPI"          ,&getRd4_c_a16,&getK8_d44 },    //0011 KKKK dddd KKKK
	{ &INST_CP           , 0b1111110000000000, 0b0001010000000000, "CP"           ,&getRd5_c,&getRr5_c },         //0001 01rd dddd rrrr
	{ &INST_CPSE         , 0b1111110000000000, 0b0001000000000000, "CPSE"         ,&getRd5_c,&getRr5_c },         //0001 00rd dddd rrrr
	{ &INST_SUB          , 0b1111110000000000, 0b0001100000000000, "SUB"          ,&getRd5_c,&getRr5_c },         //0001 10rd dddd rrrr
	{ &INST_LDD_Y        , 0b1101001000001000, 0b1000000000001000, "LDD_Y"        ,&getRd5_c,&getq6_d123 },       //10q0 qq0d dddd 1qqq
	{ &INST_LDD_Z        , 0b1101001000001000, 0b1000000000000000, "LDD_Z"        ,&getRd5_c,&getq6_d123 },       //10q0 qq0d dddd 0qqq
	{ &INST_LD_Y         , 0b1111111000001111, 0b1000000000001000, "LD_Y"         ,&getRd5_c },                   //1000 000d dddd 1000
	{ &INST_LD_Z         , 0b1111111000001111, 0b1000000000000000, "LD_Z"         ,&getRd5_c },                   //1000 000d dddd 0000
	{ &INST_STD_Y        , 0b1101001000001000, 0b1000001000001000, "STD_Y"        ,&getRd5_c,&getq6_d123 },       //10q0 qq1r rrrr 1qqq
	{ &INST_STD_Z        , 0b1101001000001000, 0b1000001000000000, "STD_Z"        ,&getRd5_c,&getq6_d123 },       //10q0 qq1r rrrr 0qqq
	{ &INST_ST_Y         , 0b1111111000001111, 0b1000001000001000, "ST_Y"         ,&getRd5_c },                   //1000 001r rrrr 1000
	{ &INST_ST_Z         , 0b1111111000001111, 0b1000001000000000, "ST_Z"         ,&getRd5_c },                   //1000 001r rrrr 0000
	{ &INST_SUBI         , 0b1111000000000000, 0b0101000000000000, "SUBI"         ,&getRd4_c_a16,&getK8_d44 },    //0101 KKKK dddd KKKK
	{ &INST_ANDI         , 0b1111000000000000, 0b0111000000000000, "ANDI"         ,&getRd4_c_a16,&getK8_d44 },    //0111 KKKK dddd KKKK
	{ &INST_SBCI         , 0b1111000000000000, 0b0100000000000000, "SBCI"         ,&getRd4_c_a16,&getK8_d44 },    //0100 KKKK dddd KKKK
	{ &INST_ORI          , 0b1111000000000000, 0b0110000000000000, "ORI"          ,&getRd4_c_a16,&getK8_d44 },    //0110 KKKK dddd KKKK
};


uint8_t A32u4::InstHandler::getInstInd1(uint16_t word) {
	for (uint8_t i = 0; i < instListLen; i++) {
		if ((word & instList[i].mask) == instList[i].res) {
			return i;
		}
	}

	println("Unhaldled Inst: 0x" << std::hex << word); // << " At: 0x" << mcu->cpu.PC

	return 0xff;
}
uint8_t A32u4::InstHandler::getInstInd2(uint16_t word) {
	uint8_t startInd = startIndArr[(word & 0b1100000000000000) >> 14];

	for (uint8_t i = startInd; i < instListLen; i++) {
		if ((word & instList[i].mask) == instList[i].res) {
			return i;
		}
	}
	println("Unhaldled Inst: 0x" << std::hex << word); // << " At: 0x" << mcu->cpu.PC
	return 0xff;
}

*/