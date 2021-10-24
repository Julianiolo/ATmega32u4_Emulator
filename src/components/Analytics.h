#ifndef _A32u4_ANALYTICS
#define _A32u4_ANALYTICS

#include "config.h"
#include "InstHandler.h"
#include "Flash.h"

namespace A32u4 {
	class Analytics {
	public:
		static constexpr size_t PCHeatArrSize = A32u4::Flash::sizeMax / 2;
		static constexpr size_t InstHeatArrSize = InstHandler::instListLen;
	private:
		friend class ATmega32u4;
		friend class InstHandler;
#if !USE_HEAP
		uint64_t pcCounter[PCHeatArrSize];
		uint64_t instCounter[InstHeatArrSize];
#else
		uint64_t* pcCounter;
		uint64_t* instCounter;
#endif

		

		Analytics();
		~Analytics();

		void reset();
		void addData(uint8_t instInd,uint16_t PC);

	public:
		std::string str;
		uint16_t maxSP = 0xFFFF;
		uint64_t sleepSum = 0;

		uint64_t getPCCnt(uint16_t addr);
		uint16_t findMostUsedPCCnt();

		const uint64_t* getPCHeat() const;
		const uint64_t* getInstHeat() const;
	};
}

#endif