#ifndef _A32u4_FLASH
#define _A32u4_FLASH

#include "config.h"

#include <stdint.h>

#define FLASH_USE_INSTIND_CACHE 1

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
#if FLASH_USE_INSTIND_CACHE
		uint8_t instCache[size/2];
#endif
#else
		uint8_t* data;
#if FLASH_USE_INSTIND_CACHE
		uint8_t* instCache;
#endif
#endif

		size_t size_ = sizeMax;
		bool hasProgram = false;

		Flash(ATmega32u4* mcu);
		~Flash();

		uint8_t getInstIndCache(uint16_t addr) const;
		void populateInstIndCache();
	public:
		uint8_t getByte(uint16_t addr) const;
		uint16_t getWord(uint16_t addr) const;
		uint16_t getInst(uint16_t pc) const;
		uint8_t getInstInd(uint16_t pc) const;

		uint8_t* getData();

		bool loadFromHexFile(const char* str);
		void loadFromHexString(const char* str);
		void clear();

		size_t size() const;
		size_t sizeWords() const;
	};
}
#endif