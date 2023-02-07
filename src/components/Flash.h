#ifndef _A32u4_FLASH
#define _A32u4_FLASH

#include <cstring>
#include <iostream>

#include "../config.h"
#include "../A32u4Types.h"


#define FLASH_USE_INSTIND_CACHE 1

namespace A32u4 {
	class ATmega32u4;

	class Flash {
	public:
		static constexpr sizemcu_t sizeMax = 32768;
	private:
		friend class Disassembler;
		friend class InstHandler;
		friend class ATmega32u4;
		friend class CPU;

		ATmega32u4* mcu;

#if !USE_HEAP
		uint8_t data[sizeMax];
#if FLASH_USE_INSTIND_CACHE
		uint8_t instCache[sizeMax/2];
#endif
#else
		uint8_t* data;
#if FLASH_USE_INSTIND_CACHE
		uint8_t* instCache;
#endif
#endif

		sizemcu_t size_ = sizeMax;
		bool hasProgram = false;

		Flash(ATmega32u4* mcu);
		~Flash();

		Flash(const Flash& src);
		Flash& operator=(const Flash& src);

		uint8_t getInstIndCache(pc_t pc) const;
		void populateInstIndCache();
		void populateInstIndCacheEntry(pc_t pc);
	public:
		uint8_t getByte(addrmcu_t addr) const;
		uint16_t getWord(addrmcu_t addr) const;
		uint16_t getInst(pc_t pc) const;
		uint8_t getInstInd(pc_t pc) const;

		const uint8_t* getData();

		void setByte(addrmcu_t addr, uint8_t val);
		void setInst(pc_t pc, uint16_t val);

		void loadFromMemory(const uint8_t* data, size_t dataLen);
		bool loadFromHexFile(const char* path);
		bool loadFromHexString(const char* str, const char* str_end = 0);
		void clear();

		sizemcu_t size() const;
		sizemcu_t sizeWords() const;

		bool isProgramLoaded() const; // returns true if a program was loaded

		void getRomState(std::ostream& output);
		void setRomState(std::istream& input);

		void getState(std::ostream& output);
		void setState(std::istream& input);

		bool operator==(const Flash& other) const;
	};
}
#endif