#if !defined(__A32U4_DEBUGGER_H__) && MCU_INCLUDE_EXTRAS
#define __A32U4_DEBUGGER_H__

#include <string>
#include <set>
#include <vector>
#include <array>
#include <iostream>

#include "../A32u4Types.h"
#include "../config.h"

#include "../components/Flash.h" // for Flash::sizeMax
#include "../components/DataSpace.h" // for DataSpace::Consts::*

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

			bool operator==(const CallData& other) const;
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
#if !MCU_USE_HEAP
		Breakpoint breakpoints[breakPointArrMaxSize];
		CallData callStack[addressStackMaxSize];
		uint8_t addressStackIndicators[addressStackIndicatorsSize];
#else
		Breakpoint* breakpoints;
		CallData* callStack;
		uint8_t* addressStackIndicators;
#endif
#else
#if !MCU_USE_HEAP
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

		void updateBreakpointListFromArr();
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
		void clearAllBreakpoints();
		const std::set<uint16_t>& getBreakpointList() const;

		const Breakpoint* getBreakpoints() const;
		const CallData* getCallStack() const;
		uint16_t getCallStackPointer() const;
		uint16_t getStackPCAt(uint16_t stackInd) const;
		uint16_t getStackFromPCAt(uint16_t stackInd) const;
		const uint8_t* getAddressStackIndicators() const;

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Debugger& other) const;
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	};
}
namespace DataUtils {
	inline size_t approxSizeOf(const A32u4::Debugger& v) {
		return v.sizeBytes();
	}
}

#endif