#ifndef NPU_PSUM_H
#define NPU_PSUM_H

#include "Module.h"
#include "Tensor.h"
#include "Memory.h"
#include "Weight.h"
#include <string>
using std::vector;
using std::string;

class Psum : public Module {
private:
    static const int OCH_PER_GROUP    = 64;
    static const int CONV_MODE_DW     = 3; 
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    Sptr<vector<string>> weights_;
    std::string dump_suffix_;
    Sptr<Memory> pmem_;


    int ich_num_;
    int psum_num_;
    int row_num_;
    int col_num_;
    int full_ch_;
    int ch_st_;
    int spad_offest_;
    int ich_group_num_;
    int pmem_col_num_;
    int trim_;
    int conv_mode_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoPsumAcc();
    void PreviousPsumAcc();
    void DoBias();
    void DoFineShift();
    void DoClamp();

public:
    Psum() = default;
    // Psum(std::string);
    ~Psum() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetWeight(Sptr<vector<string>> wt);
    // void SetPMEM(Sptr<Memory> pmem);
    void SetPMEM(Sptr<Memory> bank);
};

#endif