#include "Trim.h"
#include <string>

// Trim::Trim(std::string name){
//     dump_suffix_ = name;
// }

void Trim::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void Trim::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void Trim::Run(){
    Init();
    DoTrim();
    output_->DumpMatrix("Trim_out");
}

void Trim::DoTrim(){
    for (int ch = 0; ch < ch_num_; ch++) {
        for (int psum = 0; psum < psum_num_; psum++) {
            for (int r = 0; r < row_num_o_; r++) {
                for (int c = 0; c < col_num_o_; c++) {
                    output_->Set(ch, psum, r, c, input_->Get(ch, psum, r*stride_, c*stride_));
                }
            }
        }
    }
}

void Trim::Init(){
    stride_ = (*reg_file_)["reg_CONV_MODE"]["trim"] + 1;
    row_num_i_ = input_->GetRow();
    col_num_i_ = input_->GetCol();
    row_num_o_ = (row_num_i_ - 1) / stride_ + 1;
    col_num_o_ = (col_num_i_ - 1) / stride_ + 1;
    ch_num_ = input_->GetCh();
    psum_num_ = input_->GetPsumDim();
    CreateOutFM();
}

void Trim::CreateOutFM(){
    output_->Resize(ch_num_, psum_num_, row_num_o_, col_num_o_);
}

bool Trim::SanityCheck() {
    return 0;
}

void Trim::Dump(){}

void Trim::CleanUp(){}