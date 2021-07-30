#ifndef _ATMEGA32U4
#define _ATMEGA32U4

#include "components/CPU.h"
#include "components/DataSpace.h"
#include "components/Flash.h"
#include "components/Debugger.h"
#include "components/Analytics.h"

#include "components/config.h"

#if RANGE_CHECK
#define A32U4_ASSERT_INRANGE(A_val,A_from,A_to,A_msg) if((A_val) < (A_from) || (A_val) >= (A_to)) mcu->log((A_msg), ATmega32u4::LogLevel_Error, __FILE__, __LINE__)
#define A32U4_ASSERT_INRANGE_M(A_val,A_from,A_to,A_msg,A_module) if((A_val) < (A_from) || (A_val) >= (A_to)) mcu->log((A_msg), ATmega32u4::LogLevel_Error, __FILE__, __LINE__,A_module)
#else
#define A32U4_ASSERT_INRANGE(A_val,A_from,A_to,A_msg)
#define A32U4_ASSERT_INRANGE_M(A_val,A_from,A_to,A_msg,A_module)
#endif

#define A32U4_ADDR_ERR_STR(A_msg,A_addr,A_HexPlaces) A_msg  + std::to_string(A_addr) + " => 0x" + stringExtras::intToHex(A_addr,A_HexPlaces)

namespace A32u4 {
	class ATmega32u4 {
	private:
		void(*logCallB)(const char* str) = nullptr;
		uint8_t currentExecFlags = 0;
	public:
		ATmega32u4();

		A32u4::CPU cpu;
		A32u4::DataSpace dataspace;
		A32u4::Flash flash;

		A32u4::Debugger debugger;
		A32u4::Analytics analytics;

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
			LogLevel_Error
		};
		typedef int LogLevel;
		enum {
			LogFlags_None                   =        0,
			LogFlags_ShowFileNameAndLineNum = (1 << 0),
			LogFlags_ShowModule             = (1 << 1),
			LogFlags_ShowAll = LogFlags_ShowFileNameAndLineNum | LogFlags_ShowModule
		};

		void reset();

		void hardwareReset();

		void powerOn();

		void execute(uint64_t cyclAmt, uint8_t flags);

		static int logFlags;
		void log(const char* msg, LogLevel logLevel, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);
		void log(const std::string& msg, LogLevel logLevel, const char* fileName = NULL, size_t lineNum = -1, const char* Module = NULL);

		void setLogCallB(void (*newCallB)(const char* str));
	};
}


#endif