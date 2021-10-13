#include "bitArray.h"

BitArray::BitArray(bsize_t size) : size(size), byteSize(size/8+(size%8==0?0:1)), data(new uint8_t[byteSize]){
    for(size_t i = 0; i<byteSize;i++) {
        data[i] = 0;
    }
}
BitArray::~BitArray(){
    if(data)
        delete data;
}
bool BitArray::get(bsize_t i){
    if(i < size)
        return (data[i/8] & (1 << i%8)) != 0;
}
void BitArray::set(bsize_t i, bool val){
    if(val){
        data[i/8] |= (1 << i%8);
    }
    else{
        data[i/8] &= ~(1 << i%8);
    }
}