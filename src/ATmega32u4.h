#ifndef _ATMEGA32U4
#define _ATMEGA32U4

#include "components/CPU.h"
#include "components/DataSpace.h"
#include "components/Flash.h"
#include "components/Debugger.h"
#include "components/Analytics.h"

#include "components/config.h"

#include "utils/StringUtils.h"

#if RANGE_CHECK
#define A32U4_ASSERT_INRANGE(val,from,to,msg,action) if((val) < (from) || (val) >= (to)) { mcu->log(ATmega32u4::LogLevel_Error, (msg), __FILE__, __LINE__); action;}
#define A32U4_ASSERT_INRANGE_M(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ mcu->log(ATmega32u4::LogLevel_Error, (msg), __FILE__, __LINE__,module_); action;}
#define A32U4_ASSERT_INRANGE_M_W(val,from,to,msg,module_, action) if((val) < (from) || (val) >= (to)){ mcu->log(ATmega32u4::LogLevel_Warning, (msg), __FILE__, __LINE__,module_); action;}
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
		uint8_t currentExecFlags = -1;
	public:
		ATmega32u4();

		A32u4::CPU cpu;
		A32u4::DataSpace dataspace;
		A32u4::Flash flash;

		A32u4::Debugger debugger;
		A32u4::Analytics analytics;

		void reset();

		void resetHardware();

		void powerOn();

		void execute(uint64_t cyclAmt, uint8_t flags);

		static int logFlags;
		void log(LogLevel logLevel, const char* msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		void log(LogLevel logLevel, const std::string& msg, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		template<typename ... Args>
		void logf(LogLevel logLevel, const char* msg, Args ... args) {
			log(logLevel, StringUtils::format(msg, args ...).get());
		}

		void setLogCallB(LogCallB newLogCallB);
		void setLogCallBSimple(LogCallBSimple newLogCallBSimple);
	};
}


#endif