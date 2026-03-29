#include "Lut.h"


/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/15 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void LUT::DoLUTCh(){
    Tensor<int> lut;
    lut = mem_->Load(0, 0, 1, 0, 1, 256, EntryFormat::COL1_CH32);
    for (int ch = 0; ch < ch_num_; ++ch) {
        for (int r = 0; r < row_num_; ++r) {
            for (int c = 0; c < col_num_; ++c) {
                uint8_t din = (uint8_t) input_->Get(ch, 0, r, c);
                int depth = din % 256;
                output_->Set(ch, 0, r, c, lut.Get(0, 0, 0, depth));
            }
        }
    }
}

void LUT::Run() {
    Init();
    switch (LUT_en_) {
        case 0: *output_ = *input_; break;
        case 1: DoLUTCh(); break;
        default: std::cerr<< "Error: LUT_en_ is bool type, but got 2!"<< std::endl;
    }
}

void LUT::Init() {
    LUT_en_ = (*reg_file_)["NL_MODE"]["act_LUT_en"];
    row_num_ = input_->GetRow();
    col_num_ = input_->GetCol();
    ch_num_ = input_->GetCh();
    CreateOutFM();
}

void LUT::SetInputFM(Sptr<Tensor<int>> input) {
    input_ = input;
}

void LUT::SetOutputFM(Sptr<Tensor<int>> output) {
    output_ = output;
}

void LUT::SetLUTMEM(Sptr<Memory> lut_mem) {
    mem_ = lut_mem;
}


void LUT::CreateOutFM() {
    output_->Resize(ch_num_, 1, row_num_, col_num_);
}

void LUT::Dump() {

}

void LUT::CleanUp() {

}

bool LUT::SanityCheck() {

}