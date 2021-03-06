#ifndef _A32u4_DEBUGGER
#define _A32u4_DEBUGGER

#include <string>
#include <set>

#include "../A32u4Types.h"

#include "config.h"
#include "Flash.h"
#include "DataSpace.h"

namespace A32u4 {
	class ATmega32u4;

	class Debugger {
	public:
		typedef uint8_t Breakpoint;

		static constexpr sizemcu_t addressStackMaxSize = 512;
		static constexpr sizemcu_t breakPointArrMaxSize = Flash::sizeMax / 2;
		static constexpr sizemcu_t addressStackIndicatorsSize = DataSpace::Consts::ISRAM_size;

	private:
		friend class ATmega32u4;
		friend class DataSpace;
		friend class InstHandler;
		friend class CPU;

		ATmega32u4* mcu;
		
		uint16_t callStackPtr = 0;

		bool halted = false;
		bool doStep = false;
		uint64_t skipCycs = -1;

		std::set<pc_t> breakpointList;
#if !USE_HEAP
		Breakpoint breakpoints[breakPointArrMaxSize];
		uint16_t callStack[addressStackMaxSize];
		uint16_t callStackFrom[addressStackMaxSize];
		uint8_t addressStackIndicators[DataSpace::Consts::ISRAM_size];
#else
		Breakpoint* breakpoints;
		pc_t* callStack;
		pc_t* callStackFrom;
		uint8_t* addressStackIndicators;
#endif

		// last recived SP (relative to ISRAM_start)
		at_addr_t lastSPRecived = DataSpace::Consts::SP_initaddr - DataSpace::Consts::ISRAM_start;

		Debugger(ATmega32u4* mcu);
		~Debugger();

		void reset();
		void resetBreakpoints();

		void pushPCOnCallStack(pc_t pc, pc_t fromPC);
		void popPCFromCallStack();

		void registerAddressBytes(at_addr_t addr);
		void registerStackDec(at_addr_t addr);

		bool checkBreakpoints();
		bool doHaltActions();
		void doHaltActionsLog();

		bool execShouldReturn();
	public:
		enum {
			OutputMode_Log = 0,
			OutputMode_Passthrough
		};
		static uint8_t debugOutputMode;

		bool printDisassembly = false;

		std::string regToStr(regind_t ind) const;
		std::string AllRegsToStr() const;
		std::string debugStackToString() const;

		std::string getHelp() const;

		bool isHalted() const;
		void halt();
		void step();
		void continue_();
		void setBreakpoint(pc_t pc);
		void clearBreakpoint(pc_t pc);
		const std::set<uint16_t>& getBreakpointList() const;

		const Breakpoint* getBreakpoints() const;
		const pc_t* getCallStack() const;
		const pc_t* getCallStackFrom() const;
		uint16_t getCallStackPointer() const;
		uint16_t getPCAt(uint16_t stackInd) const;
		uint16_t getFromPCAt(uint16_t stackInd) const;
		const uint8_t* getAddressStackIndicators() const;
	};
}
#endif