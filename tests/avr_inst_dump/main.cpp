#include <fstream>
#include <iostream>
#include <vector>
#include <bitset>

#include "ATmega32u4.h"
#include "components/InstHandlerTemplates.h"

constexpr uint8_t A_REG_IND = 16;
constexpr uint8_t B_REG_IND = 17;

bool test_inst_name(std::ifstream& file, const char* inst_name) {
    char name_buf[8];
    file.read(name_buf, sizeof(name_buf));
    name_buf[sizeof(name_buf)-1] = 0;
    if(std::strcmp(inst_name, name_buf) != 0) {
        std::cerr << "expected inst " << inst_name << "; got " << name_buf << std::endl;
        return true;
    }
    return false;
}
bool test_inst_type(std::ifstream& file, uint8_t type) {
    char inst_type;
    file.read(&inst_type, 1);
    if(inst_type != type) {
        std::cerr << "expected inst type" << (int)type << "; got " << (int)inst_type << std::endl;
        return true;
    }
    return false;
}

bool check_res8(uint8_t ref_res, uint8_t res, const char* inst_name, uint8_t i, uint8_t j, uint8_t s) {
    if(res != ref_res) {
        std::cerr << "res mismatch: expected " << (int)ref_res << " got " << (int)res << " (" << inst_name << " " << (int)i << ", " << (int)j << "; sreg: " << (int)s << ")\n";
        return true;
    }
    return false;
}
bool check_res16(uint16_t ref_res, uint16_t res, const char* inst_name, uint16_t i, uint8_t j, uint8_t s) {
    if(res != ref_res) {
        std::cerr << "res mismatch: expected " << (int)ref_res << " got " << (int)res << " (" << inst_name << " " << (int)i << ", " << (int)j << "; sreg: " << (int)s << ")\n";
        return true;
    }
    return false;
}
void print_sreg(uint8_t sreg) {
    std::cerr 
        << 'I' << !!(sreg & (1 << 7))
        << 'T' << !!(sreg & (1 << 6))
        << 'H' << !!(sreg & (1 << 5))
        << 'S' << !!(sreg & (1 << 4))
        << 'V' << !!(sreg & (1 << 3))
        << 'N' << !!(sreg & (1 << 2))
        << 'Z' << !!(sreg & (1 << 1))
        << 'C' << !!(sreg & (1 << 0));
}
bool check_sreg(uint8_t ref_sreg, uint8_t sreg, const char* inst_name, uint16_t i, uint8_t j, uint8_t s) {
    if(sreg != ref_sreg) {
        std::cerr << "sreg mismatch:\n  expected: ";
        print_sreg(ref_sreg);
        std::cerr << "\n  got:      ";
        print_sreg(sreg);
        std::cerr << "\n (" << inst_name << " " << (int)i << ", " << (int)j << "; sreg: " << (int)s << ")\n";
        return true;
    }
    return false;
}

bool test_inst(A32u4::ATmega32u4& mcu, std::ifstream& file, const char* inst_name, uint16_t inst_word) {
    if(test_inst_name(file, inst_name)) return true;
    if(test_inst_type(file, 0)) return true;

    std::vector<uint8_t> data;
    data.resize(256*256*64*2);
    file.read((char*)data.data(), data.size());

    uint8_t i = 0; 
    uint8_t j = 0;
    uint8_t s = 0;
    size_t off = 0;
    while(true){ \
        mcu.dataspace.setGPReg(A_REG_IND, i);
        mcu.dataspace.setGPReg(B_REG_IND, j);
        mcu.dataspace.setDataByte(A32u4::DataSpace::Consts::SREG, s);
        A32u4::InstHandler::handleInstRawT<false>(&mcu, inst_word);

        uint8_t res = mcu.dataspace.getGPReg(A_REG_IND);
        uint8_t sreg = mcu.dataspace.getDataByte(A32u4::DataSpace::Consts::SREG);

        uint8_t ref_sreg = data[off+0];
        uint8_t ref_res = data[off+1];
        off += 2;

        if(check_res8(ref_res, res, inst_name, i, j, s)) return true;
        if(check_sreg(ref_sreg, sreg, inst_name, i, j, s)) return true;

        i++;
        if(i == 0) j++;
        if(i == 0 && j == 0) s++;
        if(i==0 && j==0 && s>=0b1000000) break;
    }

    return false;
}

