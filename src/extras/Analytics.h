#if !defined(__A32U4_ANALYTICS_H__) && MCU_INCLUDE_EXTRAS
#define __A32U4_ANALYTICS_H__

#include <array>
#include <vector>
#include <iostream>

#include "../A32u4Types.h"
#include "../config.h"

#include "../components/InstHandler.h"
#include "../components/Flash.h"

namespace A32u4 {
	class Analytics {
	public:
		static constexpr sizemcu_t PCHeatArrSize = A32u4::Flash::sizeMax / 2;
		static constexpr uint8_t InstHeatArrSize = InstHandler::instListLen;
	private:
		friend class ATmega32u4;
		friend class InstHandler;
#if !MCU_USE_HEAP
		std::array<uint64_t,PCHeatArrSize> pcCounter;
		std::array<uint64_t,InstHeatArrSize> instCounter;
#else
		std::vector<uint64_t> pcCounter;
		std::vector<uint64_t> instCounter;
#endif

		uint64_t instTotalCnt = 0;

		Analytics();

		void reset();
		void addData(uint8_t instInd,uint16_t PC);

	public:
		addrmcu_t maxSP = ADDRMCU_T_MAX;
		uint64_t sleepSum = 0;

		uint16_t findMostUsedPCCnt();

		uint64_t getPCCnt(uint16_t addr) const;
		const uint64_t* getPCCntRaw() const;
		void resetPCCnt();

		const uint64_t* getInstHeat() const;
		uint64_t getTotalInstCnt() const;

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Analytics& other) const;
		size_t sizeBytes() const;
	};
}

#endif