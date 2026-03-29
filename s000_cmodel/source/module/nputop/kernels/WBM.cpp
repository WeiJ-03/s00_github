#include "WBM.h"
#include <string>

WBM::WBM(std::string name){
    dump_suffix_ = name;
}

void WBM::Run(){
    Init();
    bank_a_->Store(*input_, offset_x_, offset_y_, channel_st_ % ICH_PER_GROUP, EntryFormat::COL1_CH32);
    // input_->weight_->DumpMatrix(string("WBM_input"));
    // input_->DumpMatrix(string("WBM_input"));
    bank_a_->DumpMemory("WBM_Memory");
    Dump();
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | Delete spad order & spad offset  *
*----------------------------------------------------------------------------*
*****************************************************************************/

void WBM::Init(){
    channel_st_ = (*reg_file_)["reg_FM_OCH_ST"]["och_st"];
    channel_ed_ = (*reg_file_)["reg_FM_OCH_ED"]["och_ed"];
    offset_x_ = (*reg_file_)["reg_MEM_OUT"]["offset_x"];
    offset_y_ = (*reg_file_)["reg_MEM_OUT"]["offset_y"];
    AB_order_ = (*reg_file_)["reg_CONV_MODE"]["AB_order"];

    int row = input_->GetRow();
    int col = input_->GetCol();
    int ch = input_->GetCh();
    offset_x_ = (row * (channel_st_ / ICH_PER_GROUP) + offset_y_) / BANK_LINE * col + offset_x_;
    offset_y_ = (row * (channel_st_ / ICH_PER_GROUP) + offset_y_) % BANK_LINE;


}

void WBM::SetBank(Sptr<Memory> bank_a, Sptr<Memory> bank_b){
    bank_a_ = bank_a;
    bank_b_ = bank_b;
}

void WBM::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

bool WBM::SanityCheck(){
    return 0;
}
void WBM::Dump(){
    //input_->DumpSeq("single_layer_final_output");
    input_->DumpMatrix("WBM_output");
}

void WBM::CleanUp(){}
