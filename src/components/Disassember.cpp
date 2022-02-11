#include "Disassembler.h"
#include "InstInds.h"
#include "../utils/StringUtils.h"
#include "InstHandler.h"
#include <algorithm>
#include <iostream>

#define INST_PAR_TYPE_RAWVAL 0
#define INST_PAR_TYPE_RAWVALDEC 1
#define INST_PAR_TYPE_REG 2
#define INST_PAR_TYPE_ADDR 3
#define INST_PAR_TYPE_OFFSET 4
#define INST_PAR_TYPE_OFFSET16 5
#define INST_PAR_TYPE_REG_OFFSET 6

static bool compareLine(const A32u4::Disassembler::DisasmFile::DisasmData::Line& a, const A32u4::Disassembler::DisasmFile::DisasmData::Line& b) {
	return a.addr < b.addr;
}

A32u4::Disassembler::DisasmFile::DisasmFile(){

}
void A32u4::Disassembler::DisasmFile::addDisasmData(size_t size){
	disasmData = std::make_shared<DisasmData>(size);
}
void A32u4::Disassembler::DisasmFile::generateContent(){
	content.clear();
	if(!disasmData)
		throw std::runtime_error("has no disasmData");
	
	std::sort(disasmData->lines.begin(),disasmData->lines.end(),compareLine);

	at_addr_t lastAddr = 0;
	for (size_t i = 0; i < disasmData->lines.size(); i++){
		auto& line = disasmData->lines[i];

		if (line.addr - lastAddr > 512) // make big gaps in code stand out
			content += "\n     ...\n\n";

		if(disasmData.get()->funcCalls.find(i) != disasmData.get()->funcCalls.end())
			content += StringUtils::format("\n%08x <func@%x>:\n", line.addr, line.addr).get();

		content += line.str + "\n";

		switch(line.inst_ind){
			case IND_RET:
			case IND_RETI:
				content += "\n";
				break;
		}

		lastAddr = line.addr;
	}
		
}

void A32u4::Disassembler::DisasmFile::disassembleBinFile(const Flash* data){
	if (!data->isProgramLoaded()) {
		content = "Could not disassemble because no program is loaded!";
		return;
	}

	if(!disasmData)
		addDisasmData(data->sizeWords());
	for(uint16_t i = 0; i <= 0xa8;i+=4)
		disasmRecurse(i/2,data, disasmData.get());

	generateContent();
}
void A32u4::Disassembler::DisasmFile::disassembleBinFileWithAnalytics(const Flash* data, const Analytics* analytics){
	if (!data->isProgramLoaded()) {
		content = "Could not disassemble because no program is loaded!";
		return;
	}

	if(!disasmData)
		addDisasmData(data->sizeWords());
	for(uint16_t i = 0; i <= 0xa8;i+=4)
		disasmRecurse(i/2, data, disasmData.get());

	for(size_t i = 0; i < std::min(data->sizeWords(), Analytics::PCHeatArrSize); i++){
		if(analytics->getPCHeat()[i] > 0 && !disasmData.get()->disasmed.get(i))
			disasmRecurse((uint16_t)i, data, disasmData.get());
	}

	generateContent();
}

A32u4::Disassembler::DisasmFile::DisasmData::DisasmData(size_t size) : disasmed(size){

}
void A32u4::Disassembler::DisasmFile::DisasmData::addFuncCallAddr(uint16_t addr){
	if(funcCalls.find(addr) == funcCalls.end()){
		funcCalls.insert(addr);
	}
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
		return std::to_string(val);

	default:
		return "ERROR: " + std::to_string(type);
	}
}

std::string A32u4::Disassembler::disassemble(uint16_t word, uint16_t word2, uint16_t PC) {
	std::string disasm = disassembleRaw(word, word2);
	std::string instBytes = StringUtils::format("%02x %02x ", word&0xFF, (word & 0xFF00) >> 8).get();
	if (InstHandler::is2WordInst(word))
		instBytes += StringUtils::format("%02x %02x ", word2 & 0xFF, (word2 & 0xFF00) >> 8).get();
	else
		instBytes += "      ";
	return StringUtils::format("    %4x:\t%s\t%s", PC*2, instBytes.c_str(), disasm.c_str()).get();
}
std::string A32u4::Disassembler::disassembleRaw(uint16_t word, uint16_t word2) {
	uint8_t Inst_ind = InstHandler::getInstInd3(word);
	if (Inst_ind >= IND_COUNT_)
		return "";
	InstHandler::Inst_ELEM inst = InstHandler::instList[Inst_ind];

	std::string out = inst.name;
	out += "\t";

	switch (Inst_ind)
	{
	case IND_EOR:
		if (inst.par1(word) == inst.par2(word)) {
			out = "CLR R" + std::to_string(inst.par1(word));
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
		out += " R0, Z"; break;

	default:
	sw1_def:
	{
		if (inst.par1 == NULL) {
			goto end_params;
		}
		out += getParamStr(inst.par1(word), inst.parTypes & 0xF);
	}
	}

	switch (Inst_ind)
	{
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

void A32u4::Disassembler::disasmRecurse(uint16_t start, const Flash* data, DisasmFile::DisasmData* disasmData){
	uint16_t PC = start;
	while(true){
		if (disasmData->disasmed.get(PC)) {
			//printf("finished at %x from %x\n", PC*2, start*2);
			return;
		}

		uint16_t word = data->getInst(PC);
		uint16_t word2 = 0;
		uint8_t Inst_ind = InstHandler::getInstInd3(word);
		bool is2word = InstHandler::is2WordInst(word);
		if(is2word)
			word2 = data->getInst(PC+1);

		std::string disasm = disassemble(word, word2, PC);
		//std::cout << disasm << std::endl;
		disasmData->lines.push_back({ PC, disasm, Inst_ind });
		disasmData->disasmed.set(PC,true);

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
				disasmData->addFuncCallAddr(k);
				break;
			}
			case IND_RCALL:
			{
				int16_t k = InstHandler::getk12_c_sin(word);
				disasmRecurse(PC+k+1, data, disasmData);
				disasmData->addFuncCallAddr(PC+k+1);
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