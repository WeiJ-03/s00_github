#ifndef NPU_RBM_H
#define NPU_RBM_H
#include "Module.h"
#include "Memory.h"
#include "Tensor.h"
#include <string>

class RBM : public Module{
private:
    static const int ICH_PER_GROUP          = 32;
    static const int QUARTER_PER_GROUP      = 32;
    static const int CHANNEL_PER_GROUP      = 32;


    static const int CONV_MODE_BYPASS           = 0;            
    static const int CONV_MODE_3x3_fullCH       = 1;           
    static const int CONV_MODE_3x3_QuarterCH    = 2;           
    static const int CONV_MODE_DW               = 3;           
    static const int CONV_MODE_1x1              = 4;          
    static const int CONV_MODE_ADD              = 5;           


    static const int BANK_LINE           = 16;
    static const int BANK_COL            = 1024;   
private:
    Sptr<Memory> bank_a_;
    Sptr<Memory> bank_b_;
    Sptr<Tensor<int>> fm_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;

    int conv_mode_;
    int ich_num_;
    int row_num_;
    int col_num_;
    int pad_l_;
    int pad_r_;
    int pad_t_;
    int pad_b_;
    int crop_row_st_;
    int crop_col_st_;
    int crop_row_out_;
    int crop_col_out_;
    int channel_offset_;
    int channel_st_;
    int channel_ed_;
    int offset_x1_;
    int offset_y1_;
    int offset_x2_;
    int offset_y2_;
    int psum_dim_;
    int AB_order_;
    int spad_order_;
    int spad_offset_;
    EntryFormat format_;
    bool upsample_ = false;

    void Init();
    bool SanityCheck();

public:
    RBM() = default;
    RBM(std::string name);
    ~RBM() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetBank(Sptr<Memory> bank_a, Sptr<Memory> bank_b);
    void SetOutputFM(Sptr<Tensor<int>> output);
};

#endif