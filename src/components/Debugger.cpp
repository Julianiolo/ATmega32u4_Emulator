#include "Debugger.h"
#include "../ATmega32u4.h"
#include "Disassembler.h"
#include "../utils/StringUtils.h"

#include <iostream>
#include <stdint.h>

A32u4::Debugger::Debugger(ATmega32u4* mcu):
	mcu(mcu)
#if USE_HEAP
	, breakpoints(new uint8_t[breakPointArrMaxSize]),
	addressStack(new uint16_t[addressStackMaxSize]),
	fromAddressStack(new uint16_t[addressStackMaxSize]),
	addressStackIndicators(new uint8_t[DataSpace::Consts::ISRAM_size])
#endif
{
	resetBreakpoints();
}

A32u4::Debugger::~Debugger() {
#if USE_HEAP
	delete[] breakpoints;
	delete[] addressStack;
	delete[] fromAddressStack;
	delete[] addressStackIndicators;
#endif
}

uint8_t A32u4::Debugger::debugOutputMode = OutputMode_Log;

void A32u4::Debugger::reset() {
	halted = false;
	doStep = false;
	skipHalting = false;

	addressStackPointer = 0;
	for (int i = 0; i < DataSpace::Consts::ISRAM_size; i++) {
		addressStackIndicators[i] = false;
	}
}
void A32u4::Debugger::resetBreakpoints(){
	for (int i = 0; i < breakPointArrMaxSize; i++) {
		breakpoints[i] = 0;
	}
}

void A32u4::Debugger::pushAddrOnAddressStack(uint16_t addr, uint16_t fromAddr) {
	//return;
	A32U4_ASSERT_INRANGE_M(addressStackPointer+1, 0, addressStackMaxSize, A32U4_ADDR_ERR_STR("Debug Address Stack overflow: ",addressStackPointer,4), "Debugger", return);
	addressStack[addressStackPointer] = addr;
	fromAddressStack[addressStackPointer++] = fromAddr;
}
void A32u4::Debugger::popAddrFromAddressStack() {
	//return;
	A32U4_ASSERT_INRANGE_M(addressStackPointer, 1, addressStackMaxSize, A32U4_ADDR_ERR_STR("Debug Address Stack underflow: ",addressStackPointer,4), "Debugger", return);

	addressStackPointer--;
	addressStack[addressStackPointer] = 0;
	fromAddressStack[addressStackPointer] = 0;
}

void A32u4::Debugger::registerAddressBytes(uint16_t addr) {
	addr -= DataSpace::Consts::ISRAM_start;
	addressStackIndicators[addr] = 1;
	addressStackIndicators[addr - 1] = 2;
}

void A32u4::Debugger::clearAddressByte(uint16_t addr) {
	//static bool doPopAddress = false;

	addr -= DataSpace::Consts::ISRAM_start;

	if (addressStackIndicators[addr] == 1) {
		//if (doPopAddress) {
		popAddrFromAddressStack();
		//}
		//doPopAddress = !doPopAddress;
	}

	addressStackIndicators[addr] = 0;
}
void A32u4::Debugger::clearAddressByteRaw(uint16_t addr) {
	addr -= DataSpace::Consts::ISRAM_start;

	addressStackIndicators[addr] = 0;
}

bool A32u4::Debugger::checkBreakpoints() {
	if (breakpoints[mcu->cpu.PC] || halted) {
		return doHaltActions();
	}
	return false;
}
bool A32u4::Debugger::doHaltActions() {
	if (debugOutputMode == OutputMode_Log)
		doHaltActionsLog();
	else {
		bool ret = true;
		if(doStep)
			ret = false;

		doStep = false;
		if(!skipHalting)
			halted = true;
		skipHalting = false;
		mcu->cpu.breakOutOfOptim = true;

		return ret;
	}
	return false;
}
void A32u4::Debugger::doHaltActionsLog(){
	uint16_t word = mcu->flash.getInst(mcu->cpu.PC);
	uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);

	if (!halted) {
		mcu->log(ATmega32u4::LogLevel_Output, "Halted at : " + Disassembler::disassemble(word, word2, mcu->cpu.PC)); // + stringExtras::intToHex(PC * 2,4)
	}
	halted = true;
	printDisassembly = true;

	while (true) {
		std::string input;
		std::cout << ">";
		std::getline(std::cin, input);
		if (input == "c" || input == "continue") {
			halted = false;
			printDisassembly = false;
			break;
		}
		else if (input == "s" || input == "step") {
			break;
		}
		else if (input == "sk" || input == "stack") {
			mcu->log(ATmega32u4::LogLevel_Output, debugStackToString());
		}
		else if (input == "R" || input == "RAll") {
			mcu->log(ATmega32u4::LogLevel_Output, AllRegsToStr());
		}
		else if (input.substr(0, 1) == "R") {
			int ind = std::stoi(input.substr(1, input.length()));;
			if (ind < 32) {
				mcu->log(ATmega32u4::LogLevel_Output, regToStr(ind));
			}
			else {
				mcu->log(ATmega32u4::LogLevel_Output, input + " is out of Range of the Register (R0-R31)");
			}
		}
		else if (input.substr(0, 2) == "io") {
			int addr = StringUtils::numStrToUInt(input.c_str() + 3, input.c_str() + input.size());

			if (addr < DataSpace::Consts::io_size + DataSpace::Consts::ext_io_size) {
				uint8_t ioVal = mcu->dataspace.getIOAt(addr);
				mcu->logf(ATmega32u4::LogLevel_Output, "%s: 0x%02h > %d", input.c_str(), ioVal, ioVal); // (input + ": 0x" + stringExtras::intToHex(ioVal, 2) + " > " + std::to_string(ioVal)).c_str());
			}
			else {
				mcu->log(ATmega32u4::LogLevel_Output, input + " is out of Range of the IO space (0-" + std::to_string(DataSpace::Consts::io_size + DataSpace::Consts::ext_io_size) + ")");
			}
		}
		else if (input == "e" || input == "exit") {
			//TODO exit out of program
			// maybe this should also be removed
		}
		else if (input == "h" || input == "help") {
			mcu->log(ATmega32u4::LogLevel_Output, getHelp());
		}
		else {
			mcu->log(ATmega32u4::LogLevel_Output, "Invalid command, try help");
		}
	}
}

