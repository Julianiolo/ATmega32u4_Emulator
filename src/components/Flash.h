#ifndef _A32u4_FLASH
#define _A32u4_FLASH

#include <cstring>
#include <iostream>

#include "../config.h"
#include "../A32u4Types.h"

namespace A32u4 {
	class ATmega32u4;

	class Flash {
	public:
		static constexpr sizemcu_t sizeMax = 32768;
	private:
		friend class ATmega32u4;  // for con/de-structor

		ATmega32u4* mcu;

#if !MCU_USE_HEAP
		uint8_t data[sizeMax];
#if MCU_USE_INSTCACHE
		uint8_t instCache[sizeMax/2];
#endif
#else
		uint8_t* data;
#if MCU_USE_INSTCACHE
		uint8_t* instCache;
#endif
#endif

		sizemcu_t size_ = sizeMax;
		bool hasProgram = false;

		Flash(ATmega32u4* mcu);
		~Flash();

		Flash(const Flash& src);
		Flash& operator=(const Flash& src);

#if MCU_USE_INSTCACHE
		uint8_t getInstIndCache(pc_t pc) const;
		void populateInstIndCache();
		void populateInstIndCacheEntry(pc_t pc);
#endif
	public:
		uint8_t getByte(addrmcu_t addr) const;
		uint16_t getWord(addrmcu_t addr) const;
		uint16_t getInst(pc_t pc) const;
		uint8_t getInstInd(pc_t pc) const;

		const uint8_t* getData();

		void setByte(addrmcu_t addr, uint8_t val);
		void setInst(pc_t pc, uint16_t val);

		bool loadFromMemory(const uint8_t* data, size_t dataLen);
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
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	};
}
namespace DataUtils {
	inline size_t approxSizeOf(const A32u4::Flash& v) {
		return v.sizeBytes();
	}
}

template<>
struct std::hash<A32u4::Flash>{
    inline std::size_t operator()(const A32u4::Flash& v) const noexcept{
        return (size_t)v.hash();
    }
};

#endif