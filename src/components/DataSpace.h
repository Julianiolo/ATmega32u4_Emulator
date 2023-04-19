#ifndef __A32U4_DATASPACE_H__
#define __A32U4_DATASPACE_H__

#include <stdint.h>
#include <iostream> // istream & ostream
#include <cstring> // for NULL

#include "../A32u4Types.h"
#include "../config.h"

#include "CPU.h" // for CPU::ClockFreq

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

		ATmega32u4* mcu;
		
#if !MCU_USE_HEAP
		uint8_t data[Consts::data_size];
#else
		uint8_t* data;
#endif

		
#if !MCU_USE_HEAP
		uint8_t eeprom[Consts::eeprom_size];
#else
		uint8_t* eeprom;
#endif

		void (*SCK_Callback)() = NULL;
		SPIByteCallB SPI_Byte_Callback = NULL;


		uint8_t sreg[8] = {0,0,0,0,0,0,0,0};

		struct LastSet {
			uint64_t EECR_EEMPE = 0;
			uint64_t PLLCSR_PLLE = 0;
			uint64_t ADCSRA_ADSC = 0;
			uint64_t Timer0Update = 0;

			void resetAll();
			bool operator==(const LastSet& other) const;
			size_t sizeBytes() const;
			size_t hash() const noexcept;
		} lastSet;

		static constexpr uint32_t PLLCSR_PLOCK_wait = (CPU::ClockFreq / 1000) * 1; //1ms
		static constexpr uint64_t ADC_wait = 0;

		DataSpace(ATmega32u4* mcu);
		~DataSpace();

		DataSpace(const DataSpace& src);
		DataSpace& operator=(const DataSpace& src);

		void reset();
		void resetIO();
		
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


		void update_Get(uint16_t Addr, bool onlyOne);

		void update_Set(uint16_t Addr, uint8_t val, uint8_t oldVal);
		void setEECR(uint8_t val, uint8_t oldVal);
		void setPLLCSR(uint8_t val, uint8_t oldVal);
		void setSPDR();
		void setTCCR0B(uint8_t val);

		void updateSREGCache();

		void updateCache();


		uint8_t getGPReg_(regind_t ind) const;
		void setGPReg_(regind_t ind, reg_t val);

		uint16_t getADCVal();


		// Timer stuff
		static constexpr uint16_t timerPresc[] = {1,1,8,64,256,1024};
		void doTick();
		void doTicks(uint8_t num);
		void checkForIntr();
		uint8_t getTimer0Presc() const;
		uint16_t getTimer0PrescDiv() const;
		void markTimer0Update();


		void setFlags_NZ(uint8_t res);
		void setFlags_NZ(uint16_t res);
		void setFlags_HSVNZC_ADD(uint8_t a, uint8_t b, uint8_t c, uint8_t res);
		void setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t c, uint8_t res, bool Incl_Z);

		void setFlags_SVNZ(uint8_t res);
		void setFlags_SVNZC(uint8_t res);

		void setFlags_SVNZC_ADD_16(uint16_t a, uint16_t b, uint16_t res);
		void setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res);
	public:
		void setSPIByteCallB(SPIByteCallB func);

		uint8_t& getGPRegRef(regind_t ind);
		uint8_t getGPReg(regind_t ind) const;
		void setGPReg(regind_t ind, reg_t val);

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
		void loadDataFromMemory(const uint8_t* data, size_t len);

		addrmcu_t getSP() const;

		void getRamState(std::ostream& output);
		void setRamState(std::istream& input);
		void getEepromState(std::ostream& output);
		void setEepromState(std::istream& input);

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const DataSpace& other) const;
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	};
}
namespace DataUtils {
	inline size_t approxSizeOf(const A32u4::DataSpace& v) {
		return v.sizeBytes();
	}
}
template<>
struct std::hash<A32u4::DataSpace>{
    inline std::size_t operator()(const A32u4::DataSpace& v) const noexcept{
        return v.hash();
    }
};

#endif