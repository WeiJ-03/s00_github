#include "Bank.h"
/*
Bank::Bank(): memArray(nullptr) {

}

Bank::Bank(int len) {
    memArray = new Entry[len];
    length = len;
}

Bank::~Bank() {
    delete [] memArray;
}

void Bank::clear() {
    for (int i = 0; i < length; i++) {
        Entry & e = *(memArray + i);
        e.xF = e.xE = e.xD = e.xC = 0;
        e.xB = e.xA = e.x9 = e.x8 = 0;
        e.x7 = e.x6 = e.x5 = e.x4 = 0;
        e.x3 = e.x2 = e.x1 = e.x0 = 0;
    }
}

Entry Bank::read(int addr) {
    if (addr >= length) {
        throw "BankIndexOutOfBound";
    }
    return *(memArray + addr);
}

void Bank::write(int addr, Entry e) {
    if (addr >= length) {
        throw "BankIndexOutOfBound";
    }
    *(memArray + addr) = e;
}

Entry & Bank::operator[](int addr) {
    if (addr >= length) {
        throw "BankIndexOutOfBound";
    }
    return *(memArray + addr);
}

*/