bool test_inst_mul(A32u4::ATmega32u4& mcu, std::ifstream& file, const char* inst_name, uint16_t inst_word) {
    if(test_inst_name(file, inst_name)) return true;
    if(test_inst_type(file, 1)) return true;

    std::vector<uint8_t> data;
    data.resize(256*256*64*3);
    file.read((char*)data.data(), data.size());

    uint8_t i = 0; 
    uint8_t j = 0;
    uint8_t s = 0;
    size_t off = 0;
    while(true){ \
        mcu.dataspace.setGPReg(A_REG_IND, i);
        mcu.dataspace.setGPReg(B_REG_IND, j);
        mcu.dataspace.setDataByte(A32u4::DataSpace::Consts::SREG, s);
        A32u4::InstHandler::handleInstRawT<false>(&mcu, inst_word);

        uint16_t res = mcu.dataspace.getWordReg(0);
        uint8_t sreg = mcu.dataspace.getDataByte(A32u4::DataSpace::Consts::SREG);

        uint8_t ref_sreg = data[off+0];
        uint16_t ref_res = data[off+1] | ((uint16_t)data[off+2] << 8);
        off += 3;

        if(check_res16(ref_res, res, inst_name, i, j, s)) return true;
        if(check_sreg(ref_sreg, sreg, inst_name, i, j, s)) return true;

        i++;
        if(i == 0) j++;
        if(i == 0 && j == 0) s++;
        if(i==0 && j==0 && s>=0b1000000) break;
    }

    return false;
}

bool test_inst_unary(A32u4::ATmega32u4& mcu, std::ifstream& file, const char* inst_name, uint16_t inst_word) {
    if(test_inst_name(file, inst_name)) return true;
    if(test_inst_type(file, 2)) return true;

    std::vector<uint8_t> data;
    data.resize(256*64*2);
    file.read((char*)data.data(), data.size());

    uint8_t i = 0;
    uint8_t s = 0;
    size_t off = 0;
    while(true){ \
        mcu.dataspace.setGPReg(A_REG_IND, i);
        mcu.dataspace.setDataByte(A32u4::DataSpace::Consts::SREG, s);
        A32u4::InstHandler::handleInstRawT<false>(&mcu, inst_word);

        uint8_t res = mcu.dataspace.getGPReg(A_REG_IND);
        uint8_t sreg = mcu.dataspace.getDataByte(A32u4::DataSpace::Consts::SREG);

        uint8_t ref_sreg = data[off+0];
        uint16_t ref_res = data[off+1];
        off += 2;

        if(check_res8(ref_res, res, inst_name, i, 0, s)) return true;
        if(check_sreg(ref_sreg, sreg, inst_name, i, 0, s)) return true;

        i++; \
        if(i == 0) s++;
        if(i == 0 && s>=0b1000000) break;
    }

    return false;
}

bool test_instiw(A32u4::ATmega32u4& mcu, std::ifstream& file, const char* inst_name, uint16_t inst_word_base) {
    if(test_inst_name(file, inst_name)) return true;
    if(test_inst_type(file, 4)) return true;

    std::vector<uint8_t> data;
    data.resize(256*256*64*2*3);
    file.read((char*)data.data(), data.size());

    uint16_t i = 0; 
    uint8_t j = 0;
    uint8_t s = 0;
    size_t off = 0;
    while(true){ \
        mcu.dataspace.setWordReg(24, i);
        mcu.dataspace.setDataByte(A32u4::DataSpace::Consts::SREG, s);
        uint16_t inst_word = inst_word_base | ((j & 0b110000) << 2) | j & 0xf;
        A32u4::InstHandler::handleInstRawT<false>(&mcu, inst_word);

        uint16_t res = mcu.dataspace.getWordReg(24);
        uint8_t sreg = mcu.dataspace.getDataByte(A32u4::DataSpace::Consts::SREG);

        uint8_t ref_sreg = data[off+0];
        uint16_t ref_res = data[off+1] | ((uint16_t)data[off+2] << 8);
        off += 3;

        if(check_res16(ref_res, res, inst_name, i, j, s)) return true;
        if(check_sreg(ref_sreg, sreg, inst_name, i, j, s)) return true;

        i++;
        if(i == 0) j++;
        if(i == 0 && j == 64) { j = 0; s += 0b111111; }
        if(i==0 && j==0 && s>=0b1000000) break;
    }

    return false;
}

