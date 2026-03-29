#ifndef NPU_WBM_H
#define NPU_WBM_H
#include "Module.h"
#include "Memory.h"
#include "Tensor.h"
#include <string>  

class WBM : public Module{
private:
    static const int ICH_PER_GROUP = 32;

    static const int BANK_LINE = 16;
    static const int BANK_COL = 1024;   
private:
    Sptr<Memory> bank_a_;
    Sptr<Memory> bank_b_;
    Sptr<Tensor<int>> input_;
    std::string dump_suffix_;

    void Init();
    bool SanityCheck();

    int offset_x_; 
    int offset_y_;
    int channel_st_;
    int channel_ed_;
    int AB_order_;

public:
    WBM() = default;
    WBM(std::string name);
    ~WBM() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetBank(Sptr<Memory> bank_a, Sptr<Memory> bank_b);
    void SetInputFM(Sptr<Tensor<int>> input);

};

#endif