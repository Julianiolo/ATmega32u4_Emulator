#include "Disassembler.h"

#include <algorithm>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <chrono>

#include "StringUtils.h"
#include "DataUtils.h"

#include "../ATmega32u4.h"

#include "../components/InstInds.h"
#include "../components/InstHandler.h"

#define LU_MODULE "Disassembler"

#define INST_PAR_TYPE_RAWVAL 0
#define INST_PAR_TYPE_RAWVALDEC 1
#define INST_PAR_TYPE_REG 2
#define INST_PAR_TYPE_ADDR 3
#define INST_PAR_TYPE_OFFSET 4
#define INST_PAR_TYPE_OFFSET16 5
#define INST_PAR_TYPE_REG_OFFSET 6

A32u4::Disassembler::DisasmData::DisasmData(size_t size) : disasmed(size) {

}
void A32u4::Disassembler::DisasmData::addFuncCallAddr(pc_t addr){
	if(funcCalls.find(addr) == funcCalls.end()){
		funcCalls.insert(addr);
	}
}

std::string A32u4::Disassembler::disassembleBinFile(const Flash& data, const AdditionalDisasmInfo& info){
	if (!data.isProgramLoaded()) {
		LU_LOG_(LogUtils::LogLevel_Error, "Could not disassemble because no program is loaded!");
		return "";
	}

	auto start = std::chrono::high_resolution_clock::now();

	DisasmData disasmData(data.sizeWords());

	for(addrmcu_t i = 0; i <= 0xa8;i+=4)
		disasmRecurse(i/2, data, disasmData);

#if MCU_INCLUDE_EXTRAS
	if (info.analytics) {
		for(size_t i = 0; i < std::min(data.sizeWords(), Analytics::PCHeatArrSize); i++){
			if (info.analytics->getPCCntRaw()[i] > 0 && !disasmData.disasmed[i])
				disasmRecurse((pc_t)i, data, disasmData);
		}
	}
#endif

	if (info.additionalDisasmSeeds) {
		for(size_t i = 0; i<info.additionalDisasmSeeds->size(); i++) {
			disasmRecurse((*info.additionalDisasmSeeds)[i]/2, data, disasmData);
		}
	}
	

	auto end0 = std::chrono::high_resolution_clock::now();
	{
		double ms = std::chrono::duration_cast<std::chrono::microseconds>(end0 - start).count()/1000.0;
		LU_LOGF_(LogUtils::LogLevel_DebugOutput, "walking program took %f ms", ms);
	}

	// generate content
	std::string content;
	{
		std::sort(disasmData.lines.begin(), disasmData.lines.end(), [](const DisasmData::Line& a, const DisasmData::Line& b) {
			return a.addr < b.addr;
		});

		size_t funcSymbInd = 0;
		size_t dataSymbInd = 0;

		size_t currSrcLinesInd = 0;

		addrmcu_t lastAddr = 0;
		for (size_t i = 0; i < disasmData.lines.size(); i++){
			auto& line = disasmData.lines[i];

			// draw data blocks
			if (info.dataSymbs) {
				while(dataSymbInd < info.dataSymbs->size() && line.addr * 2 > std::get<1>((*info.dataSymbs)[dataSymbInd])) {
					const std::string& dataSymbName = std::get<0>((*info.dataSymbs)[dataSymbInd]);
					uint32_t dataSymbValue = std::get<1>((*info.dataSymbs)[dataSymbInd]);
					uint32_t dataSymbSize = std::get<2>((*info.dataSymbs)[dataSymbInd]);


					content += StringUtils::format("\n%08x <%s>:\n", dataSymbValue, dataSymbName.c_str()); //symbol label

					for(size_t i = 0; i<dataSymbSize; i+=16){
						std::string bytes;
						for(size_t j = 0; j<std::min((size_t)16,dataSymbSize-i); j++){
							uint8_t byte = data.getByte((addrmcu_t)(dataSymbValue+i+j));
							bytes += StringUtils::uIntToHexStr(byte, 2);
							bytes += ' ';
						}

						std::string ascii;
						for(size_t j = 0; j<std::min((size_t)16,dataSymbSize-i); j++){
							char byte = data.getByte((addrmcu_t)(dataSymbValue+i+j));
							if(!StringUtils::isprint(byte)) byte = '.';
							ascii += byte;
						}

						// bytes padd amt = (2+1)*16=48
						content += StringUtils::format(
							"%8x:\t%-48s    %s\n",
							dataSymbValue+i, bytes.c_str(), ascii.c_str()
						);
					}

					dataSymbInd++;
					content += "\n";

					lastAddr = (addrmcu_t)(dataSymbValue + dataSymbSize);
				}
			}
			

			if (line.addr - lastAddr > (InstHandler::is2WordInst(data.getInst(line.addr))?4:2)) // make big gaps in program stand out
				content += "\n     ...\n\n";

			if (info.funcSymbs) {
				while (funcSymbInd < info.funcSymbs->size() && (*info.funcSymbs)[funcSymbInd].first < line.addr * 2) {
					funcSymbInd++;
				}

				if (funcSymbInd < info.funcSymbs->size() && (*info.funcSymbs)[funcSymbInd].first == line.addr * 2) {
					content += StringUtils::format("\n%08x <%s>:\n", line.addr*2, (*info.funcSymbs)[funcSymbInd].second.c_str());
					goto skip;
				}
			}
			if (disasmData.funcCalls.find(line.addr) != disasmData.funcCalls.end()) {
				// has a function
				content += StringUtils::format("\n%08x <func@%x>:\n", line.addr*2, line.addr*2);
			}
		skip:

			if (info.srcLines) {
				while (currSrcLinesInd < info.srcLines->size() && info.srcLines->at(currSrcLinesInd).first < line.addr*2) {
					currSrcLinesInd++;
				}

				if (currSrcLinesInd < info.srcLines->size() && info.srcLines->at(currSrcLinesInd).first == line.addr*2) {
					content += info.srcLines->at(currSrcLinesInd).second + '\n';
				}
			}

			content += line.str + "\n";

			switch(line.inst_ind){
				case IND_RET:
					DU_FALLTHROUGH;
				case IND_RETI:
					content += "\n";
					break;
			}

			lastAddr = line.addr;
		}

		
	}

	auto end = std::chrono::high_resolution_clock::now();
	{
		double ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000.0;
		LU_LOGF_(LogUtils::LogLevel_DebugOutput, "disassembly in total took %f ms", ms);
	}

	return content;
}

