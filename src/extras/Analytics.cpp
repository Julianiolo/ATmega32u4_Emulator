#include "Analytics.h"

A32u4::Analytics::Analytics()
#if USE_HEAP
	: pcCounter(PCHeatArrSize),
	instCounter(InstHeatArrSize)
#endif
{

}

void A32u4::Analytics::reset() {
	for (int i = 0; i < InstHandler::instListLen; i++) {
		instCounter[i] = 0;
	}
	for (int i = 0; i < Flash::sizeMax / 2; i++) {
		pcCounter[i] = 0;
	}

	sleepSum = 0;
	maxSP = 0xFFFF;
}

void A32u4::Analytics::addData(uint8_t instInd, uint16_t PC) {
	instCounter[instInd]++;
	pcCounter[PC]++;
	instTotalCnt++;
}

uint64_t A32u4::Analytics::getPCCnt(uint16_t addr) const {
	return pcCounter[addr/2];
}

uint16_t A32u4::Analytics::findMostUsedPCCnt() {
	uint64_t max = 0;
	uint16_t maxInd = 0;
	for (int i = 0; i < A32u4::Flash::sizeMax / 2; i++) {
		if (pcCounter[i] > max) {
			max = pcCounter[i];
			maxInd = i;
		}
	}
	return maxInd;
}

const uint64_t* A32u4::Analytics::getPCHeat() const{
	return &pcCounter[0];
}
const uint64_t* A32u4::Analytics::getInstHeat() const{
	return &instCounter[0];
}

uint64_t A32u4::Analytics::getTotalInstCnt() const {
	return instTotalCnt;
}

void A32u4::Analytics::resetPCHeat(){
	for(uint16_t i = 0; i<PCHeatArrSize;i++){
		pcCounter[i] = 0;
	}
}

void A32u4::Analytics::getState(std::ostream& output){
	output.write((char*)&pcCounter[0], PCHeatArrSize);
	output.write((char*)&instCounter[0], InstHeatArrSize);

	output << maxSP;
	output << sleepSum;
}
void A32u4::Analytics::setState(std::istream& input){
	input.read((char*)&pcCounter[0], PCHeatArrSize);
	input.read((char*)&instCounter[0], InstHeatArrSize);

	input >> maxSP;
	input >> sleepSum;
}