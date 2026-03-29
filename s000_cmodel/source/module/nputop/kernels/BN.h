#ifndef NPU_BN_H
#define NPU_BN_H

#include "Module.h"
#include "Weight.h"
#include "Tensor.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

class BN : public Module {
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;
    Sptr<vector<string>> weights_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoBN();

    int row_num_;
    int col_num_;
    int ich_num_;
    int ich_group_num_;
    int bn_en_;
    int nl_en_;

public:
    BN() = default;
    BN(std::string name);
    ~BN() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetWeight(Sptr<vector<string>> wt);
};

#endif