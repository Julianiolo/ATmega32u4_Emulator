#ifndef _ATMEGA32U4
#define _ATMEGA32U4

#include "components/CPU.h"
#include "components/DataSpace.h"
#include "components/Flash.h"
#include "components/Debugger.h"
#include "components/Analytics.h"

namespace A32u4 {
	class ATmega32u4 {
	private:
		void(*logCallB)(const char* str) = nullptr;

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

		void reset();

		void hardwareReset();

		void powerOn();

		void execute(uint64_t cyclAmt, uint8_t flags);

		void log(const char* msg);
		void log(const std::string& msg);

		void setLogCallB(void (*newCallB)(const char* str));
	};
}


#endif