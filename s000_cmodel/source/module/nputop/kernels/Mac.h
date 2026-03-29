#ifndef NPU_MAC_H
#define NPU_MAC_H

#include "Module.h"
#include "Define.h"
#include "Tensor.h"
#include "Weight.h"
#include "spdlog/spdlog.h"
#include "Memory.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

class Mac : public Module{
private:
    static const int CS_WIDTH               = 8388608; 


    static const int ICH_PER_GROUP_fullCH              = 128;
    static const int OCH_PER_GROUP_fullCH              = 64;

    static const int ICH_PER_GROUP_quarterCH           = 32;
    static const int OCH_PER_GROUP_quarterCH           = 64;

    static const int ICH_PER_GROUP_1x1                 = 128;
    static const int OCH_PER_GROUP_1x1                 = 64;

    static const int ICH_PER_GROUP_DW                  = 64;
    static const int OCH_PER_GROUP_DW                  = 64;

    static const int ICH_PER_GROUP_BYPASS_ADD          = 64;
    static const int OCH_PER_GROUP_BYPASS_ADD          = 64;



    static const int WEIGHT_BLOCK_CYCLE_3x3_fullCH_            = 64;
    static const int WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_         = 48;
    static const int WEIGHT_BLOCK_CYCLE_1x1_                   = 64;
    static const int WEIGHT_BLOCK_CYCLE_DW_                    = 16;
    static const int PARAMETER_BLOCK_CYCLE_                    = 4;
    static const int WEIGHT_BLOCK_CYCLE_BYPASS_ADD_            = 0;    

    static const int IN_FIFO_WIDTH                     = 128; 

    static const int CONV_MODE_BYPASS           = 0;            
    static const int CONV_MODE_3x3_fullCH       = 1;           
    static const int CONV_MODE_3x3_QuarterCH    = 2;           
    static const int CONV_MODE_DW               = 3;           
    static const int CONV_MODE_1x1              = 4;          
    static const int CONV_MODE_ADD              = 5;       

private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;
    Sptr<Weight> origin_weight_;
    Sptr<Weight> weight_mac_;
    Sptr<Weight> weight_param_;    //weight tensor demension is (1, 1, 1, 192)
    Sptr<Memory> LUT_memory_;
    int weight_idx_ = 0;
    int conv_mode_;
    int ich_num_, och_num_;
    int ich_group_num_, och_group_num_;
    int row_num_i_, col_num_i_;
    int row_num_o_, col_num_o_;
    int kernel_width_;
    int kernel_height_;

    int stride_;
    int weight_len_ = 0;    //mac weight depth

    int pad_l_;
    int pad_r_;
    int pad_b_;
    int pad_t_;

    Sptr<spdlog::logger> logger_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoBypass();
    void DoConvolution_3x3_fullCH(const Tensor<int> &);
    void DoConvolution_3x3_QuarterCH(const Tensor<int> &);
    void DoConvolution1x1(const Tensor<int> &);
    void DoConvolution1x1_dump(const Tensor<int> &);
    void DoDepthWise(const Tensor <int> &);
    void DoCoarseShift(const Tensor<int> &);
    void DoAdd(const Tensor<int> &);
    void Evaluation() const;
    void AssignWeight();
    void ParseWeight(Tensor<int> & wt, Tensor<int> & cs, Tensor<int> & ds);   //parse weight_mac_ into tensor
    void PrintReg();
    int  appro_mul(int a, int b);
    std::string GetDumpName(const std::string& base_name);  // 生成带计数的 dump 文件名

public:
    Sptr<vector<string>> param_vector_;

private:
    static int mac_call_count_;  // 统计 Mac 调用次数

public:
    Mac() = default;
    Mac(std::string name);
    ~Mac() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetWeight(Sptr<Weight> wt);
    void SetLUTMem(Sptr<Memory> lut_mem);
    static int GetMacCallCount() { return mac_call_count_; }  // 获取调用次数
    static void ResetMacCallCount() { mac_call_count_ = 0; }  // 重置计数器
    static void PrintMacCallCount();  // 打印调用次数
};





#endif