#include "Debugger.h"
#include "../ATmega32u4.h"
#include "Disassembler.h"
#include "../utils/stringExtras.h"

#include <iostream>
#include <stdint.h>

A32u4::Debugger::Debugger(ATmega32u4* mcu):
	mcu(mcu)
#if USE_HEAP
	, breakpoints(new uint8_t[breakPointArrSize]),
	addressStack(new uint16_t[addressStackSize]),
	fromAddressStack(new uint16_t[addressStackSize]),
	addressStackIndicators(new uint8_t[DataSpace::Consts::ISRAM_size])
#endif
{

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
	addressStackPointer = 0;
	for (int i = 0; i < breakPointArrSize; i++) {
		breakpoints[i] = 0;
	}
	for (int i = 0; i < DataSpace::Consts::ISRAM_size; i++) {
		addressStackIndicators[i] = false;
	}
}

void A32u4::Debugger::pushAddrOnAddressStack(uint16_t addr, uint16_t fromAddr) {
	A32U4_ASSERT_INRANGE_M(addressStackPointer, 0, addressStackSize, A32U4_ADDR_ERR_STR("Debug Address Stack overflow: ",addressStackPointer,4), "Debugger");
	addressStack[addressStackPointer] = addr;
	fromAddressStack[addressStackPointer++] = fromAddr;
}
void A32u4::Debugger::popAddrFromAddressStack() {
	A32U4_ASSERT_INRANGE_M(addressStackPointer, 1, addressStackSize, A32U4_ADDR_ERR_STR("Debug Address Stack underflow: ",addressStackPointer,4), "Debugger");

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

void A32u4::Debugger::checkBreakpoints() {
	if (halted || breakpoints[mcu->cpu.PC]) {
		doHaltActions();
	}
}
void A32u4::Debugger::doHaltActions() {
	if (debugOutputMode == OutputMode_Log) {
		uint16_t word = mcu->flash.getInst(mcu->cpu.PC);
		uint16_t word2 = mcu->flash.getInst(mcu->cpu.PC + 1);

		if (!halted) {
			static int cnt = 0;
			mcu->log("Halted at : " + Disassembler::disassemble(word, word2, mcu->cpu.PC), ATmega32u4::LogLevel_Output); // + stringExtras::intToHex(PC * 2,4)
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
				mcu->log(debugStackToString(), ATmega32u4::LogLevel_Output);
			}
			else if (input == "R" || input == "RAll") {
				mcu->log(AllRegsToStr(), ATmega32u4::LogLevel_Output);
			}
			else if (input.substr(0, 1) == "R") {
				int ind = std::stoi(input.substr(1, input.length()));;
				if (ind < 32) {
					mcu->log(regToStr(ind), ATmega32u4::LogLevel_Output);
				}
				else {
					mcu->log(input + " is out of Range of the Register (R0-R31)", ATmega32u4::LogLevel_Output);
				}
			}
			else if (input.substr(0, 2) == "io") {
				int addr;
				if (input.substr(3, 2) == "0x") {
					addr = stringExtras::HexStrToInt(input.substr(3 + 2, input.length()).c_str());
				}
				else {
					addr = std::stoi(input.substr(3, input.length()));
				}

				if (addr < DataSpace::Consts::io_size + DataSpace::Consts::ext_io_size) {
					uint8_t ioVal = mcu->dataspace.getIOAt(addr);
					mcu->log((input + ": 0x" + stringExtras::intToHex(ioVal, 2) + " > " + std::to_string(ioVal)).c_str(), ATmega32u4::LogLevel_Output);
				}
				else {
					mcu->log(input + " is out of Range of the IO space (0-" + std::to_string(DataSpace::Consts::io_size + DataSpace::Consts::ext_io_size) + ")", ATmega32u4::LogLevel_Output);
				}
			}
			else if (input == "e" || input == "exit") {
				//TODO exit out of program
				// maybe this should also be removed
			}
			else if (input == "h" || input == "help") {
				mcu->log(getHelp(), ATmega32u4::LogLevel_Output);
			}
			else {
				mcu->log("Invalid command, try help", ATmega32u4::LogLevel_Output);
			}
		}
	}
	else {
		halted = true;
		doStep = false;
		mcu->cpu.breakOutOfOptim = true;
	}
}

std::string A32u4::Debugger::regToStr(uint8_t ind) const {
	uint8_t regVal = mcu->dataspace.data[ind];
	std::string regNumStr = stringExtras::paddRight(std::to_string(ind), 2, ' ');
	std::string decValStr = stringExtras::paddLeft(std::to_string(regVal), 3, ' ');
	return "R" + regNumStr + ": 0x" + stringExtras::intToHex(regVal, 2) + " > " + decValStr;
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
		str += stringExtras::intToHex(addressStack[i] * 2, 4) + " : at " + stringExtras::intToHex(fromAddressStack[i] * 2, 4);
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

bool A32u4::Debugger::isHalted() {
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
}
void A32u4::Debugger::setBreakpoint(uint16_t addr) {
	breakpoints[addr/2] = 1;
}
void A32u4::Debugger::clearBreakpoint(uint16_t addr) {
	breakpoints[addr/2] = 0;
}

const uint8_t* A32u4::Debugger::getBreakpoints() const {
	return breakpoints;
}
const uint16_t* A32u4::Debugger::getAddressStack() const {
	return addressStack;
}
const uint16_t A32u4::Debugger::getAddressStackPointer() const {
	return addressStackPointer;
}