std::string A32u4::Disassembler::getParamStr(uint16_t val, uint8_t type) {
	switch (type)
	{
	case INST_PAR_TYPE_RAWVAL:
		return "0x" + StringUtils::uIntToHexStr(val, 2);
	case INST_PAR_TYPE_ADDR:
		return "0x" + StringUtils::uIntToHexStr(val, 4);

	case INST_PAR_TYPE_RAWVALDEC:
		return std::to_string(val);

	case INST_PAR_TYPE_REG:
		return "R" + std::to_string(val);

	case INST_PAR_TYPE_OFFSET: //never used
		return "." + getSignInt(((int8_t)val)*(int16_t)2);
	case INST_PAR_TYPE_OFFSET16: //never used
		return "." + getSignInt((int16_t)val*(int32_t)2);

	case INST_PAR_TYPE_REG_OFFSET:
		return val > 0 ? std::string("+") + std::to_string(val) : "";

	default:
		return "ERROR: " + std::to_string(type);
	}
}

std::string A32u4::Disassembler::disassemble(uint16_t word, uint16_t word2, uint16_t PC) {
	std::string disasm = disassembleRaw(word, word2);
	std::string instBytes = StringUtils::format("%02x %02x ", word&0xFF, (word & 0xFF00) >> 8);
	if (InstHandler::is2WordInst(word))
		instBytes += StringUtils::format("%02x %02x ", word2 & 0xFF, (word2 & 0xFF00) >> 8);
	else
		instBytes += "      ";

	std::string ret = StringUtils::format("    %4x:\t%s\t%s", PC*2, instBytes.c_str(), disasm.c_str());
	if(PC < 43*2) {
		uint8_t intNum = PC/2;
		const auto& info = ATmega32u4::interruptInfo[intNum];
		ret += StringUtils::format("  ; INT%" PRIu8 ": %s \"%s\"", intNum, info.source, info.definition);
	}
	return ret;
}
std::string A32u4::Disassembler::disassembleRaw(uint16_t word, uint16_t word2) {
	uint8_t Inst_ind = InstHandler::getInstInd3(word);
	if (Inst_ind >= IND_COUNT_)
		return StringUtils::format(".word\t0x%04x", word);
	InstHandler::Inst_ELEM inst = InstHandler::instList[Inst_ind];

	std::string out = inst.name;
	out += "\t";

	switch (Inst_ind) {
		case IND_EOR:
			if (inst.par1(word) == inst.par2(word)) {
				out = "clr\tR" + std::to_string(inst.par1(word));
				goto end_params;
			}
			else {
				goto sw1_def;
			}

		case IND_JMP:
		case IND_CALL:
			out += std::string("0x") + StringUtils::uIntToHexStr(word2 * 2, 4);
			break;

		case IND_RJMP:
		case IND_RCALL:
			out += std::string(".") + std::to_string(InstHandler::getk12_c_sin(word) * 2);
			break;

		case IND_LPM_0:
			out += "R0, Z"; break;

		default:
		sw1_def:
		{
			if (inst.par1 == NULL) {
				goto end_params;
			}
			out += getParamStr(inst.par1(word), inst.parTypes & 0xF);
		}
	}

	switch (Inst_ind) {
		case IND_LDS:
		case IND_STS:
			out += ", 0x" + StringUtils::uIntToHexStr(word2, 4);
			break;

		case IND_LD_X:
		case IND_ST_X:
			out += ", X"; break;
		case IND_LD_XpostInc:
		case IND_ST_XpostInc:
			out += ", X+"; break;
		case IND_LD_XpreDec:
		case IND_ST_XpreDec:
			out += ", -X"; break;

		case IND_LD_Y:
		case IND_ST_Y:
			out += ", Y"; break;
		case IND_LD_YpostInc:
		case IND_ST_YpostInc:
			out += ", Y+"; break;
		case IND_LD_YpreDec:
		case IND_ST_YpreDec:
			out += ", -Y"; break;

		case IND_LDD_Y:
		case IND_STD_Y:
			out += ", Y" + getParamStr(inst.par2(word), INST_PAR_TYPE_REG_OFFSET);
			break;

		case IND_LD_Z:
		case IND_ST_Z:
			out += ", Z"; break;
		case IND_LD_ZpostInc:
		case IND_ST_ZpostInc:
			out += ", Z+"; break;
		case IND_LD_ZpreDec:
		case IND_ST_ZpreDec:
			out += ", -Z"; break;

		case IND_LPM_d:
			out += ", Z"; break;
		case IND_LPM_dpostInc:
			out += ", Z+"; break;

		case IND_LDD_Z:
		case IND_STD_Z:
			out += ", Z" + getParamStr(inst.par2(word), INST_PAR_TYPE_REG_OFFSET);
			break;


		default:
		{
			if (inst.par2 == NULL) {
				goto end_params;
			}
			out += ", ";

			out += getParamStr(inst.par2(word), (inst.parTypes & 0xF0) >> 4);
		}
	}
end_params:

	return out;
}

