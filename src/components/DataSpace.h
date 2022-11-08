//#pragma once

#ifndef _A32u4_DATASPACE
#define _A32u4_DATASPACE

#include <stdint.h>
#include <string.h> //for NULL
#include "../A32u4Types.h"

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

		class Timers {
		public:
			ATmega32u4* const mcu;

			uint8_t timer0_presc_cache = 0;
			uint64_t lastTimer0Update = 0;
			uint32_t lastCounter;

			static constexpr uint16_t presc[] = {0,1,8,64,256,1024};

			Timers(ATmega32u4* mcu);


			void reset();

			void update();
			void doTick(uint8_t& timer);
			void doTicks(uint8_t num);
			void checkForIntr();
			uint8_t getTimer0Presc() const;
			uint16_t getTimer0PrescDiv() const;
			void markTimer0Update();
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
		Timers timers;

		uint8_t errorIndicator = -1;

		uint8_t sreg[8] = {0,0,0,0,0,0,0,0};

		DataSpace(ATmega32u4* mcu);
		~DataSpace();

		void reset();
		void resetIO();

		uint8_t& getByteRefAtAddr(uint16_t addr);

		
		uint8_t getByteAt(uint16_t Addr);
		void setByteAt(uint16_t Addr, uint8_t val);

		uint8_t getIOAt(uint8_t ind);
		void setIOAt(uint8_t ind, uint8_t val);

		uint8_t getRegBit(uint16_t id, uint8_t bit);
		void setRegBit(uint16_t id, uint8_t bit, bool val);

		
		uint16_t getWordRegRam(uint16_t id) const;
		void setWordRegRam(uint16_t id, uint16_t val);

		uint32_t getExtendedZ();

		void setSP(uint16_t val);

		void pushByteToStack(uint8_t val);
		uint8_t popByteFromStack();
		void pushAddrToStack(addrmcu_t Addr);
		addrmcu_t popAddrFromStack();

		uint64_t lastEECR_EEMPE_set = 0;

		static constexpr uint32_t PLLCSR_PLOCK_wait = (CPU::ClockFreq / 1000) * 1; //1ms
		uint64_t lastPLLCSR_PLLE_set = 0;
		void (*SCK_Callback)() = NULL;
		SPIByteCallB SPI_Byte_Callback = NULL;

		void update_Get(uint16_t Addr, bool onlyOne);

		void update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal);
		void setEECR(uint8_t val, uint8_t oldVal);
		void setPLLCSR(uint8_t val, uint8_t oldVal);
		void setSPDR();
		void setTCCR0B(uint8_t val);


		uint8_t getGPReg_(uint8_t ind) const;
		void setGPReg_(uint8_t ind, reg_t val);
	public:
		void setSPIByteCallB(SPIByteCallB func);

		uint8_t& getGPRegRef(uint8_t ind);
		uint8_t getGPReg(uint8_t ind) const;
		void setGPReg(uint8_t ind, reg_t val);

		uint16_t getX() const;
		uint16_t getY() const;
		uint16_t getZ() const;
		void setX(uint16_t word);
		void setY(uint16_t word);
		void setZ(uint16_t word);

		uint16_t getWordReg(uint8_t id) const;
		void setWordReg(uint8_t id, uint16_t val);
		uint8_t* getEEPROM();
		const uint8_t* getData();
		uint8_t getDataByte(addrmcu_t Addr);
		void setDataByte(addrmcu_t Addr, uint8_t byte);
		void setBitTo(addrmcu_t Addr, uint8_t bit, bool val);
		void setBitsTo(addrmcu_t Addr, uint8_t mask, uint8_t bits);

		addrmcu_t getSP() const;
	};
}

#endif