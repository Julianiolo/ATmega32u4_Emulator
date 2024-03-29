#ifndef __A32U4_CPU_H__
#define __A32U4_CPU_H__

#include <iostream> // istream & ostream

#include "../A32u4Types.h"

// sub components
#include "InstHandler.h"

namespace A32u4 {
	class ATmega32u4;

	class CPU {
	public:
		static constexpr uint64_t ClockFreq = 16000000;
	private:
		friend class ATmega32u4;
		friend class InstHandler;
		friend class DataSpace;
		friend class Debugger;
	private:
		ATmega32u4* mcu;

		pc_t PC;
		uint64_t totalCycls;
		uint64_t targetCycls;

		uint64_t interruptFlags;
		bool insideInterrupt;

		bool CPU_sleep = false;
		uint64_t sleepCycsLeft = 0;

		CPU(ATmega32u4* mcu_);

		template<bool debug>
		void execute(uint64_t amt);
		template<bool debug>
		void execute4T(uint64_t amt);

		void executeError();

		void queueInterrupt(uint16_t addr);
		void executeInterrupts();
		void directExecuteInterrupt(uint8_t num);

		void setFlags_NZ(uint8_t res);
		void setFlags_NZ(uint16_t res);
		void setFlags_HSVNZC_ADD(uint8_t a, uint8_t b, uint8_t c, uint8_t res);
		void setFlags_HSVNZC_SUB(uint8_t a, uint8_t b, uint8_t c, uint8_t res, bool Incl_Z);

		void setFlags_SVNZ(uint8_t res);
		void setFlags_SVNZC(uint8_t res);

		void setFlags_SVNZC_ADD_16(uint16_t a, uint16_t b, uint16_t res);
		void setFlags_SVNZC_SUB_16(uint16_t a, uint16_t b, uint16_t res);

		void reset();

		void breakOutOfOptimisation();
	public:
		pc_t& getPCRef();
		pc_t getPC() const;
		addrmcu_t getPCAddr() const;
		uint64_t getTotalCycles() const;
		bool isSleeping() const;

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const CPU& other) const;
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	};
}
namespace DataUtils {
	inline size_t approxSizeOf(const A32u4::CPU& v) {
		return v.sizeBytes();
	}
}

template<>
struct std::hash<A32u4::CPU>{
    inline std::size_t operator()(const A32u4::CPU& cpu) const noexcept{
        return (size_t)cpu.hash();
    }
};

#endif