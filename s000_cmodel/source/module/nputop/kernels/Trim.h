#ifndef NPU_TRIM_H
#define NPU_TRIM_H

#include "Module.h"
#include "Tensor.h"
#include <string>

class Trim : public Module {
private:
    Sptr<Tensor<int>> input_;
    Sptr<Tensor<int>> output_;
    std::string dump_suffix_;
    int row_num_i_;
    int row_num_o_;
    int col_num_i_;
    int col_num_o_;
    int ch_num_   ;
    int psum_num_;
    int stride_;

    void Init();
    bool SanityCheck();
    void CreateOutFM();
    void DoTrim();

public:
    Trim() = default;
    // Trim(std::string name);
    ~Trim() = default;
    void Dump();
    void Run();
    void CleanUp();
    void SetInputFM(Sptr<Tensor<int>> input);
    void SetOutputFM(Sptr<Tensor<int>> output);

};

#endif