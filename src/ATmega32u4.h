#ifndef __A32u4_ATMEGA32U4_H__
#define __A32u4_ATMEGA32U4_H__

#include <iostream>
#include <functional>

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
#define A32U4_ASSERT_INRANGE(val,from,to,action,msg,...) if((val) < (from) || (val) >= (to)) { LU_LOGF_(LogUtils::LogLevel_Error, msg, __VA_ARGS__); mcu->cpu.executeError(); action;}
#define A32U4_ASSERT_INRANGE2(val,from,to,action,msg) if((val) < (from) || (val) >= (to)) { LU_LOGF_(LogUtils::LogLevel_Error, msg, val, val); mcu->cpu.executeError(); action;}
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
		friend DataSpace;
		LogUtils::LogCallB logCallB = defaultLogHandler;
		void* logCallBUserData = nullptr;

		bool running = false;
		std::function<void(uint8_t pinReg, reg_t oldVal, reg_t val)> pinChangeCallB = nullptr;
	public:
		struct InterruptInfo {
			addrmcu_t addr;
			const char* source;
			const char* definition;
		};
		static constexpr InterruptInfo interruptInfo[43] = {
			{ 0x0000, "RESET", "External Pin, Power-on Reset, Brown-out Reset, Watchdog Reset, and JTAG AVR Reset" },
			{ 0x0002, "INT0", "External Interrupt Request 0" },
			{ 0x0004, "INT1", "External Interrupt Request 1" },
			{ 0x0006, "INT2", "External Interrupt Request 2" },
			{ 0x0008, "INT3", "External Interrupt Request 3" },
			{ 0x000A, "Reserved", "Reserved" },
			{ 0x000C, "Reserved", "Reserved" },
			{ 0x000E, "INT6", "ExternalInterrupt Request 6" },
			{ 0x0010, "Reserved", "Reserved" },
			{ 0x0012, "PCINT0", "Pin Change Interrupt Request 0" },
			{ 0x0014, "USB General", "USB General Interrupt request" },
			{ 0x0016, "USB Endpoint", "USB Endpoint Interrupt request" },
			{ 0x0018, "WDT", "Watchdog Time-out Interrupt" },
			{ 0x001A, "Reserved", "Reserved" },
			{ 0x001C, "Reserved", "Reserved" },
			{ 0x001E, "Reserved", "Reserved" },
			{ 0x0020, "TIMER1 CAPT", "Timer/Counter1 Capture Event" },
			{ 0x0022, "TIMER1 COMPA", "Timer/Counter1 Compare Match A" },
			{ 0x0024, "TIMER1 COMPB", "Timer/Counter1 Compare Match B" },
			{ 0x0026, "TIMER1 COMPC", "Timer/Counter1 Compare Match C" },
			{ 0x0028, "TIMER1 OVF", "Timer/Counter1 Overflow" },
			{ 0x002A, "TIMER0 COMPA", "Timer/Counter0 Compare Match A" },
			{ 0x002C, "TIMER0 COMPB", "Timer/Counter0 Compare match B" },
			{ 0x002E, "TIMER0 OVF", "Timer/Counter0 Overflow" },
			{ 0x0030, "SPI (STC)", "SPI Serial Transfer Complete" },
			{ 0x0032, "USART1 RX", "USART1 Rx Complete" },
			{ 0x0034, "USART1 UDRE", "USART1 Data Register Empty" },
			{ 0x0036, "USART1TX", "USART1Tx Complete" },
			{ 0x0038, "ANALOG COMP", "Analog Comparator" },
			{ 0x003A, "ADC", "ADC Conversion Complete" },
			{ 0x003C, "EE READY", "EEPROM Ready" },
			{ 0x003E, "TIMER3 CAPT", "Timer/Counter3 Capture Event" },
			{ 0x0040, "TIMER3 COMPA", "Timer/Counter3 Compare Match A" },
			{ 0x0042, "TIMER3 COMPB", "Timer/Counter3 Compare Match B" },
			{ 0x0044, "TIMER3 COMPC", "Timer/Counter3 Compare Match C" },
			{ 0x0046, "TIMER3 OVF", "Timer/Counter3 Overflow" },
			{ 0x0048, "TWI", "2-wire Serial Interface" },
			{ 0x004A, "SPM READY", "Store Program Memory Ready" },
			{ 0x004C, "TIMER4 COMPA", "Timer/Counter4 Compare Match A" },
			{ 0x004E, "TIMER4 COMPB", "Timer/Counter4 Compare Match B" },
			{ 0x0050, "TIMER4 COMPD", "Timer/Counter4 Compare Match D" },
			{ 0x0052, "TIMER4 OVF", "Timer/Counter4 Overflow" },
			{ 0x0054, "TIMER4 FPF", "Timer/Counter4 Fault Protection Interrupt" },
		};

		enum {
			PinChange_PORTB = 0,
			PinChange_PORTC,
			PinChange_PORTD,
			PinChange_PORTE,
			PinChange_PORTF
		};

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

		void setPinChangeCallB(const std::function<void(uint8_t pinReg, reg_t oldVal, reg_t val)>& callB);

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