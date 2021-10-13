#ifndef _A32u4_FLASH
#define _A32u4_FLASH

#include "config.h"

#include <stdint.h>

namespace A32u4 {
	class ATmega32u4;

	class Flash {
	public:
		static constexpr uint16_t sizeMax = 32768;
	private:
		friend class Disassembler;
		friend class InstHandler;
		friend class ATmega32u4;
		friend class CPU;

		ATmega32u4* mcu;

#if !USE_HEAP
		uint8_t data[size];
		uint8_t instCache[size/2];
#else
		uint8_t* data;
		uint8_t* instCache;
#endif

		size_t size_ = sizeMax;
		bool hasProgram = false;

		Flash(ATmega32u4* mcu);
		~Flash();

		uint8_t getInstIndCache(uint16_t addr);

	public:
		uint8_t getByte(uint16_t addr) const;
		uint16_t getWord(uint16_t addr) const;
		uint16_t getInst(uint16_t addr) const;

		uint8_t* getData();

		bool loadFromHexFile(const char* str);
		void loadFromHexString(const char* str);
		void clear();

		size_t size() const;
	};
}
#endif