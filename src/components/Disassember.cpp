#include "Disassembler.h"
#include "InstInds.h"
#include "StringUtils.h"
#include "InstHandler.h"
#include <algorithm>
#include <fstream>
#include <streambuf>
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
const A32u4::Disassembler::DisasmFile::DisasmData* A32u4::Disassembler::DisasmFile::getDisasmData() const {
	return disasmData.get();
}

void A32u4::Disassembler::DisasmFile::loadSrc(const char* str, const char* strEnd) {
	if (strEnd == NULL)
		strEnd = str + std::strlen(str);

	content = std::string(str, strEnd);
	processContent();
}

bool A32u4::Disassembler::DisasmFile::loadSrcFile(const char* path) {
	bool success = false;
	content = StringUtils::loadFileIntoString(path, &success);

	if (!success)
		return false;

	processContent();
	return true;
}

void A32u4::Disassembler::DisasmFile::generateContent(const AdditionalDisasmInfo& info){
	content.clear();
	if(!disasmData)
		throw std::runtime_error("has no disasmData");
	
	std::sort(disasmData->lines.begin(),disasmData->lines.end(),compareLine);

	addrmcu_t lastAddr = 0;
	for (size_t i = 0; i < disasmData->lines.size(); i++){
		auto& line = disasmData->lines[i];

		if (line.addr - lastAddr > 512) // make big gaps in code stand out
			content += "\n     ...\n\n";

		if (disasmData.get()->funcCalls.find(line.addr) != disasmData.get()->funcCalls.end()) {
			if (info.getSymbolNameFromAddr) {
				std::string name;
				if (info.getSymbolNameFromAddr(line.addr*2, false, &name, info.symbolUserData)) {
					content += StringUtils::format("\n%08x <%s>:\n", line.addr*2, name.c_str());
					goto skip;
				}
			}
			content += StringUtils::format("\n%08x <func@%x>:\n", line.addr*2, line.addr*2);
		skip:
			;
		}
			

		if (info.getLineInfoFromAddr != nullptr) {
			std::string add;
			if (info.getLineInfoFromAddr(line.addr * 2, &add, info.lineUserData))
				content += add + "\n";
		}

		content += line.str + "\n";

		switch(line.inst_ind){
			case IND_RET:
			case IND_RETI:
				content += "\n";
				break;
		}

		lastAddr = line.addr;
	}
	
	processContent();
}

void A32u4::Disassembler::DisasmFile::disassembleBinFile(const Flash* data, const AdditionalDisasmInfo& info){
	if (!data->isProgramLoaded()) {
		content = "Could not disassemble because no program is loaded!";
		return;
	}

	if(!disasmData)
		addDisasmData(data->sizeWords());
	for(uint16_t i = 0; i <= 0xa8;i+=4)
		disasmRecurse(i/2,data, disasmData.get());

	if (info.analytics) {
		for(size_t i = 0; i < std::min(data->sizeWords(), Analytics::PCHeatArrSize); i++){
			if(info.analytics->getPCHeat()[i] > 0 && !disasmData.get()->disasmed.get(i))
				disasmRecurse((uint16_t)i, data, disasmData.get());
		}
	}

	generateContent(info);
}

uint16_t A32u4::Disassembler::DisasmFile::generateAddrFromLine(const char* start, const char* end) {
	if (start + 8 > end || *start != ' ' || start[8] != ':') {
		if (*start == '0' && start[8] == ' ' && isValidHexAddr(start,start+8))
			return Addrs_symbolLabel;
		else
			return Addrs_notAnAddr;
	}

	if(!isValidHexAddr(start,start+8))
		return Addrs_notAnAddr;

	return (uint16_t)StringUtils::hexStrToUIntLen<uint64_t>(start, 8);
}
bool A32u4::Disassembler::DisasmFile::isValidHexAddr(const char* start, const char* end) {
	static constexpr char validHexDigits[] = {' ','0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	for(const char* it = start; it < end; it++) {
		char c = *it;
		if(c >= 'A' && c <= 'Z') {
			c += ('a'-'A');
		}
		for(int i = 0; i<17;i++){
			if(c == validHexDigits[i])
				goto continue_outer;
		}
		return false;
	continue_outer:
		;
	}
	if(*(end-1) == ' ')
		return false;
	return true;
}
void A32u4::Disassembler::DisasmFile::addAddrToList(const char* start, const char* end, size_t lineInd) {
	uint16_t Addr = generateAddrFromLine(start, end);
	if(lineInd >= addrs.size()){
		addrs.resize(lineInd);
	}
	addrs[lineInd-1] = Addr;

	if (Addr == Addrs_symbolLabel) {
		// should never be bigger than 2 bytes
		addrmcu_t symbAddr = (addrmcu_t)StringUtils::hexStrToUIntLen<uint64_t>(start, 8);
		labels[symbAddr] = lineInd-1;
	}
}

