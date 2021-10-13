#ifndef __BITARRAY_H__
#define __BITARRAY_H__

#include <stdint.h>

class BitArray{
private:
    typedef size_t bsize_t;

    
    bsize_t size;
    size_t byteSize;
    uint8_t* data = nullptr;
public:
    BitArray(bsize_t size);
    ~BitArray();
    bool get(bsize_t i);
    void set(bsize_t i, bool val);
};

#endif