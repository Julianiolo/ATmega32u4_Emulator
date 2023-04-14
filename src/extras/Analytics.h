#if !defined(__A32U4_ANALYTICS_H__) && MCU_INCLUDE_EXTRAS
#define __A32U4_ANALYTICS_H__

#include <array>
#include <vector>
#include <iostream>

#include "../A32u4Types.h"
#include "../config.h"

#include "../components/InstHandler.h"
#include "../components/Flash.h"
#include "../components/DataSpace.h"

namespace A32u4 {
	class Analytics {
	public:
		static constexpr sizemcu_t PCHeatArrSize = A32u4::Flash::sizeMax / 2;
		static constexpr uint8_t InstHeatArrSize = InstHandler::instListLen;
	private:
		friend class ATmega32u4;
		friend class InstHandler;
		friend DataSpace;
#if !MCU_USE_HEAP
		std::array<uint64_t,PCHeatArrSize> pcCounter;
		std::array<uint64_t,InstHeatArrSize> instCounter;

		std::array<uint64_t, DataSpace::Consts::data_size> ramReadBuf;
		std::array<uint64_t, DataSpace::Consts::data_size> ramWriteBuf;
#else
		std::vector<uint64_t> pcCounter;
		std::vector<uint64_t> instCounter;

		std::vector<uint64_t> ramReadBuf;
		std::vector<uint64_t> ramWriteBuf;
#endif

		uint64_t instTotalCnt = 0;

		Analytics();

		void reset();
		void addData(uint8_t instInd,uint16_t PC);

		void ramRead(addrmcu_t addr);
		void ramWrite(addrmcu_t addr);

		void eepromRead(addrmcu_t addr);
		void eepromWrite(addrmcu_t addr);

		void romRead(addrmcu_t addr);
		void romWrite(addrmcu_t addr);
	public:
		addrmcu_t maxSP = ADDRMCU_T_MAX;
		uint64_t sleepSum = 0;

		uint16_t findMostUsedPCCnt();

		uint64_t getPCCnt(uint16_t addr) const;
		const uint64_t* getPCCntRaw() const;
		void resetPCCnt();

		const uint64_t* getInstHeat() const;
		uint64_t getTotalInstCnt() const;

		const uint64_t* getRamRead() const;
		const uint64_t* getRamWrite() const;
		void clearRamRead();
		void clearRamWrite();

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Analytics& other) const;
		size_t sizeBytes() const;
	};
}

#endif