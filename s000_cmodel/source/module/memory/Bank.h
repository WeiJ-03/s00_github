//
// Created by mingzhe on 5/25/19.
//

#ifndef NPU_BANK_H
#define NPU_BANK_H

#include <cstdint>
//#include "utility.h"

class Bank {
private:
    //Entry * memArray;
    //int length;

public:
    Bank() = default;
    //Bank(int len);
    ~Bank() = default;


    //void clear(); // Make all contents in the memArray to be 0
    //Entry read(int addr); // Note that this is 128-bit align address
    //void write(int addr, Entry e); // Note that this is 128-bit align address

    //Entry & operator[](int addr);

};

#endif //NPU_BANK_H
