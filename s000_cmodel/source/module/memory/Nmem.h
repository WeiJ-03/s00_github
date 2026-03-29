#ifndef NPU_NMEM_H
#define NPU_NMEM_H

#include "Bank.h"
//#include "Tensor.h"
//#include "utility.h"
#include "RegisterFile.h"
//#include "NPUConfig.h"

class Nmem {
private:
    //Bank memory[NUM_BANK];
    //Bank memory[16];

    RegisterFile * regFile;

    //Space nmem_fm;
    //Space nmem_ps;
    //Space nmem_st;
    //Space nmem_pi;
    //Space nmem_po;
    //Space nmem_rdma;
    //Space nmem_wdma;


    //positionToMem(Position p);
public:
    Nmem() = default;
    ~Nmem() = default;

    //void init(RegisterFile * rf);
    //void spaceAllocate();

    //Tensor read(std::string dest, Position p);
    //void write(std::string dest, Position p, Tensor t);

};

#endif //NPU_NMEM_H
