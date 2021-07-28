#ifndef _A32u4_ANALYTICS
#define _A32u4_ANALYTICS

#include "config.h"
#include "InstHandler.h"
#include "Flash.h"

namespace A32u4 {
	class Analytics {
	private:
		friend class ATmega32u4;
		friend class InstHandler;
#if !USE_HEAP
		uint64_t pcCounter[A32u4::Flash::size / 2];
		uint64_t instCounter[InstHandler::instListLen];
#else
		uint64_t* pcCounter;
		uint64_t* instCounter;
#endif

		Analytics();
		~Analytics();

		void reset();
		void addData(uint8_t instInd,uint16_t PC);

	public:
		uint64_t getPCCnt(uint16_t addr);
		uint16_t findMostUsedPCCnt();
	};
}

#endif