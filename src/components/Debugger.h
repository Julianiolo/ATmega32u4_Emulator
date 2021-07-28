#ifndef _A32u4_DEBUGGER
#define _A32u4_DEBUGGER

#include <string>

#include "config.h"
#include "Flash.h"
#include "DataSpace.h"

namespace A32u4 {
	class ATmega32u4;

	class Debugger {
	public:
		static constexpr uint16_t addressStackSize = 512;
		static constexpr uint16_t breakPointArrSize = Flash::size / 2;

	private:
		friend class ATmega32u4;
		friend class DataSpace;
		friend class InstHandler;
		friend class CPU;

		ATmega32u4* mcu;
		
		uint16_t addressStackPointer = 0;

		bool halted = false;
#if !USE_HEAP
		uint8_t breakpoints[breakPointArrSize];
		uint16_t addressStack[addressStackSize];
		uint16_t fromAddressStack[addressStackSize];
		uint8_t addressStackIndicators[DataSpace::Consts::ISRAM_size];
#else
		uint8_t* breakpoints;
		uint16_t* addressStack;
		uint16_t* fromAddressStack;
		uint8_t* addressStackIndicators;
#endif
		Debugger(ATmega32u4* mcu);
		~Debugger();

		void reset();

		void pushAddrOnAddressStack(uint16_t addr, uint16_t fromAddr);
		void popAddrFromAddressStack();

		void registerAddressBytes(uint16_t addr);
		void clearAddressByte(uint16_t addr);
		void clearAddressByteRaw(uint16_t addr);

		void checkBreakpoints();
		void haltedActions();
	public:
		

		bool printDisassembly = false;

		std::string regToStr(uint8_t ind) const;
		std::string AllRegsToStr() const;
		std::string debugStackToString() const;

		std::string getHelp() const;

		void halt();
		void setBreakpoint(uint16_t addr);
		void clearBreakpoint(uint16_t addr);

		const uint8_t* getBreakpoints() const;
		const uint16_t* getAddressStack() const;
		const uint16_t getAddressStackPointer() const;
	};
}
#endif