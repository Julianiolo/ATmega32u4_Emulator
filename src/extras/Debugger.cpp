#include "Debugger.h"

#include <iostream>
#include <stdint.h>

#include "StringUtils.h"
#include "StreamUtils.h"

#include "../ATmega32u4.h"
#include "Disassembler.h"



A32u4::Debugger::Debugger(ATmega32u4* mcu):
	mcu(mcu)
#if !DEBUGGER_STD
#if USE_HEAP
	, breakpoints(new Breakpoint[breakPointArrMaxSize]),
	callStack(new CallData[addressStackMaxSize]),
	addressStackIndicators(new uint8_t[DataSpace::Consts::ISRAM_size])
#endif
#else
#if USE_HEAP
	, breakpoints(breakPointArrMaxSize),
	callStack(addressStackMaxSize),
	addressStackIndicators(DataSpace::Consts::ISRAM_size)
#endif
#endif
{
	resetBreakpoints();
}

A32u4::Debugger::~Debugger() {
#if !DEBUGGER_STD
#if USE_HEAP
	delete[] breakpoints;
	delete[] callStack;
	delete[] addressStackIndicators;
#endif
#else

#endif
}

uint8_t A32u4::Debugger::debugOutputMode = OutputMode_Log;
bool A32u4::Debugger::printDisassembly = false;

void A32u4::Debugger::reset() {
	halted = false;
	doStep = false;
	skipCycs = -1;

	callStackPtr = 0;
	for (int i = 0; i < DataSpace::Consts::ISRAM_size; i++) {
		addressStackIndicators[i] = false;
	}
}
void A32u4::Debugger::resetBreakpoints(){
	for (int i = 0; i < breakPointArrMaxSize; i++) {
		breakpoints[i] = 0;
	}
}

void A32u4::Debugger::pushPCOnCallStack(pc_t pc, pc_t fromPC) {
	//return;
	A32U4_ASSERT_INRANGE_M(callStackPtr+1, 0, addressStackMaxSize, A32U4_ADDR_ERR_STR("Debug Address Stack overflow: ",callStackPtr,4), "Debugger", return);
	callStack[callStackPtr++] = CallData{pc, fromPC};
}
void A32u4::Debugger::popPCFromCallStack() {
	//return;
	A32U4_ASSERT_INRANGE_M(callStackPtr, 1, addressStackMaxSize, A32U4_ADDR_ERR_STR("Debug Address Stack underflow: ",callStackPtr,4), "Debugger", return);

	callStackPtr--;
	callStack[callStackPtr] = CallData{0,0};
}

void A32u4::Debugger::registerAddressBytes(addrmcu_t addr) {
	addr -= DataSpace::Consts::ISRAM_start;
	addressStackIndicators[addr] = 1;
	addressStackIndicators[addr - 1] = 2;
	lastSPRecived = addr;
}

