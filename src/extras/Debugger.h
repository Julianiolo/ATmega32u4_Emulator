#ifndef _A32u4_DEBUGGER
#define _A32u4_DEBUGGER

#include <string>
#include <set>

#include "../A32u4Types.h"

#include "../config.h"
#include "../components/Flash.h"
#include "../components/DataSpace.h"

namespace A32u4 {
	class ATmega32u4;

	class Debugger {
	public:
		typedef uint8_t Breakpoint;

		static constexpr sizemcu_t addressStackMaxSize = 512;
		static constexpr sizemcu_t breakPointArrMaxSize = Flash::sizeMax / 2;
		static constexpr sizemcu_t addressStackIndicatorsSize = DataSpace::Consts::ISRAM_size;

		struct CallData{
			pc_t to;
			pc_t from;
		};
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
#define DEBUGGER_STD 1
#if !DEBUGGER_STD
#if !USE_HEAP
		Breakpoint breakpoints[breakPointArrMaxSize];
		CallData callStack[addressStackMaxSize];
		uint8_t addressStackIndicators[addressStackIndicatorsSize];
#else
		Breakpoint* breakpoints;
		CallData* callStack;
		uint8_t* addressStackIndicators;
#endif
#else
#if !USE_HEAP
		std::array<Breakpoint,breakPointArrMaxSize> breakpoints;
		std::array<CallData,addressStackMaxSize> callStack;
		std::array<uint8_t,addressStackIndicatorsSize> addressStackIndicators;
#else
		std::vector<Breakpoint> breakpoints;
		std::vector<CallData> callStack;
		std::vector<uint8_t> addressStackIndicators;
#endif
#endif

		// last recived SP (relative to ISRAM_start)
		addrmcu_t lastSPRecived = DataSpace::Consts::SP_initaddr - DataSpace::Consts::ISRAM_start;

		Debugger(ATmega32u4* mcu);
		~Debugger();
		Debugger(const Debugger& src);
		Debugger& operator=(const Debugger& src);

		void reset();
		void resetBreakpoints();

		void pushPCOnCallStack(pc_t pc, pc_t fromPC);
		void popPCFromCallStack();

		void registerAddressBytes(addrmcu_t addr);
		void registerStackDec(addrmcu_t addr);

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

		static bool printDisassembly;

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
		const CallData* getCallStack() const;
		uint16_t getCallStackPointer() const;
		uint16_t getPCAt(uint16_t stackInd) const;
		uint16_t getFromPCAt(uint16_t stackInd) const;
		const uint8_t* getAddressStackIndicators() const;
	};
}
#endif