#ifndef _A32u4_INSTHANDLER
#define _A32u4_INSTHANDLER

#include <string>

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
		template<bool debug,bool analyse>
		void handleInstT(uint8_t& CYCL_ADD_Ref, int16_t& PC_ADD_Ref);
		void executeInstSwitch(uint16_t word);
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
			const char* name;

			const uint8_t parTypes;

			uint8_t(*par1)(uint16_t word) = NULL;
			uint8_t(*par2)(uint16_t word) = NULL;
		};

		static constexpr uint8_t instListLen = 111;
		static const Inst_ELEM instList[instListLen];
	};
}

#endif