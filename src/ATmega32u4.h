#ifndef _ATMEGA32U4
#define _ATMEGA32U4

#include <iostream>

#include "config.h"

#include "StringUtils.h"

#include "components/CPU.h"
#include "components/DataSpace.h"
#include "components/Flash.h"
#include "extras/Debugger.h"
#include "extras/Analytics.h"
#include "extras/SymbolTable.h"

#define MCU_MCUPTR_PREFIX mcu->

#define MCU_LOG(level, msg) MCU_MCUPTR_PREFIX log(level,msg,__FILE__,__LINE__,MCU_MODULE);
#define MCU_LOGF(level, msg, ...) MCU_MCUPTR_PREFIX logf2(level,MCU_MODULE,__FILE__,__LINE__,msg,##__VA_ARGS__);
#define MCU_LOG_(level, msg) A32u4::ATmega32u4::log_(level,msg,__FILE__,__LINE__,MCU_MODULE);
#define MCU_LOGF_(level, msg, ...) A32u4::ATmega32u4::logf2_(level,MCU_MODULE,__FILE__,__LINE__,msg,##__VA_ARGS__);


#if RANGE_CHECK
#if RANGE_CHECK_ERROR
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...) if((val) < (from) || (val) >= (to)) { MCU_LOGF(ATmega32u4::LogLevel_Error, msg, ##__VA_ARGS__); action;}
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg) if((val) < (from) || (val) >= (to)) { MCU_LOGF(ATmega32u4::LogLevel_Error, msg, val, val); action;}
#else
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...) if((val) < (from) || (val) >= (to)) { action;}
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg) if((val) < (from) || (val) >= (to)) { action;}
#endif
#else
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...)
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg)
#endif

#define MCU_ADDR_FORMAT "%" MCU_PRIuADDR "(0x%" MCU_PRIxADDR ")"

//#define A32U4_ADDR_ERR_STR(A_msg,A_addr) "%s%u => 0x%4x @PC:%", MCU_PRIuPC, A_msg, A_addr, A_addr, mcu->cpu.PC

namespace A32u4 {
	class ATmega32u4 {
	public:
		enum {
			ExecFlags_None =       0,
			ExecFlags_Debug =   1<<0,
			ExecFlags_Analyse = 1<<1
		};
		enum {
			LogLevel_None,
			LogLevel_DebugOutput,
			LogLevel_Output,
			LogLevel_Warning,
			LogLevel_Error,
			LogLevel_COUNT
		};
		typedef int LogLevel;
		static constexpr const char* logLevelStrs[] = {"None","DebugOutput","Output","Warning","Error"};

		typedef void (*LogCallB)(LogLevel logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData);
	private:
		LogCallB logCallB = defaultLogHandler;
		void* logCallBUserData = nullptr;
		static ATmega32u4* currLogTarget;

		bool wasReset = false;
	public:
		ATmega32u4();
		ATmega32u4(const ATmega32u4& src);
		ATmega32u4& operator=(const ATmega32u4& src);

		A32u4::CPU cpu;
		A32u4::DataSpace dataspace;
		A32u4::Flash flash;

		A32u4::Debugger debugger;
		A32u4::Analytics analytics;
		A32u4::SymbolTable symbolTable;

		void reset();

		void resetHardware();

		void powerOn();

		void execute(uint64_t cyclAmt, uint8_t flags);

		bool load(const uint8_t* data, size_t dataLen);
		bool loadFile(const char* path);

		bool loadFromHex(const uint8_t* data, size_t dataLen);
		bool loadFromHexFile(const char* path);
		bool loadFromELF(const uint8_t* data, size_t dataLen);
		bool loadFromELFFile(const char* path);

		void log(LogLevel logLevel, const char* msg, const char* fileName = NULL, int lineNum = -1, const char* module = NULL);
		void log(LogLevel logLevel, const std::string& msg, const char* fileName = NULL, int lineNum = -1, const char* module = NULL);
		template<typename ... Args>
		void logf(LogLevel logLevel, const char* msg, Args ... args) {
			log(logLevel, StringUtils::format(msg, args ...));
		}
		template<typename ... Args>
		void logf2(LogLevel logLevel, const char* module, const char* fileName, int lineNum, const char* msg, Args ... args) {
			log(logLevel, StringUtils::format(msg, args ...), fileName, lineNum, module);
		}

		void activateLog();
		static void log_(LogLevel logLevel, const char* msg, const char* fileName = NULL, int lineNum = -1, const char* module = NULL);
		static void log_(LogLevel logLevel, const std::string& msg, const char* fileName = NULL, int lineNum = -1, const char* module = NULL);
		template<typename ... Args>
		static void logf_(LogLevel logLevel, const char* msg, Args ... args) {
			log_(logLevel, StringUtils::format(msg, args ...));
		}
		template<typename ... Args>
		static void logf2_(LogLevel logLevel, const char* module, const char* fileName, int lineNum, const char* msg, Args ... args) {
			log_(logLevel, StringUtils::format(msg, args ...), fileName, lineNum, module);
		}

		void setLogCallB(LogCallB newLogCallB, void* userData);
		static void defaultLogHandler(LogLevel logLevel, const char* msg, const char* fileName , int lineNum, const char* module, void* userData);

		void getState(std::ostream& output);
		void setState(std::istream& input);
		
		bool operator==(const ATmega32u4& other) const;
	private:
		void setMcu();
	};
}


#endif