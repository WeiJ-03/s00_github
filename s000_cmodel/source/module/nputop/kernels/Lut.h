#ifndef NPU_LUT_H
#define NPU_LUT_H

#include "Module.h"
#include "Tensor.h"
#include "Memory.h"

class LUT : public Module {
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    Sptr<Memory> mem_;
    int LUT_en_;
    int LUT_dma_;
    int row_num_;
    int col_num_;
    int ch_num_;

    void Init();
    void DoLUTCh();
    // void DoLUTCh1();
    // void DoLUTCh4();
    // void DoLUTCh16();
    bool SanityCheck();
    void CreateOutFM();

public:
    LUT() = default;
    ~LUT() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetLUTMEM(Sptr<Memory> lut_mem);
};

#endif //NPU_LUT_H