void A32u4::Disassembler::DisasmFile::processBranches() {
	branchRoots.clear();
	branchRootInds.clear();
	branchRootInds.clear();

	branchRootInds.resize(lines.size(), -1);
	passingBranches.resize(lines.size());

	for (size_t i = 0; i < lines.size(); i++) {
		addrmcu_t addr = addrs[i];
		if (addr != Addrs_notAnAddr && addr != Addrs_symbolLabel) {
			addrmcu_t dest;
			{
				const char* lineStart = content.c_str() + lines[i];
				const char* line_end = content.c_str() + ((i + 1 < lines.size()) ? lines[i] : content.size());

				uint16_t word = ( StringUtils::hexStrToUIntLen<uint16_t>(lineStart+FileConsts::instBytesStart,   2)) |
					( StringUtils::hexStrToUIntLen<uint16_t>(lineStart+FileConsts::instBytesStart+3, 2) << 8);
				uint16_t word2 = 0;
				if(*(lineStart+FileConsts::instBytesStart+3+3) != ' ') {
					word2 =		( StringUtils::hexStrToUIntLen<uint16_t>(lineStart+FileConsts::instBytesStart+3+3,   2)) |
						( StringUtils::hexStrToUIntLen<uint16_t>(lineStart+FileConsts::instBytesStart+3+3+3, 2) << 8);
				}

				pc_t destPC = Disassembler::getJumpDests(word,word2,addr/2);
				if (destPC == (pc_t)-1)
					continue; // instruction doesn't jump anywhere
				dest = destPC * 2;
			}
			
			size_t destLine = getLineIndFromAddr(dest);

			if (addrs[destLine] != dest) {
				continue; // kinda weird that that is neccessary
			}

			size_t branchRootInd = branchRoots.size();
			branchRootInds[i] = branchRootInd;

			branchRoots.push_back(BranchRoot());
			BranchRoot& branchRoot = branchRoots.back();
			branchRoot.start = addr;
			branchRoot.dest = dest;
			branchRoot.startLine = i;
			branchRoot.destLine = destLine;

			

			size_t from = std::min(i, destLine);
			size_t to = std::max(i, destLine);

			bool isLongBranch = to - from >= distOfLongBranch;
			branchRoot.displayFully = !isLongBranch;
			
			if (!isLongBranch) {
				uint16_t maxDepth = 0;
				for (size_t l = from; l <= to; l++) {
					auto& passing = passingBranches[l];
					if (passing.size() > maxDepth)
						maxDepth = passing.size();

					passing.push_back(branchRootInd);
				}

				branchRoot.displayDepth = maxDepth;
				maxBranchDisplayDepth = std::max(maxBranchDisplayDepth, maxDepth);
			}
			else {
				passingBranches[from].push_back(branchRootInd);
				passingBranches[to].push_back(branchRootInd);
			}
			

			
		}
	}
}

void A32u4::Disassembler::DisasmFile::processContent() {
	size_t lineInd = 1;
	lines.clear();
	lines.push_back(0);
	addrs.clear();

	const char* str = content.c_str();
	size_t i = 0;
	for(; i < content.size(); i++) {
		if(str[i] == '\n'){
			lines.push_back(i+1);

			addAddrToList(str + lines[lineInd - 1], str + i, lineInd);

			lineInd++;
		}
	}
	addAddrToList(str + lines[lineInd - 1], str + content.size(), lineInd);
	//lineInd++;

	lines.resize(lineInd);
	addrs.resize(lineInd);

	processBranches();

	for (size_t i = 0; i < lines.size(); i++) {
		addrmcu_t addr = addrs[i];
		if (addr != Addrs_notAnAddr && addr != Addrs_symbolLabel) {

		}
	}
}

size_t A32u4::Disassembler::DisasmFile::getLineIndFromAddr(uint16_t Addr) const{
	if(isEmpty())
		return -1;

	size_t from = 0;
	size_t to = lines.size()-1;
	while ((addrs[to] == Addrs_notAnAddr || addrs[to] == Addrs_symbolLabel) && to > 0)
		to--;
	if (addrs[to] < Addr || to == 0)
		return 0;

	while(from != to){
		size_t mid = from + ((to-from)/2);

		uint16_t lineAddr;
		while((lineAddr = addrs[mid]) == Addrs_notAnAddr || lineAddr == Addrs_symbolLabel)
			mid++;

		if(lineAddr == Addr){
			return mid;
		}
		else {
			if (lineAddr < Addr) {
				if(mid == from)
					break;
				from = mid;
			}else{
				if(mid == to)
					break;
				to = mid;
			}
		}
	}
	return from;
}
bool A32u4::Disassembler::DisasmFile::isEmpty() const {
	return content.size() == 0;
}
size_t A32u4::Disassembler::DisasmFile::getNumLines() const {
	return lines.size();
}
bool A32u4::Disassembler::DisasmFile::isSelfDisassembled() const {
	return disasmData.get() != nullptr;
}

A32u4::Disassembler::DisasmFile::DisasmData::DisasmData(size_t size) : disasmed(size){

}
void A32u4::Disassembler::DisasmFile::DisasmData::addFuncCallAddr(pc_t addr){
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
	std::string instBytes = StringUtils::format("%02x %02x ", word&0xFF, (word & 0xFF00) >> 8);
	if (InstHandler::is2WordInst(word))
		instBytes += StringUtils::format("%02x %02x ", word2 & 0xFF, (word2 & 0xFF00) >> 8);
	else
		instBytes += "      ";
	return StringUtils::format("    %4x:\t%s\t%s", PC*2, instBytes.c_str(), disasm.c_str());
}
std::string A32u4::Disassembler::disassembleRaw(uint16_t word, uint16_t word2) {
	uint8_t Inst_ind = InstHandler::getInstInd3(word);
	if (Inst_ind >= IND_COUNT_)
		return StringUtils::format(".word 0x%04x", word);
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

void A32u4::Disassembler::disasmRecurse(pc_t start, const Flash* data, DisasmFile::DisasmData* disasmData){
	pc_t PC = start;
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
				if (k != 0) {
					disasmRecurse(PC+k+1, data, disasmData);
					disasmData->addFuncCallAddr(PC+k+1);
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