void A32u4::Debugger::registerStackDec(addrmcu_t addr){
	addr -= DataSpace::Consts::ISRAM_start;
	for(uint16_t i = lastSPRecived-1; i<=addr; i++){
		if(addressStackIndicators[i] == 1)
			popPCFromCallStack();
		addressStackIndicators[i] = 0;
	}
	lastSPRecived = addr;
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
		if (skipCycs == mcu->cpu.getTotalCycles()) {
			halted = false;
			ret = false;
		}
			
		else
			halted = true;
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
			uint64_t addr = StringUtils::numStrToUInt<uint64_t>(input.c_str() + 3, input.c_str() + input.size());

			if (addr < DataSpace::Consts::io_size + DataSpace::Consts::ext_io_size) {
				uint8_t ioVal = mcu->dataspace.getIOAt((uint8_t)addr);
				mcu->logf(ATmega32u4::LogLevel_Output, "%d: 0x%02h > %d", addr, ioVal, ioVal); // (input + ": 0x" + stringExtras::intToHex(ioVal, 2) + " > " + std::to_string(ioVal)).c_str());
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

std::string A32u4::Debugger::regToStr(regind_t ind) const {
	uint8_t regVal = mcu->dataspace.data[ind];
	return StringUtils::format("R%2d: 0x%02x > %3d", ind, regVal, regVal);
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
	for (int i = callStackPtr - 1; i >= 0; i--) {
		const CallData& call = callStack[i];
		str += StringUtils::format("%04x : at %04x", call.to * 2, call.from * 2);
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
	//doStep = true;
	skipCycs = mcu->cpu.getTotalCycles();
}
void A32u4::Debugger::setBreakpoint(pc_t pc) {
	breakpoints[pc] = 1;
	breakpointList.insert(pc);
}
void A32u4::Debugger::clearBreakpoint(pc_t pc) {
	breakpoints[pc] = 0;
	if (breakpointList.find(pc) != breakpointList.end())
		breakpointList.erase(pc);
}
const std::set<uint16_t>& A32u4::Debugger::getBreakpointList() const {
	return breakpointList;
}

const A32u4::Debugger::Breakpoint* A32u4::Debugger::getBreakpoints() const {
	return &breakpoints[0];
}
const A32u4::Debugger::CallData* A32u4::Debugger::getCallStack() const {
	return &callStack[0];
}
uint16_t A32u4::Debugger::getCallStackPointer() const {
	return callStackPtr;
}
uint16_t A32u4::Debugger::getPCAt(uint16_t stackInd) const {
	return callStack[stackInd].to;
}
uint16_t A32u4::Debugger::getFromPCAt(uint16_t stackInd) const {
	return callStack[stackInd].from;
}
const uint8_t* A32u4::Debugger::getAddressStackIndicators() const {
	return &addressStackIndicators[0];
}

void A32u4::Debugger::updateBreakpointListFromArr() {
	breakpointList.clear();
	for(size_t i = 0; i<breakPointArrMaxSize; i++){
		if(breakpoints[i])
			breakpointList.insert(i);
	}
}

void A32u4::Debugger::getState(std::ostream& output){
	StreamUtils::write(output, callStackPtr);
	StreamUtils::write(output, halted);

	output.write((const char*)&breakpoints[0], breakPointArrMaxSize);
	output.write((const char*)&callStack[0], addressStackMaxSize);
	output.write((const char*)&addressStackIndicators[0], addressStackIndicatorsSize);
}
void A32u4::Debugger::setState(std::istream& input){
	StreamUtils::read(input, &callStackPtr);
	StreamUtils::read(input, &halted);

	input.read((char*)&breakpoints[0], breakPointArrMaxSize);
	input.read((char*)&callStack[0], addressStackMaxSize);
	input.read((char*)&addressStackIndicators[0], addressStackIndicatorsSize);

	updateBreakpointListFromArr();
}

bool A32u4::Debugger::CallData::operator==(const CallData& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(from) && _CMP_(to);
#undef _CMP_
}


bool A32u4::Debugger::operator==(const Debugger& other) const{
#define _CMP_(x) (x==other.x)
	return _CMP_(callStackPtr) && _CMP_(halted) &&
		_CMP_(breakpoints) && _CMP_(callStack) && _CMP_(addressStackIndicators);
#undef _CMP_
}


/*

void A32u4::Debugger::clearAddressByte(uint16_t addr) {
	//static bool doPopAddress = false;

	addr -= DataSpace::Consts::ISRAM_start;

	if (addressStackIndicators[addr] == 1) {
		//if (doPopAddress) {
		popPCFromCallStack();
		//}
		//doPopAddress = !doPopAddress;
	}

	addressStackIndicators[addr] = 0;
}
void A32u4::Debugger::clearAddressByteRaw(uint16_t addr) {
	addr -= DataSpace::Consts::ISRAM_start;

	addressStackIndicators[addr] = 0;
}

std::string regNumStr = stringExtras::paddRight(std::to_string(ind), 2, ' ');
std::string decValStr = stringExtras::paddLeft(std::to_string(regVal), 3, ' ');
return "R" + regNumStr + ": 0x" + stringExtras::intToHex(regVal, 2) + " > " + decValStr;
*/