std::string A32u4::Disassembler::getSignInt(int32_t val) {
	if (val >= 0) {
		return "+" +std::to_string(val);
	}
	else {
		return std::to_string(val);
	}
}
pc_t A32u4::Disassembler::getJumpDests(uint16_t word, uint16_t word2, pc_t pc) {
	uint8_t Inst_ind = InstHandler::getInstInd3(word);
	switch (Inst_ind) {
		case IND_JMP:
		{
			uint32_t k = InstHandler::getLongAddr(word,word2);
			return k;
		}
		case IND_RJMP:
		{
			int16_t k = InstHandler::getk12_c_sin(word);
			return pc + k + 1;
		}

		case IND_SBRC:
		case IND_SBRS:
		case IND_SBIC:
		case IND_SBIS:
		{
			return pc + 2;
		}

		case IND_BRBC:
		case IND_BRBS:
		{
			int8_t k = (int8_t)InstHandler::getk7_c_sin(word);
			return pc + k + 1;
		}


		case IND_CALL:
		{
			uint32_t k = InstHandler::getLongAddr(word, word2);
			return k;
		}
		case IND_RCALL:
		{
			int16_t k = InstHandler::getk12_c_sin(word);
			return pc + k + 1;
		}
	}
	return -1;
}

void A32u4::Disassembler::disasmRecurse(pc_t start, const Flash& data, DisasmData& disasmData){
	if (start >= data.sizeWords()) {
		LU_LOGF_(LogUtils::LogLevel_Error,"Reached illegal address while disassembling: 0x%" MCU_PRIxPC " (max is 0x%" MCU_PRIxPC ")", start, data.sizeWords());
		return;
	}

	pc_t PC = start;
	while(true){
		if (PC >= data.sizeWords() || disasmData.disasmed[PC]) {
			//printf("finished at %x from %x\n", PC*2, start*2);
			return;
		}

		uint16_t word = data.getInst(PC);
		uint16_t word2 = 0;
		uint8_t Inst_ind = InstHandler::getInstInd3(word);
		bool is2word = InstHandler::is2WordInst(word);
		if(is2word)
			word2 = data.getInst(PC+1);

		std::string disasm = disassemble(word, word2, PC);
		//std::cout << disasm << std::endl;
		disasmData.lines.push_back({ PC, disasm, Inst_ind });
		disasmData.disasmed[PC] = true;

		switch(Inst_ind){
			case IND_JMP:
			{
				uint32_t k = InstHandler::getLongAddr(word,word2);
				PC = k;
				continue;
			}
			case IND_RJMP:
			{
				int16_t k = InstHandler::getk12_c_sin(word);
				PC += k+1;
				continue;
			}

			case IND_SBRC:
			case IND_SBRS:
			case IND_SBIC:
			case IND_SBIS:
			case IND_CPSE:
			{
				disasmRecurse(PC+2, data, disasmData);
				break;
			}


			case IND_BRBC:
			case IND_BRBS:
			{
				int8_t k = (int8_t)InstHandler::getk7_c_sin(word);
				disasmRecurse(PC+k+1, data, disasmData);
				break;
			}


			case IND_CALL:
			{
				uint32_t k = InstHandler::getLongAddr(word, word2);
				disasmRecurse(k, data, disasmData);
				disasmData.addFuncCallAddr(k);
				break;
			}
			case IND_RCALL:
			{
				int16_t k = InstHandler::getk12_c_sin(word);
				if (k != 0) {
					disasmRecurse(PC+k+1, data, disasmData);
					disasmData.addFuncCallAddr(PC+k+1);
				}
				
				break;
			}

			case IND_RET:
			case IND_RETI:
				return;
		}

		PC++;
		if(is2word)
			PC++;
	}
}

/*

std::string out = disassembleRaw(word, word2);
std::string add = "" + stringExtras::intToHex(PC * 2, 4) + ":   ";
add += stringExtras::intToHex(word & 0xFF, 2) + " " + stringExtras::intToHex((word & 0xFF00) >> 8, 2);
if (!InstHandler::is2WordInst(word)) {
add += "       ";
}
else {
add += " " + stringExtras::intToHex(word2 & 0xFF, 2) + " " + stringExtras::intToHex((word2 & 0xFF00) >> 8, 2) + " ";
}
out = add + out;
*/