bool test_insti(A32u4::ATmega32u4& mcu, std::ifstream& file, const char* inst_name, uint16_t inst_word_base) {
    if(test_inst_name(file, inst_name)) return true;
    if(test_inst_type(file, 3)) return true;

    std::vector<uint8_t> data;
    data.resize(256*256*64*2);
    file.read((char*)data.data(), data.size());

    uint8_t i = 0; 
    uint8_t j = 0;
    uint8_t s = 0;
    size_t off = 0;
    while(true){ \
        mcu.dataspace.setGPReg(24, i);
        mcu.dataspace.setDataByte(A32u4::DataSpace::Consts::SREG, s);
        uint16_t inst_word = inst_word_base | ((j & 0xf0) << 4) | j & 0xf;
        A32u4::InstHandler::handleInstRawT<false>(&mcu, inst_word);

        uint8_t res = mcu.dataspace.getGPReg(24);
        uint8_t sreg = mcu.dataspace.getDataByte(A32u4::DataSpace::Consts::SREG);

        uint8_t ref_sreg = data[off+0];
        uint8_t ref_res = data[off+1];
        off += 2;

        if(check_res8(ref_res, res, inst_name, i, j, s)) return true;
        if(check_sreg(ref_sreg, sreg, inst_name, i, j, s)) return true;

        i++;
        if(i == 0) j++;
        if(i == 0 && j == 0) s++;
        if(i==0 && j==0 && s>=0b1000000) break;
    }

    return false;
}

int main(int argc, char** argv) {
    std::set_terminate([]() {
        try
        {
            std::exception_ptr eptr{std::current_exception()};
            if (eptr) {
                std::rethrow_exception(eptr);
            } else {
                std::cerr << "terminating without exception" << std::endl;
            }
        }
        catch (const std::exception& ex)
        {
            std::cerr << ex.what() << std::endl;
        }
        catch (...)
        {
            std::cerr << "Unknown exception" << std::endl;
        }
        std::exit(EXIT_FAILURE);
    });

    if(argc < 2) {
        std::cerr << "not enough args" << std::endl;
        return 1;
    }

    const char* test_file = argv[1];
    std::ifstream file;
    file.open(test_file, std::ios::binary);
    if(!file.is_open()) {
        std::cerr << "cannot open file: " << test_file << std::endl;
        return 1;
    }
    file.exceptions(std::ios::failbit);

    A32u4::ATmega32u4 mcu;
    mcu.reset();
    
    bool err = false;

    err = err || test_inst(mcu, file, "add", 0x0f01);
    err = err || test_inst(mcu, file, "adc", 0x1f01);
    err = err || test_inst(mcu, file, "and", 0x2301);
    err = err || test_inst(mcu, file, "cp",  0x1701);
    err = err || test_inst(mcu, file, "cpc", 0x0701);
    err = err || test_inst(mcu, file, "eor", 0x2701);
    err = err || test_inst(mcu, file, "or",  0x2b01);

    err = err || test_inst(mcu, file, "sub", 0x1b01);
    err = err || test_inst(mcu, file, "sbc", 0x0b01);

    err = err || test_inst_mul(mcu, file, "mul", 0x9f01);
    err = err || test_inst_mul(mcu, file, "muls", 0x0201);
    err = err || test_inst_mul(mcu, file, "mulsu", 0x0301);
    
    err = err || test_inst_mul(mcu, file, "fmul", 0x0309);
    err = err || test_inst_mul(mcu, file, "fmuls", 0x0381);
    err = err || test_inst_mul(mcu, file, "fmulsu", 0x0389);


    err = err || test_inst_unary(mcu, file, "ser", 0xef0f);
    err = err || test_inst_unary(mcu, file, "clr", 0x2700);
    err = err || test_inst_unary(mcu, file, "com", 0x9500);
    err = err || test_inst_unary(mcu, file, "neg", 0x9501);
    err = err || test_inst_unary(mcu, file, "inc", 0x9503);
    err = err || test_inst_unary(mcu, file, "dec", 0x950a);
    err = err || test_inst_unary(mcu, file, "tst", 0x2300);
    err = err || test_inst_unary(mcu, file, "lsl", 0x0f00);
    err = err || test_inst_unary(mcu, file, "lsr", 0x9506);
    err = err || test_inst_unary(mcu, file, "rol", 0x1f00);
    err = err || test_inst_unary(mcu, file, "ror", 0x9507);
    err = err || test_inst_unary(mcu, file, "asr", 0x9505);
    err = err || test_inst_unary(mcu, file, "swap", 0x9502);


    err = err || test_instiw(mcu, file, "adiw", 0x9600);
    err = err || test_instiw(mcu, file, "sbiw", 0x9700);

    err = err || test_insti(mcu, file, "andi", 0x7080);
    err = err || test_insti(mcu, file, "ori", 0x6080);
    err = err || test_insti(mcu, file, "cpi", 0x3080);
    err = err || test_insti(mcu, file, "subi", 0x5080);
    err = err || test_insti(mcu, file, "sbci", 0x4080);
    err = err || test_insti(mcu, file, "ldi", 0xe080);


    return err ? 1 : 0;
}