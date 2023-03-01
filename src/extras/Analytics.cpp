#include "../config.h"

#if MCU_INCLUDE_EXTRAS

#include "Analytics.h"

#include "StreamUtils.h"

A32u4::Analytics::Analytics()
#if MCU_USE_HEAP
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

	StreamUtils::write(output, maxSP);
	StreamUtils::write(output, sleepSum);
}
void A32u4::Analytics::setState(std::istream& input){
	input.read((char*)&pcCounter[0], PCHeatArrSize);
	input.read((char*)&instCounter[0], InstHeatArrSize);

	StreamUtils::read(input, &maxSP);
	StreamUtils::read(input, &sleepSum);
}

bool A32u4::Analytics::operator==(const Analytics& other) const{
#define _CMP_(x) (x==other.x)
	return pcCounter == other.pcCounter && instCounter == other.instCounter &&
		_CMP_(maxSP) && _CMP_(sleepSum);
#undef _CMP_
}
size_t A32u4::Analytics::sizeBytes() const {
	size_t sum = 0;
	
#if !MCU_USE_HEAP
	sum += sizeof(pcCounter);
	sum += sizeof(instCounter);
#else
	sum += sizeof(pcCounter) + sizeof(pcCounter[0]) * PCHeatArrSize;
	sum += sizeof(instCounter) + sizeof(instCounter[0]) * InstHeatArrSize;
#endif

	sum += sizeof(instTotalCnt);

	sum += sizeof(maxSP);
	sum += sizeof(sleepSum);

	return sum;
}

#endif