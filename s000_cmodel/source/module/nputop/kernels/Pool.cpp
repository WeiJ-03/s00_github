#include "Pool.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
using std::vector;
using std::string;

Pool::Pool(std::string name){
    dump_suffix_ = name;
}

void Pool::Run(){
    Init();
    input_ -> DumpMatrix("before_pool");
    if ((pool_en_ & nl_en_) == 1) {
        DoPool();
//        Dump();
    }
    else *output_ = *input_;
    output_ -> DumpMatrix("after_pool");
}

void Pool::DoPool(){
    if (pool_mode_ == 0) DoMaxPool();
    else if (pool_mode_ == 1) DoAvgPool();
}

void Pool::DoMaxPool(){
    Tensor<int> padded_fm(ich_num_, 1, row_num_i_ + pad_t_ + pad_b_, col_num_i_ + pad_l_ + pad_r_);
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_i_ + pad_t_ + pad_b_; r++) {
            for (int c = 0; c < col_num_i_ + pad_l_ + pad_r_; c++) {
                padded_fm.Set(ch, 0, r, c, -32768);
            }
        }
    }
    //do padding
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_i_; r++) {
            for (int c = 0; c < col_num_i_; c++) {
                padded_fm.Set(ch, 0, r + pad_t_, c + pad_l_, input_->Get(ch, 0, r, c));
            }
        }
    }
    //do maxpool
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_o_; r++) {
            for (int c = 0; c < col_num_o_; c++) {
                int max = padded_fm.Get(ch, 0, r*pool_stride_, c*pool_stride_);
                for (int k_r = 0; k_r < pool_kernel_; k_r++) {
                    for (int k_c = 0; k_c < pool_kernel_; k_c++) {
                        int input = padded_fm.Get(ch, 0, r*pool_stride_+k_r, c*pool_stride_+k_c);
                        max = (max > input) ? max : input;
                    }
                }
                output_->Set(ch, 0, r, c, max);
            }
        }
    }
}

void Pool::DoAvgPool(){
    //AvgPool
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_o_; r++) {
            for (int c = 0; c < col_num_o_; c++) {
                if (ch == 97){
                    int a = 0;
                }
                //row num
                int row_st = r * pool_stride_ - pad_t_;       //start idx with padding
                int row_ed = row_st + pool_kernel_ - 1;       //end idx without padding
                row_st = (row_st < 0) ? 0 : row_st;
                row_ed = (row_ed > row_num_i_ - 1) ? (row_num_i_ - 1) : row_ed;
                int row_in = row_ed - row_st + 1;
                //col_num
                int col_st = c * pool_stride_ - pad_l_;
                int col_ed = col_st + pool_kernel_ - 1;
                col_st = (col_st < 0) ? 0 : col_st;
                col_ed = (col_ed > col_num_i_ - 1) ? (col_num_i_ - 1) : col_ed;
                int col_in = col_ed - col_st + 1;
                int weight = (int) (1 / (double) (row_in * col_in) * pow(2, 8));
                weight = (row_in * col_in == 1) ? weight - 1: weight;
                //do avg
                int64_t res = 0;
                for (int i = row_st; i <= row_ed; i++) {
                    for (int j = col_st; j <= col_ed; j++) {
                        res += input_->Get(ch, 0, i, j);
                    }
                }
                res = (res * (int64_t) weight) >> 8;
                output_->Set(ch, 0, r, c, (int) res);
            }
        }
    }
}

void Pool::Init(){
    row_num_i_ = input_->GetRow();
    col_num_i_ = input_->GetCol();
    ich_num_ = input_->GetCh();
    ich_group_num_ = ich_num_ / 16;
    pool_en_ = (*reg_file_)["NL_MODE"]["pool_en"];
    nl_en_ = (*reg_file_)["NL_MODE"]["nl_en"];

    pool_mode_ = (*reg_file_)["NL_MODE"]["pool_mode"];
    if (pool_mode_ > 1) std::cout << "Error: Doesn't have this pool mode!" << std::endl;

    pool_kernel_ = (*reg_file_)["NL_MODE"]["pool_kernel"] + 2;
    if (pool_kernel_ > 3) std::cout << "Error: Doesn't have this pool kernel size!" << std::endl;

    pool_stride_ = (*reg_file_)["NL_MODE"]["pool_stride"] + 1;
    if (pool_stride_ > pool_kernel_) std::cout << "Error: Pool stride is larger than pool kernel!" << std::endl;

    pad_l_ = (*reg_file_)["POOL_PAD"]["l"];
    pad_r_ = (*reg_file_)["POOL_PAD"]["r"];
    pad_b_ = (*reg_file_)["POOL_PAD"]["b"];
    pad_t_ = (*reg_file_)["POOL_PAD"]["t"];
    row_num_o_ = (row_num_i_ + pad_t_ + pad_b_ - pool_kernel_) / pool_stride_ + 1;
    col_num_o_ = (col_num_i_ + pad_r_ + pad_l_ - pool_kernel_) / pool_stride_ + 1;
    CreateOutFM();
}

void Pool::CreateOutFM(){
    if ((pool_en_ & nl_en_) == 0) output_->Resize(ich_num_, 1, row_num_i_, col_num_i_);
    else output_->Resize(ich_num_, 1, row_num_o_, col_num_o_);
}

void Pool::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void Pool::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void Pool::SetWeight(Sptr<vector<string>> wt){
   weights_ = wt;
}

bool Pool::SanityCheck() {
    return 0;
}

void Pool::Dump() {
    output_->DumpSeq("single_layer_POOL_output");
//    output_->DumpMatrix("single_layer_POOL_output");
}

void Pool::CleanUp(){}