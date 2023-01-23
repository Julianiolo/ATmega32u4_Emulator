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

#define MCU_LOG(level, msg) ATmega32u4::log_(level,msg,__FILE__,__LINE__);
#define MCU_LOG_M(level, msg, module_) ATmega32u4::log_(level,msg,__FILE__,__LINE__,module_);

#if RANGE_CHECK
#if RANGE_CHECK_ERROR
#define A32U4_ASSERT_INRANGE(val,from,to,msg,action) if((val) < (from) || (val) >= (to)) { mcu->log(ATmega32u4::LogLevel_Error, (msg), __FILE__, __LINE__); action;}
#define A32U4_ASSERT_INRANGE_M(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ mcu->log(ATmega32u4::LogLevel_Error, (msg), __FILE__, __LINE__,module_); action;}
#define A32U4_ASSERT_INRANGE_M_W(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ mcu->log(ATmega32u4::LogLevel_Warning, (msg), __FILE__, __LINE__,module_); action;}
#else
#define A32U4_ASSERT_INRANGE(val,from,to,msg,action) if((val) < (from) || (val) >= (to)) { action;}
#define A32U4_ASSERT_INRANGE_M(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ action;}
#define A32U4_ASSERT_INRANGE_M_W(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ action;}
#endif
#else
#define A32U4_ASSERT_INRANGE(val,from,to,msg,action)
#define A32U4_ASSERT_INRANGE_M(val,from,to,msg,module_,action)
#define A32U4_ASSERT_INRANGE_M_W(val,from,to,msg,module_, action)
#endif

#define A32U4_ADDR_ERR_STR(A_msg,A_addr,A_HexPlaces) A_msg  + std::to_string(A_addr) + " => 0x" + StringUtils::uIntToHexStr(A_addr,A_HexPlaces)

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
		enum {
			LogFlags_None                   =        0,
			LogFlags_ShowFileNameAndLineNum = (1 << 0),
			LogFlags_ShowModule             = (1 << 1),
			LogFlags_ShowAll = LogFlags_ShowFileNameAndLineNum | LogFlags_ShowModule
		};

		typedef void (*LogCallB)(LogLevel logLevel, const char* msg, const char* fileName , size_t lineNum, const char* Module);
		typedef void (*LogCallBSimple)(LogLevel logLevel, const char* msg);
	private:
		LogCallB logCallB = nullptr;
		LogCallBSimple logCallBSimple = nullptr;
		static ATmega32u4* currLogTarget;
		uint8_t currentExecFlags = -1;

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

		static int logFlags;
		void log(LogLevel logLevel, const char* msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		void log(LogLevel logLevel, const std::string& msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		template<typename ... Args>
		void logf(LogLevel logLevel, const char* msg, Args ... args) {
			log(logLevel, StringUtils::format(msg, args ...));
		}

		void activateLog();
		static void log_(LogLevel logLevel, const char* msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		static void log_(LogLevel logLevel, const std::string& msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		template<typename ... Args>
		static void logf_(LogLevel logLevel, const char* msg, Args ... args) {
			log_(logLevel, StringUtils::format(msg, args ...));
		}

		void setLogCallB(LogCallB newLogCallB);
		void setLogCallBSimple(LogCallBSimple newLogCallBSimple);

		void getState(std::ostream& output);
		void setState(std::istream& input);

	private:
		void setMcu();
	};
}


#endif