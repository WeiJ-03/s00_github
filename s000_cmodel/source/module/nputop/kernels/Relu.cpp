#include "Relu.h"
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
using std::vector;
using std::string;

Relu::Relu(std::string name){
    dump_suffix_ = name;
}

void Relu::Run(){
    Init();
    if (relu_en_ == 1) {
        DoRelu();
//        Dump();
    } else {
        *output_ = *input_;
    }
    output_ -> DumpMatrix("after_relu");
}

void Relu::DoRelu(){
    if (relu_mode_ == 0) DoNormalRelu();
    else if (relu_mode_ == 1) DoPrelu();
     else if (relu_mode_ == 2) DoRelu6();
    else std::cerr << "Doesn't have this relu mode!" << std::endl;
}

void Relu::DoNormalRelu(){
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                if (input_->Get(ch, 0, r, c) < 0) output_->Set(ch, 0, r, c, 0);
                else output_->Set(ch, 0, r, c, input_->Get(ch, 0, r, c));
            }
        }
    }
}

void Relu::DoPrelu(){
    Tensor<int> prelu_alpha(ich_num_, 1, 1, 1);
    //weight assignment
    for (int i = 0; i < ich_group_num_; i++) {
        std::vector<std::string> alpha(64);
        for (int j = 0; j < 64; j++) {
            alpha.at(j) = weights_->at(i*4+2).substr(1024 - 8 - j*8, 8);
            prelu_alpha.Set(i*64+j, 0, 0, 0, Weight::bin_to_int(alpha.at(j)));
        }
    }
    //do prelu
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int64_t res = 0;
                int input = input_->Get(ch, 0, r, c);
                if (input < 0) {
                    res = (int64_t) input * (int64_t) prelu_alpha.Get(ch, 0, 0, 0);
                    res >>= 9;
                    output_->Set(ch, 0, r, c, res);
                } else {
                    output_->Set(ch, 0, r, c, input);
                }
            }
        }
    }
}

void Relu::DoRelu6(){
    Tensor<int> relu6_th(ich_num_, 1, 1, 1);
    //weight assignment
    for (int i = 0; i < ich_group_num_; i++) {
        std::vector<std::string> relu6(64);
        for (int j = 0; j < 64; j++) {
            relu6.at(j) = weights_->at(i*4+2).substr(1024 - 512 - 8 - j*8, 8); 
            relu6_th.Set(i*64+j, 0, 0, 0, Weight::bin_to_uint(relu6.at(j)));
        }
    }
    //do relu6
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = 0;
                int input = input_->Get(ch, 0, r, c);
                int th = relu6_th.Get(ch, 0, 0, 0);
                if (input < 0) res = 0;
                else if (input > th) res = th;
                else res = input;
                output_->Set(ch, 0, r, c, res);
            }
        }
    }
}

void Relu::Init(){
    row_num_ = input_->GetRow();
    col_num_ = input_->GetCol();
    ich_num_ = input_->GetCh();
    ich_group_num_ = ceil((double) ich_num_ / 64);
    relu_mode_ = (*reg_file_)["NL_MODE"]["relu_mode"];   //0:do nothing, 1:normal, 2:prelu, 3:relu6
    relu_en_ = (*reg_file_)["NL_MODE"]["relu_en"]; 
    CreateOutFM();
    input_ -> DumpMatrix("before_relu");
}

void Relu::CreateOutFM(){
    output_->Resize(ich_num_, 1, row_num_, col_num_);
}

void Relu::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void Relu::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void Relu::SetWeight(Sptr<vector<string>> wt){
   weights_ = wt;
}


bool Relu::SanityCheck() {
    return 0;
}

void Relu::Dump() {
    output_->DumpSeq("single_layer_ReLU_output");
//    output_->DumpMatrix("yolo_cut_ReLU_output");
}

void Relu::CleanUp(){}