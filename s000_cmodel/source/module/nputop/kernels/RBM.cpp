#include "RBM.h"
#include <string>
#include <cmath>

RBM::RBM(std::string name){
    dump_suffix_ = name;
}

void RBM::Run(){
    Init();
    bank_a_ -> DumpMemory("RBM_memroy");
    if (conv_mode_ == CONV_MODE_ADD) {
        Tensor<int> fm_1 = bank_a_->RBMLoad(offset_x1_, offset_y1_, ich_num_, 0, row_num_, col_num_, format_);
        Tensor<int> fm_2 = bank_a_->RBMLoad(offset_x2_, offset_y2_, ich_num_, 0, row_num_, col_num_, format_);
        for (int ch = 0; ch < ich_num_; ch++) {
            for (int r = 0; r < row_num_; r++) {
                for (int c = 0; c < col_num_; c++) {
                    fm_->Set(ch, 0, r, c, fm_1.Get(ch, 0, r, c));
                    fm_->Set(ch, 1, r, c, fm_2.Get(ch, 0, r, c));
                }
            }
        }
    } else {
        *fm_ = bank_a_->RBMLoad(offset_x1_, offset_y1_, ich_num_, 0, row_num_, col_num_, format_);      //fm_ size is (ich, 0, r, c)
    }
    fm_->DumpMatrix("RBMinput");

    int ich_num_tmp = 0;    //Get channel num multiple of 32 or 16
    if (conv_mode_ == CONV_MODE_BYPASS || conv_mode_ == CONV_MODE_DW || conv_mode_ == CONV_MODE_ADD) 
        ich_num_tmp = ((ich_num_ - 1) / CHANNEL_PER_GROUP + 1) * CHANNEL_PER_GROUP;
    else if (conv_mode_ == CONV_MODE_1x1 || conv_mode_ == CONV_MODE_3x3_fullCH) 
        ich_num_tmp = ((ich_num_ - 1) / ICH_PER_GROUP + 1) * ICH_PER_GROUP;
    else
        ich_num_tmp = ((ich_num_ - 1) / QUARTER_PER_GROUP + 1) * QUARTER_PER_GROUP;


    Tensor<int> cropped_fm(psum_dim_, ich_num_tmp, crop_row_out_, crop_col_out_);

    for (int i = 0; i < ich_num_; i++) {
        for (int j = 0; j < crop_row_out_; j++) {
            for (int k = 0; k < crop_col_out_; k++) {
                for (int psum = 0; psum < psum_dim_; psum++) {
                    cropped_fm.Set(psum, i, j, k, fm_->Get(i, psum, j+crop_row_st_, k+crop_col_st_));
                }
            }
        }
    }
    int row = pad_t_+pad_b_+crop_row_out_;
    int col = pad_l_+pad_r_+crop_col_out_;
    Tensor<int> padded_fm(psum_dim_, ich_num_tmp, row, col);
    for (int i = 0; i < ich_num_; i++) {
        for (int j = 0; j < crop_row_out_; j++) {
            for (int k = 0; k < crop_col_out_; k++) {
                for (int psum = 0; psum < psum_dim_; psum++) {
                    padded_fm.Set(psum, i, j+pad_t_, k+pad_l_, cropped_fm.Get(psum, i, j, k));
                }
            }
        }
    }
    if (conv_mode_ == 0 && upsample_) {   //do upsample
        output_->Resize(psum_dim_, ich_num_tmp, 2*row, 2*col);
        for (int psum = 0; psum < psum_dim_; ++psum) {
            for (int ch = 0; ch < ich_num_tmp; ++ch) {
                for (int r = 0; r < row; ++r) {
                    for (int c = 0; c < col; ++c) {
                        output_->Set(psum, ch, 2*r, 2*c, padded_fm.Get(psum, ch, r, c));
                        output_->Set(psum, ch, 2*r+1, 2*c, padded_fm.Get(psum, ch, r, c));
                        output_->Set(psum, ch, 2*r, 2*c+1, padded_fm.Get(psum, ch, r, c));
                        output_->Set(psum, ch, 2*r+1, 2*c+1, padded_fm.Get(psum, ch, r, c));
                    }
                }
            }
        }
    } else {
        (*output_) = padded_fm;
    }
    output_->DumpMatrix("RBMoutput");
}

void RBM::SetBank(Sptr<Memory> bank_a, Sptr<Memory> bank_b){
    bank_a_ = bank_a;
    bank_b_ = bank_b;
}

void RBM::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/12 |  1.0      | Rui           | Delete spad order & spad offset  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void RBM::Init(){
    conv_mode_ = (*reg_file_)["reg_CONV_MODE"]["mode"];
    ich_num_ = (*reg_file_)["reg_FM_ICH"]["ich"];
    row_num_ = (*reg_file_)["reg_FM_ROW"]["row"];
    col_num_ = (*reg_file_)["reg_FM_COL"]["col"];

    pad_l_ = (*reg_file_)["reg_PAD2"]["l"];
    pad_r_ = (*reg_file_)["reg_PAD2"]["r"];
    pad_t_ = (*reg_file_)["reg_PAD1"]["t"];
    pad_b_ = (*reg_file_)["reg_PAD1"]["b"];

    
    crop_row_st_    = (*reg_file_)["reg_CROP"]["row_st"];
    crop_col_st_    = (*reg_file_)["reg_CROP"]["col_st"];
    crop_row_out_   = (*reg_file_)["reg_CROP_ROW"]["row_out"];
    crop_col_out_   = (*reg_file_)["reg_CROP_COL"]["col_out"];
    channel_st_     = (*reg_file_)["reg_FM_OCH_ST"]["och_st"];
    channel_ed_     = (*reg_file_)["reg_FM_OCH_ED"]["och_ed"];
    offset_x1_      = (*reg_file_)["reg_MEM_IN1"]["offset_x"];
    offset_y1_      = (*reg_file_)["reg_MEM_IN1"]["offset_y"];
    offset_x2_      = (*reg_file_)["reg_MEM_IN2"]["offset_x"];
    offset_y2_      = (*reg_file_)["reg_MEM_IN2"]["offset_y"];
    format_         = EntryFormat::COL1_CH32;
    upsample_       = (*reg_file_)["reg_CONV_MODE"]["upsample"];
    AB_order_       = (*reg_file_)["reg_CONV_MODE"]["AB_order"];

    //this part may be useful for big kernel DW
    if (conv_mode_ == CONV_MODE_DW) {
        offset_x1_ = (row_num_ * (channel_st_ / ICH_PER_GROUP) + offset_y1_) / BANK_LINE * col_num_ + offset_x1_;
        offset_y1_ = (row_num_ * (channel_st_ / ICH_PER_GROUP) + offset_y1_) % BANK_LINE;
    }
    psum_dim_ = (conv_mode_ == CONV_MODE_ADD) ? 2 : 1;
    fm_ = std::make_shared<Tensor<int>>(ich_num_, psum_dim_, row_num_, col_num_);
}

void RBM::Dump(){}

void RBM::CleanUp(){}

bool RBM::SanityCheck(){
    return 0;
}

