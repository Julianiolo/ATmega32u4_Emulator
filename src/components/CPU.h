#ifndef _A32u4_CPU
#define _A32u4_CPU

#include <iostream> // istream & ostream

#include "../A32u4Types.h"

// sub components
#include "InstHandler.h"

namespace A32u4 {
	class ATmega32u4;

	class CPU {
	public:
		static constexpr uint64_t ClockFreq = 16000000;
	private:
		friend class ATmega32u4;
		friend class InstHandler;
		friend class DataSpace;
		friend class Debugger;
	private:
		ATmega32u4* mcu;

		pc_t PC;
		uint64_t totalCycls, targetCycs;

		uint64_t interruptFlags;
		bool insideInterrupt;

		bool breakOutOfOptim = false;

		bool CPU_sleep = false;
		uint64_t sleepCycsLeft = 0;

		CPU(ATmega32u4* mcu_);

		template<bool debug, bool analyse>
		void execute(uint64_t amt);
		template<bool debug, bool analyse>
		void execute4T(uint64_t amt);

		void queueInterrupt(uint16_t addr);
		void executeInterrupts();
		void directExecuteInterrupt(uint8_t num);

		uint64_t cycsToNextTimerInt();

		void setFlags_NZ(uint8_t res);
		void setFlags_NZ(uint16_t res);
		void setFlags_HSVNZC_ADD(uint8_t a, uint8_t b, uint8_t c, uint8_t res);
		void setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t c, uint8_t res, bool Incl_Z);

		void setFlags_SVNZ(uint8_t res);
		void setFlags_SVNZC(uint8_t res);

		void setFlags_SVNZC_ADD_16(uint16_t a, uint16_t b, uint16_t res);
		void setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res);

		void reset();
	public:
		pc_t& getPCRef();
		pc_t getPC() const;
		addrmcu_t getPCAddr() const;
		uint64_t getTotalCycles() const;
		bool isSleeping() const;

		void getState(std::ostream& output);
		void setState(std::istream& input);
	};
}

#endif