#include "bitArray.h"
#include <stdexcept>

BitArray::BitArray(){

}
BitArray::BitArray(bsize_t size) : size(size), byteSize(size/8+(size%8==0?0:1)), data(new uint8_t[byteSize]){
    for(size_t i = 0; i<byteSize;i++) {
        data[i] = 0;
    }
}
BitArray::~BitArray(){
    if(data)
        delete data;
    data = nullptr;
}
BitArray::BitArray(const BitArray& a) {
    if (data)
        delete data;
    initSizeTo(a.size);
    for(size_t i = 0; i<byteSize;i++) {
        data[i] = a.data[i];
    }
}
bool BitArray::get(bsize_t i){
    if(i < size)
        std::runtime_error("Bytearray get index out of bounds");
    return (data[i/8] & (1 << i%8)) != 0;
}
void BitArray::set(bsize_t i, bool val){
    if (i / 8 >= byteSize)
        std::runtime_error("Byte array set index out of bounds");
    if(val){
        data[i/8] |= (1 << i%8);
    }
    else{
        data[i/8] &= ~(1 << i%8);
    }
}

void BitArray::initSizeTo(size_t len) {
    size = len;
    byteSize = size / 8 + (size % 8 == 0 ? 0 : 1);
    data = new uint8_t[byteSize];
}

BitArray& BitArray::operator=(const BitArray& a) {
    if (data)
        delete data;
    initSizeTo(a.size);
    for(size_t i = 0; i<byteSize;i++) {
        data[i] = a.data[i];
    }
    return *this;
}