#ifndef NPU_RELU_H
#define NPU_RELU_H

#include "Module.h"
#include "Tensor.h"
#include "Weight.h"
#include <string>
#include <vector>
using std::vector;
using std::string;

class Relu : public Module {
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;
    Sptr<vector<string>> weights_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoRelu();
    void DoNormalRelu();
    void DoPrelu();
    void DoRelu6();

    int row_num_;
    int col_num_;
    int ich_num_;
    int ich_group_num_;
    int relu_mode_;
    int relu_en_;

public:
    Relu() = default;
    Relu(std::string name);
    ~Relu() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);
    void SetWeight(Sptr<vector<string>> wt);
};

#endif