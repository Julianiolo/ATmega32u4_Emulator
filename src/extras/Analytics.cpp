#include "../config.h"

#if MCU_INCLUDE_EXTRAS

#include "Analytics.h"

#include "StreamUtils.h"
#include "DataUtilsSize.h"

A32u4::Analytics::Analytics()
#if MCU_USE_HEAP
	: pcCounter(PCHeatArrSize),
	instCounter(InstHeatArrSize),
	ramReadBuf(DataSpace::Consts::data_size), ramWriteBuf(DataSpace::Consts::data_size)
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


void A32u4::Analytics::ramRead(addrmcu_t addr) {
	ramReadBuf[addr]++;
}
void A32u4::Analytics::ramWrite(addrmcu_t addr) {
	ramWriteBuf[addr]++;
}

void A32u4::Analytics::eepromRead(addrmcu_t addr) {

}
void A32u4::Analytics::eepromWrite(addrmcu_t addr) {

}

void A32u4::Analytics::romRead(addrmcu_t addr) {

}
void A32u4::Analytics::romWrite(addrmcu_t addr) {

}


const uint64_t* A32u4::Analytics::getRamRead() const {
	return &ramReadBuf[0];
}
const uint64_t* A32u4::Analytics::getRamWrite() const {
	return &ramWriteBuf[0];
}
void A32u4::Analytics::clearRamRead() {
	std::memset(&ramReadBuf[0], 0, ramReadBuf.size());
}
void A32u4::Analytics::clearRamWrite() {
	std::memset(&ramWriteBuf[0], 0, ramWriteBuf.size());
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

const uint64_t* A32u4::Analytics::getPCCntRaw() const{
	return &pcCounter[0];
}
const uint64_t* A32u4::Analytics::getInstHeat() const{
	return &instCounter[0];
}

uint64_t A32u4::Analytics::getTotalInstCnt() const {
	return instTotalCnt;
}

void A32u4::Analytics::resetPCCnt(){
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

	sum += sizeof(ramReadBuf);
	sum += sizeof(ramWriteBuf);
#else
	sum += DataUtils::approxSizeOf(pcCounter);
	sum += DataUtils::approxSizeOf(instCounter);

	sum += DataUtils::approxSizeOf(ramReadBuf);
	sum += DataUtils::approxSizeOf(ramWriteBuf);
#endif

	sum += sizeof(instTotalCnt);

	sum += sizeof(maxSP);
	sum += sizeof(sleepSum);

	return sum;
}

uint32_t A32u4::Analytics::hash() const noexcept{
	uint32_t h = 0;
	DU_HASHCC(h, pcCounter);
	DU_HASHCC(h, instCounter);

	DU_HASHCC(h, ramReadBuf);
	DU_HASHCC(h, ramWriteBuf);

	DU_HASHC(h, instTotalCnt);

	DU_HASHC(h, maxSP);
	DU_HASHC(h, sleepSum);

	return h;
}


#endif