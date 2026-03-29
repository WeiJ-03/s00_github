#include "Psum.h"
#include <memory>
#include <string>
#include <cmath>
#include <bitset>
#include <iostream>
#include "Memory.h"


/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/14 |  1.0      | Rui           | s000 psum                        *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Psum::Run(){
    Init();
    input_->DumpMatrix("Psum_in");
    if (ch_st_ == 1) PreviousPsumAcc(); 
    DoPsumAcc();
    if (full_ch_ == 0) { // 0 1 1 1
        pmem_->Store(*output_, 0, 0, 0, EntryFormat::COL1_CH32); // 
    } else {
        DoFineShift();
        DoBias();
        // DoClamp();
    }
    Dump();
}

void Psum::PreviousPsumAcc(){
    Tensor<int> previous_psum(ich_num_, 1, row_num_, col_num_);
    previous_psum = pmem_->Load(0, 0, ich_num_, 0, row_num_, col_num_, EntryFormat::COL1_CH32);
    for (int ich = 0; ich < ich_num_; ich++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = 0;
                res = (int16_t) input_->Get(ich, 0, r, c) + (int16_t) previous_psum.Get(ich, 0, r, c);
                Tensor<int>::ClampToLSB(res, OFM_BIT);
                input_->Set(ich, 0, r, c, (int) res);
            }
        }
    }
}

void Psum::DoPsumAcc(){
    for (int ich = 0; ich < ich_num_; ich++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = 0;
                int a = 0;
                for (int psum = 0; psum < psum_num_; psum++) {
                    a = input_->Get(ich, psum, r, c);
                    res += a;
                    Tensor<int>::ClampToLSB(res, 16);
                }
                output_->Set(ich, 0, r, c, (int) res);
            }
        }
    }

}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/14 |  1.0      | Rui           | s000 psum                        *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Psum::DoFineShift(){
    Tensor<int> fine_shift(ich_num_, 1, 1, 1);
    for (int i = 0; i < ich_group_num_; i++) {
        std::vector<std::string> fs_string(64);
        for (int j = 0; j < 64; j++) {
            fs_string.at(j) = weights_->at(i*4+3).substr(1024 - 192 - 4 - j*4,4);    //fine_shift start from 80 bit and size is [15:0][3:0]
            fine_shift.Set(i * 64 + j, 0, 0, 0, Weight::bin_to_uint(fs_string.at(j)));
        }
    }
    for (int ich = 0; ich < ich_num_; ich++) {
        int shift_value = fine_shift.Get(ich, 0, 0, 0);
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = output_->Get(ich, 0, r, c);
                if (shift_value != 0) res >>= shift_value;
                Tensor<int>::ClampToLSB(res, 8);
                output_->Set(ich, 0, r, c, res);
            }
        }
    }
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/14 |  1.0      | Rui           | s000 psum                        *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Psum::DoBias(){
    Tensor<int> bias(ich_num_, 1, 1, 1);
    for (int i = 0; i < ich_group_num_; i++) {
        std::vector<std::string> bias_string(64);
        for (int j = 0; j < 64; j++) {
            bias_string.at(j) = weights_->at(i*4+3).substr(1024 - 192 - 256 - 8 - j*8,8);    //bias start from 112 bit and size is [15:0][15:0]
            bias.Set( i * 64 + j , 0, 0, 0, Weight::bin_to_uint(bias_string.at(j)));
        }
    }
    for (int ich = 0; ich < ich_num_; ich++) {
        int bias_value = bias.Get(ich, 0, 0, 0);
        if (bias_value>127) bias_value = -(256-bias_value);
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                int res = 0;
                res = output_->Get(ich, 0, r, c) + bias_value;
                Tensor<int>::ClampToLSB(res, 8);
                output_->Set(ich, 0, r, c, res);
            }
        }
    }
}

