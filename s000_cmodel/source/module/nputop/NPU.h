#ifndef NPU_NPU_H
#define NPU_NPU_H
#include <vector>
#include <string>
#include "RegisterFile.h"
#include "Define.h"
#include "RBM.h"
#include "WBM.h"
#include "Mac.h"
#include "Trim.h"
#include "Psum.h"
// #include "LUT.h"
#include "Lut.h"
#include "BN.h"
#include "Relu.h"
#include "Pool.h"
#include "CommandList.h"
#include "Weight.h"
#include "Memory.h"

using std::string;

/*****************************************************************************                                                                          *
*  @function    NPU::Init()                                           *
*----------------------------------------------------------------------------*
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/06/04 | 1.0       | Rui           | Plus psum_mem_                   * 
*----------------------------------------------------------------------------*
*****************************************************************************/

class NPU {
private:
    static const int ICH_PER_GROUP ;
    static const int ICH_PER_RGBA_GROUP;
    static const int BANK_LINE;
    static const int BANK_COL;

    string name_;
    Sptr<RegisterFile> reg_file_;
    Sptr<Weight> weight_;
    // Sptr<Memory> pmem_;
    CommandList cmd_;
    // read buffer manager
    RBM rbm_;
    // write buffer manager
    WBM wbm_;
    Mac mac_;
    Trim trim_;
    Psum psum_;
    LUT lut_;
    BN bn_;
    Relu relu_;
    Pool pool_;

    Sptr<Tensor<int>> rbm_out_;
    Sptr<Tensor<int>> mac_out_;
    Sptr<Tensor<int>> trim_out_;
    Sptr<Tensor<int>> psum_out_;
    Sptr<Tensor<int>> lut_out_;
    Sptr<Tensor<int>> bn_out_;
    Sptr<Tensor<int>> relu_out_;
    Sptr<Tensor<int>> pool_out_;

    uint16_t reg_target_idx_ = 0;
    int reg_target_value_ = 0;
    vector<int> register_value_ = vector<int> (16, 0);    //reg_file has 16 registers
    int loop_count_num_ = 0;
    int addr_offset_ = 0;
    size_t program_cnt_ = 0;
    int intr_source_ = 0;
    void ModuleReorg();

public:
    Sptr<Memory> bank_a_, bank_b_, lut_mem_, psum_mem_;

public:
    NPU() = default;
    NPU(string name);
    virtual ~NPU() = default;
    void WriteCommand(const vector<uint8_t> &);
    void WriteWeight(const vector<uint8_t> &);
    void WriteData(const Tensor<int>& input, int offset_x, int offset_y, int channel_st, EntryFormat format);
    void Run();
    void Dump();
    void Init();
    void ResetProgramCnt();
    int GetInterrupt() {return intr_source_;};
};



#endif //NPU_NPU_H