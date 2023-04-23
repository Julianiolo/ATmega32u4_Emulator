#ifndef __A32u4_ATMEGA32U4_H__
#define __A32u4_ATMEGA32U4_H__

#include <iostream>

#include "config.h"

#include "StringUtils.h"

#include "components/CPU.h"
#include "components/DataSpace.h"
#include "components/Flash.h"

#if MCU_INCLUDE_EXTRAS
#include "extras/Debugger.h"
#include "extras/Analytics.h"
#endif


#include "LogUtils.h"

#define LU_CONTEXT {A32u4::ATmega32u4::_log,mcu}

#if MCU_RANGE_CHECK
#if MCU_RANGE_CHECK_ERROR
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...) if((val) < (from) || (val) >= (to)) { LU_LOGF_(LogUtils::LogLevel_Error, msg, __VA_ARGS__); action;}
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg) if((val) < (from) || (val) >= (to)) { LU_LOGF_(LogUtils::LogLevel_Error, msg, val, val); action;}
#else
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...) if((val) < (from) || (val) >= (to)) { action;}
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg) if((val) < (from) || (val) >= (to)) { action;}
#endif
#else
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...)
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg)
#endif

#define MCU_ADDR_FORMAT "%" MCU_PRIuADDR "(0x%" MCU_PRIxADDR ")"

#if MCU_CHECK_HASH
#define A32U4_CHECK_HASH(_module_) uint32_t hash_; StreamUtils::read(input, &hash_); if (hash_ != hash()) { LU_LOG_(LogUtils::LogLevel_Warning, _module_ " read state hash does not match"); }
#else
#define A32U4_CHECK_HASH(_module_) 
#endif

namespace A32u4 {
	class ATmega32u4 {
	private:
		LogUtils::LogCallB logCallB = defaultLogHandler;
		void* logCallBUserData = nullptr;

		bool running = false;
	public:
		ATmega32u4();
		ATmega32u4(const ATmega32u4& src);
		ATmega32u4& operator=(const ATmega32u4& src);

		A32u4::CPU cpu;
		A32u4::DataSpace dataspace;
		A32u4::Flash flash;

#if MCU_INCLUDE_EXTRAS
		A32u4::Debugger debugger;
		A32u4::Analytics analytics;
#endif

		void reset();

		void resetHardware();

		void powerOn();

		void execute(uint64_t cyclAmt, bool debug);

		bool loadFile(const char* path);

		void activateLog();

		static void _log(uint8_t logLevel, const char* msg, const char* fileName, int lineNum, const char* module, void* userData);

		void setLogCallB(LogUtils::LogCallB newLogCallB, void* userData);
		static void defaultLogHandler(uint8_t logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData);

		void getState(std::ostream& output);
		void setState(std::istream& input);
		
		bool operator==(const ATmega32u4& other) const;
		size_t sizeBytes() const;
		uint32_t hash() const noexcept;
	private:
		void setMcu();
	};
}

namespace DataUtils {
	inline size_t approxSizeOf(const A32u4::ATmega32u4& v) {
		return v.sizeBytes();
	}
}

template<>
struct std::hash<A32u4::ATmega32u4>{
	inline std::size_t operator()(const A32u4::ATmega32u4& v) const noexcept{
		return (size_t)v.hash();
	}
};

#endif