void Psum::DoClamp(){
    for (int ch = 0; ch < ich_num_; ch++) {
        for (int r = 0; r < row_num_; r++) {
            for (int c = 0; c < col_num_; c++) {
                uint32_t res = 0;
                res = (uint32_t) output_->Get(ch, 0, r, c) & 0x00FF;
                res = (res <= 127) ? res : -(256 - (int) res);
                output_->Set(ch, 0, r, c, (int) res);
            }
        }
    }
}

void Psum::Init(){
    row_num_ = input_->GetRow();
    col_num_ = input_->GetCol();
    ich_num_ = input_->GetCh();
    psum_num_ = input_->GetPsumDim();
    full_ch_ = (*reg_file_)["reg_CONV_MODE"]["full_ch"];
    ch_st_ = (*reg_file_)["reg_CONV_MODE"]["ch_st"];
    conv_mode_ = (*reg_file_)["reg_CONV_MODE"]["mode"];
    trim_ = (*reg_file_)["reg_CONV_MODE"]["trim"];
    ich_group_num_ = ich_num_ / OCH_PER_GROUP;
    int pmem_depth_limit = (pmem_ != nullptr) ? pmem_->entry_depth_ : 2048;

    if(trim_ == 1){
        int row_frame = ((row_num_-1) / 4) + 1;
        int ch_group;
        if(ich_num_ > 64){
            ch_group = 4;
        } else {
            // ch_group = ich_num_ / 16;
            ch_group = 4; // even if ich_num_ is less than 64, we still set ch_group to 4 for trim=1 mode, because the psum data in PMEM is stored in a format of 4 groups of 16 channels.
        }
        int col_max = (((row_frame * ch_group) + 1)/2) * col_num_;
        if(col_max > pmem_depth_limit){
            std::cout << "Error: pmem overflow!" << std::endl;
            std::cout << "PMEM check detail (trim=1):"
                      << " row=" << row_num_
                      << ", col=" << col_num_
                      << ", ich=" << ich_num_
                      << ", row_frame=" << row_frame
                      << ", ch_group=" << ch_group
                      << ", col_max=" << col_max
                      << ", pmem_limit=" << pmem_depth_limit
                      << ", conv_mode=" << conv_mode_
                      << ", ch_st=" << ch_st_
                      << std::endl;
            std::exit(1);  
        }     
    } else {
        int row_frame = ((row_num_-1) / 8) + 1;
        int ch_group;
        if(ich_num_ > 64){
            ch_group = 4;
        } else {
            // ch_group = ich_num_ / 16;
            ch_group = 4; // even if ich_num_ is less than 64, we still set ch_group to 4 for trim=0 mode, because the psum data in PMEM is stored in a format of 4 groups of 16 channels.
        }
        int col_max = row_frame * ch_group * col_num_;
        if(col_max > pmem_depth_limit){
            std::cout << "Error: pmem overflow!" << std::endl;
            std::cout << "PMEM check detail (trim=0):"
                      << " row=" << row_num_
                      << ", col=" << col_num_
                      << ", ich=" << ich_num_
                      << ", row_frame=" << row_frame
                      << ", ch_group=" << ch_group
                      << ", col_max=" << col_max
                      << ", pmem_limit=" << pmem_depth_limit
                      << ", conv_mode=" << conv_mode_
                      << ", ch_st=" << ch_st_
                      << std::endl;
            std::exit(1);  
        }          
    }
    


    CreateOutFM();
}

void Psum::CreateOutFM(){
    output_->Resize(ich_num_, 1, row_num_, col_num_);
}

void Psum::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void Psum::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void Psum::SetPMEM(Sptr<Memory> bank){
    pmem_ = bank;
}

void Psum::SetWeight(Sptr<vector<string>> wt){
    weights_ = wt;
}

bool Psum::SanityCheck() {
    return 0;
}

void Psum::Dump(){
    output_->DumpMatrix("single_layer_psum_output");
}

void Psum::CleanUp(){}