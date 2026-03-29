#ifndef NPU_POOL_H
#define NPU_POOL_H

#include "Module.h"
#include "Tensor.h"
#include "Weight.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

class Pool : public Module {
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;
    Sptr<vector<string>> weights_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoPool();
    void DoMaxPool();
    void DoAvgPool();

    int row_num_i_;
    int col_num_i_;
    int row_num_o_;
    int col_num_o_;
    int ich_num_;
    int ich_group_num_;
    int pool_en_;
    int nl_en_;
    int pool_mode_;
    int pool_kernel_;
    int pool_stride_;
    int pad_l_;
    int pad_r_;
    int pad_b_;
    int pad_t_;

public:
    Pool() = default;
    Pool(std::string name);
    ~Pool() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetWeight(Sptr<vector<string>> wt);
};

#endif