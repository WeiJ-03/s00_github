#ifndef NPU_UTILITY_H
#define NPU_UTILITY_H
#include <bitset>

struct Space {
    int offset_x;
    int offset_y;
    int h;
    int w;
    int l;
    int offset_s;
    int s;
};

struct Position {
    int r, c, ch;
    int rStep, cStep, chStep;

    Position():r(0), c(0), ch(0), rStep(0), cStep(0), chStep(0) {};
    Position(int r, int c, int ch, int rStep, int cStep, int chStep):
            r(r), c(c), ch(ch), rStep(rStep), cStep(cStep), chStep(chStep) {};
};

struct Entry {
    int8_t xF;
    int8_t xE;
    int8_t xD;
    int8_t xC;
    int8_t xB;
    int8_t xA;
    int8_t x9;
    int8_t x8;
    int8_t x7;
    int8_t x6;
    int8_t x5;
    int8_t x4;
    int8_t x3;
    int8_t x2;
    int8_t x1;
    int8_t x0;
};


#endif //NPU_UTILITY_H
