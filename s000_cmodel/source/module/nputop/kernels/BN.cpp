#include "BN.h"
#include <string>
#include <vector>
#include <cmath>
using std::vector;
using std::string;

BN::BN(std::string name){
    dump_suffix_ = name;
}

void BN::Run(){
    Init();
    input_ -> DumpMatrix("before_BN");
    if (((bn_en_) == 1) && (nl_en_ == 1)) {
        DoBN();
//        Dump();
    }
    else *output_ = *input_;
    output_ -> DumpMatrix("after_BN");
}

void BN::DoBN(){
    Tensor<int> bn_beta(ich_num_, 1, 1, 1);
    Tensor<int> bn_gamma(ich_num_, 1, 1, 1);
    Tensor<int> bn_shift(ich_num_, 1, 1, 1);
    //weight assignment
    for (int i = 0; i < ich_group_num_; i++) {
        std::vector<std::string> beta(64),gamma(64),shift(64);
        for (int j = 0; j < 64; j++) {
            beta.at(j) = weights_->at(i*4+1).substr(1024 - 256 - 8 - j*8, 8);   //bn_beta is 8bits
            bn_beta.Set(i*64+j, 0, 0, 0, Weight::bin_to_int(beta.at(j)));
            gamma.at(j) = weights_->at(i*4).substr(1024- 8 - j*8, 8);  //bn_gamma is 16bits
            bn_gamma.Set(i*64+j, 0, 0, 0, Weight::bin_to_int(gamma.at(j)));
            shift.at(j) = weights_->at(i*4+1).substr(1024 - 256 - 512 - 4 - j*4, 4);  //bn_shift is 2bits
            bn_shift.Set(i*64+j, 0, 0, 0, Weight::bin_to_uint(shift.at(j)));
        }

    }
    //do bn
    for (int ch = 0; ch < ich_num_; ch++) {
        int shift_value = bn_shift.Get(ch, 0, 0, 0);
        int beta_value = bn_beta.Get(ch, 0, 0, 0);
        int gamma_value = bn_gamma.Get(ch, 0, 0, 0);
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = 0;
                res = (int) input_->Get(ch, 0, r, c) * (int) beta_value;
                switch (shift_value) {
                    case 0 : res *= pow(2, 12); break;
                    case 1 : res *= pow(2, 10); break;
                    case 2 : res *= pow(2, 8); break;
                    case 3 : res *= pow(2, 6); break;
                    case 4 : res *= pow(2, 4); break;
                    case 5 : res *= pow(2, 3); break;
                    case 6 : res *= pow(2, 2); break;
                    case 7 : res *= pow(2, 1); break;
                    case 9 : res >>= 1; break;
                    case 10: res >>= 2; break;
                    case 11: res >>= 3; break;
                    case 12: res >>= 4; break;
                    case 13: res >>= 6; break;
                    case 14: res >>= 8; break;
                    case 15: res >>= 10; break;
                    default: break;
                }
                res += ((int) gamma_value) * pow(2, 8);
                Tensor<int>::ClampToLSB(res, 16);
                output_->Set(ch, 0, r, c, res >> 8);
            }
        }
    }
}

void BN::Init(){
    row_num_ = input_->GetRow();
    col_num_ = input_->GetCol();
    ich_num_ = input_->GetCh();
    ich_group_num_ = ceil((double) ich_num_ / 64);
    bn_en_ = (*reg_file_)["NL_MODE"]["bn_en"];
    nl_en_ = (*reg_file_)["NL_MODE"]["nl_en"];
    CreateOutFM();
}

void BN::CreateOutFM(){
    output_->Resize(ich_num_, 1, row_num_, col_num_);
}

void BN::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void BN::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void BN::SetWeight(Sptr<vector<string>> wt){
    weights_ = wt;
}

bool BN::SanityCheck() {
    return 0;
}

void BN::CleanUp(){}

void BN::Dump() {
    output_->DumpSeq("single_layer_BN_output");
//    output_->DumpMatrix("single_layer_BN_output");
}