//#pragma once

#ifndef _A32u4_DATASPACE
#define _A32u4_DATASPACE

#include <stdint.h>

#include "config.h"

#include "CPU.h"

namespace A32u4 {
	class ATmega32u4;

	class DataSpace {
	public:
		typedef void (*SPIByteCallB)(uint8_t data);
		struct Consts {
#include "DataspaceConstants.h"
		};
	private:
		friend class ATmega32u4;
		friend class Updates;
		friend class CPU;
		friend class Debugger;
		friend class InstHandler;

		class Updates {
		public:
			ATmega32u4* const mcu;

			uint64_t lastEECR_EEMPE_set = 0;
			uint8_t& REF_EECR;

			static constexpr uint32_t PLLCSR_PLOCK_wait = (CPU::ClockFreq / 1000) * 1; //1ms
			uint64_t lastPLLCSR_PLLE_set = 0;
			uint8_t& REF_PLLCSR;

			uint8_t& REF_PORTB;
			uint8_t& REF_SPDR;
			void (*SCK_Callback)() = NULL;
			SPIByteCallB SPI_Byte_Callback = NULL;

			Updates(ATmega32u4* mcu);

			void update_Get(uint16_t Addr, bool onlyOne);

			uint8_t update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal);
			uint8_t setEECR(uint8_t val, uint8_t oldVal);
			void setPLLCSR(uint8_t val, uint8_t oldVal);
			void setSPDR();
			void setTCCR0B(uint8_t val);
		};

		class Timers {
		public:
			ATmega32u4* const mcu;

			uint8_t& REF_TCCR0B;
			uint8_t& REF_TIFR0;
			uint8_t timer0_presc_cache = 0;
			uint32_t lastCounter;

			static constexpr uint16_t presc[] = {0,1,8,64,256,1024};

			Timers(ATmega32u4* mcu);

			void update();
			void doTick(uint8_t& timer);
			void doTicks(uint8_t& timer,uint8_t num);
			void checkForIntr();
		};

		ATmega32u4* mcu;
		
#if !USE_HEAP
		uint8_t data[Consts::data_size];
#else
		uint8_t* data;
#endif

		
#if !USE_HEAP
		uint8_t eeprom[Consts::eeprom_size];
#else
		uint8_t* eeprom;
#endif

		Updates funcs;
		Timers timers;

		uint8_t errorIndicator = -1;

		DataSpace(ATmega32u4* mcu);
		~DataSpace();

		uint8_t& getByteRefAtAddr(uint16_t addr);

		uint8_t& getGPRegRef(uint8_t ind);
		uint8_t getByteAt(uint16_t Addr);
		uint8_t setByteAt(uint16_t Addr, uint8_t val);

		uint8_t getIOAt(uint8_t ind);
		uint8_t setIOAt(uint8_t ind, uint8_t val);

		uint8_t getRegBit(uint16_t id, uint8_t bit);
		void setRegBit(uint16_t id, uint8_t bit, bool val);

		uint16_t getWordReg(uint8_t id);
		void setWordReg(uint8_t id, uint16_t val);
		uint16_t getWordRegRam(uint16_t id);
		void setWordRegRam(uint16_t id, uint16_t val);
		uint16_t getX();
		uint16_t getY();
		uint16_t getZ();
		void setX(uint16_t word);
		void setY(uint16_t word);
		void setZ(uint16_t word);

		void resetIO();

		void setSP(uint16_t val);

		void pushByteToStack(uint8_t val);
		uint8_t popByteFromStack();
		void pushAddrToStack(uint16_t Addr);
		uint16_t popAddrFromStack();

	public:
		void setSPIByteCallB(SPIByteCallB func);

		uint8_t* getEEPROM();
		const uint8_t* getData();
		const uint8_t getDataByte(uint16_t Addr);
		void setDataByte(uint16_t Addr, uint8_t byte);
		void setBitTo(uint16_t Addr, uint8_t bit, bool val);
		void setBitsTo(uint16_t Addr, uint8_t mask, uint8_t bits);

		uint16_t getSP();
	};
}

#endif