std::string A32u4::Debugger::regToStr(uint8_t ind) const {
	uint8_t regVal = mcu->dataspace.data[ind];
	return StringUtils::format("R%2d: 0x%02x > %3d", ind, regVal, regVal).get();
}
std::string A32u4::Debugger::AllRegsToStr() const{
	std::string str = "";
	for (int i = 0; i < 32; i++) {
		str += regToStr(i);
		if (i < 32-1) {
			str += "\n";
		}
	}
	return str;
}

std::string A32u4::Debugger::debugStackToString() const{
	std::string str = "";
	for (int i = addressStackPointer - 1; i >= 0; i--) {
		str += StringUtils::format("%04x : at %04x", addressStack[i] * 2, fromAddressStack[i] * 2).get();
		if (i > 0) {
			str += "\n";
		}
	}
	return str;
}

std::string A32u4::Debugger::getHelp() const {
	std::string str = "";
	str += "s, step      :  step forward\n";
	str += "c, continue  :  continue execution\n";
	str += "\n";
	str += "R, RAll      :  print all registers\n";
	str += "R[0-31]      :  print specific register\n";
	str += "io[0-223]    :  print io register\n";
	str += "sk, stack    :  print debug call stack\n";
	str += "\n";
	str += "e, exit      :  exit execution\n";
	str += "h, help      :  print help";
	
	return str;
}

bool A32u4::Debugger::execShouldReturn() {
	return isHalted() && !doStep;
}

bool A32u4::Debugger::isHalted() const {
	return halted;
}
void A32u4::Debugger::halt() {
	halted = true;
	if (debugOutputMode == OutputMode_Passthrough) {
		doStep = false;
		mcu->cpu.breakOutOfOptim = true;
	}
}
void A32u4::Debugger::step() {
	doStep = true;
}
void A32u4::Debugger::continue_() {
	halted = false;
	doStep = true;
	skipHalting = true;
}
void A32u4::Debugger::setBreakpoint(uint16_t addr) {
	breakpoints[addr/2] = 1;
}
void A32u4::Debugger::clearBreakpoint(uint16_t addr) {
	breakpoints[addr/2] = 0;
}

A32u4::Debugger::Breakpoint* A32u4::Debugger::getBreakpoints() const {
	return breakpoints;
}
const A32u4::Debugger::Breakpoint* A32u4::Debugger::getBreakpointsRead() const {
	return breakpoints;
}
const uint16_t* A32u4::Debugger::getAddressStack() const {
	return addressStack;
}
const uint16_t* A32u4::Debugger::getFromAddressStack() const {
	return fromAddressStack;
}
const uint16_t A32u4::Debugger::getAddressStackPointer() const {
	return addressStackPointer;
}
const uint16_t A32u4::Debugger::getAddresAt(uint16_t stackInd) const {
	return addressStack[stackInd];
}
const uint16_t A32u4::Debugger::getFromAddresAt(uint16_t stackInd) const {
	return fromAddressStack[stackInd];
}

/*
std::string regNumStr = stringExtras::paddRight(std::to_string(ind), 2, ' ');
std::string decValStr = stringExtras::paddLeft(std::to_string(regVal), 3, ' ');
return "R" + regNumStr + ": 0x" + stringExtras::intToHex(regVal, 2) + " > " + decValStr;
*/