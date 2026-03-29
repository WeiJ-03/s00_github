#include "Mac.h"
#include <string>
#include <vector>
#include <cmath>
#include <bitset>
#include <cassert>
#include <map>
#include <numeric>
#include <cassert>
#include <iostream>

using std::cout;
using std::endl;
using std::string;

// 初始化静态计数器
int Mac::mac_call_count_ = 0;

Mac::Mac(std::string name){
    dump_suffix_ = name;
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/13 |  1.0      | Rui           | s000 mac                         *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Mac::Run(){
    // 增加 Mac 调用计数
    mac_call_count_++;
    std::cout << "Mac Call Count: " << mac_call_count_ << endl;
    Tensor<int> wt;
    Tensor<int> cs;   //coarse_shift
    Tensor<int> ds;   //data_shift
    Init();

    const bool enable_dump = ShouldDumpCurrentNode();
    
    if (enable_dump) {
        origin_weight_->weight_->DumpMatrix(GetDumpName("origin_weight"));
    }
    AssignWeight();
    if (enable_dump) {
        weight_mac_->weight_->DumpMatrix(GetDumpName("origin_weight_mac"));
        weight_param_->weight_->DumpMatrix(GetDumpName("origin_weight_para"));
    }
    ParseWeight(wt, cs, ds);
    PrintReg();
    if (enable_dump) {
        input_->DumpMatrix(GetDumpName("cu_input"));
        wt.DumpMatrix(GetDumpName("cu_wt"));
    }
    if      (conv_mode_ == CONV_MODE_BYPASS)            DoBypass();
    else if (conv_mode_ == CONV_MODE_3x3_fullCH)        DoConvolution_3x3_fullCH(wt);
    else if (conv_mode_ == CONV_MODE_3x3_QuarterCH)     DoConvolution_3x3_QuarterCH(wt);
    else if (conv_mode_ == CONV_MODE_DW)                DoDepthWise(wt);
    else if (conv_mode_ == CONV_MODE_1x1)               DoConvolution1x1(wt);
    else if (conv_mode_ == CONV_MODE_ADD)               DoAdd(ds);

    // Evaluation();
    if (enable_dump) {
        output_->DumpMatrix(GetDumpName("cu_before_coarseshift"));
    }
    DoCoarseShift(cs);
    if (enable_dump) {
        output_->DumpMatrix(GetDumpName("cu_output"));
    }

}

std::string Mac::GetDumpName(const std::string& base_name) {
    std::string name = base_name + "_node" + std::to_string(CURRENT_NPU_NODE_INDEX);
    name += "_mac" + std::to_string(mac_call_count_);
    return name;
}

void Mac::SetInputFM(Sptr<Tensor<int>> input){
    input_ = input;
}

void Mac::SetOutputFM(Sptr<Tensor<int>> output){
    output_ = output;
}

void Mac::SetWeight(Sptr<Weight> wt){
   origin_weight_ = wt;
}

void Mac::SetLUTMem(Sptr<Memory> lut_mem){
    LUT_memory_ = lut_mem;
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/13 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Mac::AssignWeight() {
    switch (conv_mode_)
    {   
        case CONV_MODE_3x3_fullCH:                      weight_len_ = ich_group_num_ * WEIGHT_BLOCK_CYCLE_3x3_fullCH_ * och_group_num_; break;
        case CONV_MODE_3x3_QuarterCH:                   weight_len_ = ich_group_num_ * WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_ * och_group_num_; break;
        case CONV_MODE_DW:                              weight_len_ = WEIGHT_BLOCK_CYCLE_DW_ * och_group_num_; break;
        case CONV_MODE_1x1:                             weight_len_ = ich_group_num_ * WEIGHT_BLOCK_CYCLE_1x1_ * och_group_num_; break;
        case CONV_MODE_BYPASS: case CONV_MODE_ADD:      weight_len_ = 0; break;
        default:                                        std::cout << "Conv Mode Error!" << std::endl;exit(1); break;
    }

    weight_mac_->weight_->Resize(1, 1, weight_len_, IN_FIFO_WIDTH);
    weight_param_->weight_->Resize(1, 1, och_group_num_*PARAMETER_BLOCK_CYCLE_, IN_FIFO_WIDTH);


    for (int i = 0; i < weight_len_; i++)    //weight for mac 
    {
        int idx = 0;
        switch (conv_mode_)
        {
        case CONV_MODE_3x3_fullCH:       idx = floor((double) (i-WEIGHT_BLOCK_CYCLE_3x3_fullCH_)/(WEIGHT_BLOCK_CYCLE_3x3_fullCH_*ich_group_num_)) + 1; break;
        case CONV_MODE_3x3_QuarterCH:    idx = floor((double) (i-WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_)/(WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_*ich_group_num_)) + 1; break;
        case CONV_MODE_DW:               idx = floor((double) (i-WEIGHT_BLOCK_CYCLE_DW_)/(WEIGHT_BLOCK_CYCLE_DW_*ich_group_num_)) + 1; break; 
        case CONV_MODE_1x1:              idx = floor((double) (i-WEIGHT_BLOCK_CYCLE_1x1_)/WEIGHT_BLOCK_CYCLE_1x1_/ich_group_num_) + 1; break;
        default:                         idx = 0; break;
        }
        for (int j = 0; j < IN_FIFO_WIDTH; j++) {
            weight_mac_->weight_->Set(0, 0, i, j, origin_weight_->weight_->Get(0, 0, weight_idx_ + i + idx * PARAMETER_BLOCK_CYCLE_, j));
        }
    }
    for (int i = 0; i < och_group_num_ * PARAMETER_BLOCK_CYCLE_; i++)    //parameter for psum and nl
    {
        int idx = 0;
        switch (conv_mode_)
        {
        case CONV_MODE_3x3_fullCH:         idx =  (i/4)*ich_group_num_*WEIGHT_BLOCK_CYCLE_3x3_fullCH_ + WEIGHT_BLOCK_CYCLE_3x3_fullCH_; break;
        case CONV_MODE_3x3_QuarterCH:      idx =  (i/4)*ich_group_num_*WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_ + WEIGHT_BLOCK_CYCLE_3x3_QuarterCH_; break;
        case CONV_MODE_DW:                 idx =  (i/4)*ich_group_num_*WEIGHT_BLOCK_CYCLE_DW_ + WEIGHT_BLOCK_CYCLE_DW_;                  break;      
        case CONV_MODE_1x1:                idx =  (i/4)*ich_group_num_*WEIGHT_BLOCK_CYCLE_1x1_ + WEIGHT_BLOCK_CYCLE_1x1_; break;
        default: idx = 0; break;
        }
        for (int j = 0; j < IN_FIFO_WIDTH; j++) {
            weight_param_->weight_->Set(0, 0, i, j, origin_weight_->weight_->Get(0, 0, weight_idx_ +  i + idx, j));
        }
    }
    weight_idx_ += (weight_len_ + och_group_num_* PARAMETER_BLOCK_CYCLE_);
    weight_idx_ = (weight_idx_ == origin_weight_->weight_->GetRow()) ? 0 : weight_idx_; 
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/12 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Mac::Init(){
    conv_mode_ = (*reg_file_)["reg_CONV_MODE"]["mode"];
    ich_num_ = input_->GetPsumDim();
    och_num_ = (*reg_file_)["reg_FM_OCH_ED"]["och_ed"] - (*reg_file_)["reg_FM_OCH_ST"]["och_st"] + 1;

    switch (conv_mode_)
    {
        case CONV_MODE_3x3_fullCH:
            och_num_ =  ((och_num_-1)/OCH_PER_GROUP_fullCH + 1) * OCH_PER_GROUP_fullCH;
            ich_group_num_ = (ich_num_ - 1)/ICH_PER_GROUP_fullCH + 1; 
            och_group_num_ = (och_num_ - 1)/OCH_PER_GROUP_fullCH + 1;  
            std::cout << "Conv Mode Must Not Be one!" << std::endl;    
            exit(1);    
            break;
        case CONV_MODE_3x3_QuarterCH:
            och_num_ =  ((och_num_-1)/OCH_PER_GROUP_quarterCH + 1) * OCH_PER_GROUP_quarterCH;
            ich_group_num_ = (ich_num_ - 1)/ICH_PER_GROUP_quarterCH + 1; 
            och_group_num_ = (och_num_ - 1)/OCH_PER_GROUP_quarterCH + 1;      
            break; 
        case CONV_MODE_1x1:
            och_num_ =  ((och_num_-1)/OCH_PER_GROUP_1x1 + 1) * OCH_PER_GROUP_1x1;
            ich_group_num_ = (ich_num_ - 1)/ICH_PER_GROUP_1x1 + 1; 
            och_group_num_ = (och_num_ - 1)/OCH_PER_GROUP_1x1 + 1;      
            break;
        case CONV_MODE_DW:
            och_num_ =  ((och_num_-1)/OCH_PER_GROUP_DW + 1) * OCH_PER_GROUP_DW;
            ich_group_num_ = (ich_num_ - 1)/ICH_PER_GROUP_DW + 1; 
            och_group_num_ = (och_num_ - 1)/OCH_PER_GROUP_DW + 1;      
            break;        
        case CONV_MODE_BYPASS: case CONV_MODE_ADD:
            och_num_ =  ((och_num_-1)/OCH_PER_GROUP_BYPASS_ADD + 1) * OCH_PER_GROUP_BYPASS_ADD;
            ich_group_num_ = (ich_num_ - 1)/ICH_PER_GROUP_BYPASS_ADD + 1; 
            och_group_num_ = (och_num_ - 1)/OCH_PER_GROUP_BYPASS_ADD + 1;      
            break;    
        default:
            std::cout << "Conv Mode Error!" << std::endl;    
            exit(1);
    }

    row_num_i_ = input_->GetRow();
    col_num_i_ = input_->GetCol();
    stride_ = (*reg_file_)["reg_CONV_MODE"]["trim"] + 1;

    pad_l_ = (*reg_file_)["reg_PAD2"]["l"];
    pad_r_ = (*reg_file_)["reg_PAD2"]["r"];
    pad_t_ = (*reg_file_)["reg_PAD1"]["t"];
    pad_b_ = (*reg_file_)["reg_PAD1"]["b"];

    switch (conv_mode_)
    {
    case CONV_MODE_1x1: case CONV_MODE_3x3_fullCH: case CONV_MODE_DW:
        kernel_width_ = 1;
        kernel_height_ = 1;
        row_num_o_ = row_num_i_;
        col_num_o_ = col_num_i_; break;
    case CONV_MODE_3x3_QuarterCH:  
        kernel_width_ = 3;
        kernel_height_ = 1;
        row_num_o_ = row_num_i_;
        col_num_o_ = col_num_i_-2; break;
    default: 
        kernel_width_ = 0;
        kernel_height_ = 0;
        row_num_o_ = row_num_i_;
        col_num_o_ = col_num_i_; break;
    }
    CreateOutFM();
    weight_mac_ = std::make_shared<Weight>();
    weight_param_ = std::make_shared<Weight>();
    logger_ = spdlog::get("s000_log");
}

void Mac::PrintReg() {
    int mode = (*reg_file_)["reg_CONV_MODE"]["mode"];
    int trim = (*reg_file_)["reg_CONV_MODE"]["trim"];
    int AB_order = (*reg_file_)["reg_CONV_MODE"]["AB_order"];
    int full_ch = (*reg_file_)["reg_CONV_MODE"]["full_ch"];
    int ch_st = (*reg_file_)["reg_CONV_MODE"]["ch_st"];
    int upsample = (*reg_file_)["reg_CONV_MODE"]["upsample"];

    int row = (*reg_file_)["reg_FM_ROW"]["row"];
    int col = (*reg_file_)["reg_FM_COL"]["col"];
    int ich = (*reg_file_)["reg_FM_ICH"]["ich"];
    int och_st = (*reg_file_)["reg_FM_OCH_ST"]["och_st"];
    int och_ed = (*reg_file_)["reg_FM_OCH_ED"]["och_ed"];

    int offset_x_in1 = (*reg_file_)["reg_MEM_IN1"]["offset_x"];
    int offset_y_in1 = (*reg_file_)["reg_MEM_IN1"]["offset_y"];

    int offset_x_in2 = (*reg_file_)["reg_MEM_IN2"]["offset_x"];
    int offset_y_in2 = (*reg_file_)["reg_MEM_IN2"]["offset_y"];

    int offset_x_out = (*reg_file_)["reg_MEM_OUT"]["offset_x"];
    int offset_y_out = (*reg_file_)["reg_MEM_OUT"]["offset_y"];

    int col_st = (*reg_file_)["reg_CROP"]["col_st"];
    int row_st = (*reg_file_)["reg_CROP"]["row_st"];

    int row_out = (*reg_file_)["reg_CROP_ROW"]["row_out"];
    int col_out = (*reg_file_)["reg_CROP_COL"]["col_out"];

    int pad_r = (*reg_file_)["reg_PAD2"]["r"];
    int pad_l = (*reg_file_)["reg_PAD2"]["l"];
    int pad_b = (*reg_file_)["reg_PAD1"]["b"];
    int pad_t = (*reg_file_)["reg_PAD1"]["t"];
    int wbm_offset_x = (*reg_file_)["reg_MEM_OUT"]["offset_x"];
    int wbm_offset_y = (*reg_file_)["reg_MEM_OUT"]["offset_y"];

    logger_->info("---------------------------start_infor----------------------------------------------------");
     logger_->info("conv mode: {}, ich_num: {}, och_num: {}, ch_dim: {}, row_num: {}, col_num: {}", conv_mode_, ich_num_, och_num_, input_->GetCh(), row_num_i_, col_num_i_);
    logger_->info("mode: {}, trim: {},  AB_order: {}, full_ch : {}, ch_st: {}, upsample: {}", mode,trim, AB_order,full_ch, ch_st, upsample);
    logger_->info("row: {}, col: {}, ich: {}, och_st: {}, och_ed: {}", row, col, ich, och_st, och_ed);
    logger_->info(
            "offset_x_in1: {}, offset_y_in1: {}, offset_x_in2: {}, offset_y_in2: {}, offset_x_out: {}, offset_y_out: {}",
            offset_x_in1, offset_y_in1, offset_x_in2, offset_y_in2, offset_x_out, offset_y_out);
    logger_->info("col_st:{}, row_st:{}, row_out:{}, col_out:{}", col_st, row_st, row_out, col_out);
    logger_->info("pad_r: {}, pad_l: {}, pad_b: {}, pad_t: {}", pad_r, pad_l, pad_b, pad_t);
    logger_->info("cu_row_in: {}, cu_col_in: {}, cu_row_out: {}, cu_col_out: {}", row_num_i_,col_num_i_,row_num_o_,col_num_o_);
    logger_->info("---------------------------end_infor----------------------------------------------------");
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/13 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Mac::ParseWeight(Tensor<int> & wt, Tensor<int> & cs, Tensor<int> & ds){
    wt.Resize(och_num_, ich_num_,kernel_height_, kernel_width_);
    cs.Resize(och_num_, 1, 1, 1);
    ds.Resize(och_num_, 1, 1, 1);

    if (conv_mode_ == WEIGHT_BLOCK_CYCLE_3x3_fullCH_){
        int weight_len_byte = weight_mac_->weight_->GetSize();
        int result;
        int source_idx;

        if ((weight_len_byte%IN_FIFO_WIDTH != 0)) {
            std::cout << "Conv weight group num have Error!" << std::endl;
            std::exit(1);
        }
        if (ShouldDumpCurrentNode()) {
            weight_mac_->weight_->DumpMatrix(GetDumpName("weight_mac"));
        }
        for (int och_group=0; och_group<och_group_num_; och_group++){
            for (int ich_group=0; ich_group<ich_group_num_; ich_group++){
                for(int ich_group_32=0; ich_group_32 < 4; ich_group_32 ++)
                    for(int och_count = 0; och_count<OCH_PER_GROUP_1x1; och_count++){
                        for(int ich_count = 0; ich_count<(ICH_PER_GROUP_1x1/4); ich_count++){
                            source_idx = och_group * ich_group_num_ * ICH_PER_GROUP_1x1 * OCH_PER_GROUP_1x1 + ich_group * ICH_PER_GROUP_1x1 * OCH_PER_GROUP_1x1 + ich_group_32 * OCH_PER_GROUP_1x1 * 32 + och_count * 32 + ich_count;
                            result = weight_mac_->weight_->Get(0, 0, source_idx/IN_FIFO_WIDTH, 128 - 1 - source_idx%IN_FIFO_WIDTH);
                            wt.Set(och_group*OCH_PER_GROUP_1x1 + och_count, ich_group * ICH_PER_GROUP_1x1 + ich_group_32 * 32 + ich_count, 0 , 0, result);
                        }
                    }
            }
        }
        //there is debug information in s900_cmodel
    } else if(conv_mode_ == CONV_MODE_3x3_QuarterCH){
        int weight_len_byte = weight_mac_->weight_->GetSize();
        int result;
        int source_idx;

        if ((weight_len_byte%IN_FIFO_WIDTH != 0)) {
            std::cout << "Conv weight group num have Error!" << std::endl;
            std::exit(1);
        }

        for (int och_group=0; och_group<och_group_num_; och_group++){
            for (int ich_group=0; ich_group<ich_group_num_; ich_group++){
                for (int position_count = 0; position_count<kernel_width_; position_count++){
                    for(int och_count = 0; och_count<OCH_PER_GROUP_quarterCH; och_count++){
                        for(int ich_count = 0; ich_count<ICH_PER_GROUP_quarterCH; ich_count++){
                            source_idx = och_group * ich_group_num_ * ICH_PER_GROUP_quarterCH * OCH_PER_GROUP_quarterCH * kernel_width_ + ich_group * ICH_PER_GROUP_quarterCH * OCH_PER_GROUP_quarterCH * kernel_width_ + position_count * ICH_PER_GROUP_quarterCH * OCH_PER_GROUP_quarterCH + och_count * ICH_PER_GROUP_quarterCH + ich_count;
                            result = weight_mac_->weight_->Get(0, 0, source_idx/IN_FIFO_WIDTH,128 - 1 - source_idx%IN_FIFO_WIDTH);
                            wt.Set(och_group*OCH_PER_GROUP_quarterCH + och_count, ich_group * ICH_PER_GROUP_quarterCH + ich_count, 0, position_count, result);
                        }
                    }
                }
            }
        }
    } else if(conv_mode_ == CONV_MODE_DW){
        wt.Resize(1, ich_num_,kernel_height_, kernel_width_);
        int weight_len_byte = weight_mac_->weight_->GetSize();
        int result_0;
        int result_1;
        int result_2;
        int source_idx;

        if ((weight_len_byte%IN_FIFO_WIDTH != 0)) {
            std::cout << "Conv weight group num have Error!" << std::endl;
            std::exit(1);
        }

        int dw_channel8_group = (ich_num_ - 1)/8 + 1;


        for(int ich_ch8_count = 0; ich_ch8_count < 8; ich_ch8_count++){
            for(int ich_count = 0; ich_count < 8; ich_count++){
                int index_row = 2 * ich_ch8_count + (ich_count / 4);
                int index_col;
                if(ich_count <= 3){
                    index_col = 128 -1 - ich_count * 33;
                } else {
                    index_col = 128 -1 - 4 - (ich_count-4) * 33;
                }
                result_0 = weight_mac_->weight_->Get(0, 0, index_row, index_col);                
                wt.Set(0, 8 * ich_ch8_count + ich_count, 0 , 0, result_0);
            }
        }

            
    } else if (conv_mode_ == CONV_MODE_1x1) {
        int weight_len_byte = weight_mac_->weight_->GetSize();
        int result;
        int source_idx;

        if ((weight_len_byte%IN_FIFO_WIDTH != 0)) {
            std::cout << "Conv weight group num have Error!" << std::endl;
            std::exit(1);
        }
        if (ShouldDumpCurrentNode()) {
            weight_mac_->weight_->DumpMatrix(GetDumpName("weight_mac"));
        }
        for (int och_group=0; och_group<och_group_num_; och_group++){
            for (int ich_group=0; ich_group<ich_group_num_; ich_group++){
                for(int ich_group_32=0; ich_group_32 < 4; ich_group_32 ++)
                    for(int och_count = 0; och_count<OCH_PER_GROUP_1x1; och_count++){
                        for(int ich_count = 0; ich_count<(ICH_PER_GROUP_1x1/4); ich_count++){
                            source_idx = och_group * ich_group_num_ * ICH_PER_GROUP_1x1 * OCH_PER_GROUP_1x1 + ich_group * ICH_PER_GROUP_1x1 * OCH_PER_GROUP_1x1 + ich_group_32 * OCH_PER_GROUP_1x1 * 32 + och_count * 32 + ich_count;
                            result = weight_mac_->weight_->Get(0, 0, source_idx/IN_FIFO_WIDTH, 128 - 1 - source_idx%IN_FIFO_WIDTH);
                            wt.Set(och_group*OCH_PER_GROUP_1x1 + och_count, ich_group * ICH_PER_GROUP_1x1 + ich_group_32 * 32 + ich_count, 0 , 0, result);
                        }
                    }
            }
        }

    } else if (conv_mode_ == CONV_MODE_BYPASS || conv_mode_ == CONV_MODE_ADD) {
        wt = wt;
    } else{
        std::cerr << "Error: Doesn't have this convolution type in parse weight!" << std::endl;
        std::exit(1);
    }

    param_vector_->resize(och_group_num_ * PARAMETER_BLOCK_CYCLE_);
    for (int i = 0; i < och_group_num_ * PARAMETER_BLOCK_CYCLE_; i++) {
        string param = "";
        for (int j = 0; j < IN_FIFO_WIDTH; j++) {
            std::string tmpA = "";
            std::string tmpB = "";
            tmpA =  std::bitset<8>(weight_param_->weight_->Get(0, 0, i, j)).to_string();
            param += tmpA;
        }
//        param_vector_->emplace_back(param);
        param_vector_->at(i) = param;
    }

    for (int i = 0; i < och_group_num_; i++) {
        std::vector<std::string> c_shift(64);
        for (int j = 0; j < 64; j++) {
            c_shift.at(j) = param_vector_->at(i*4 + 3).substr(1024 - 3 - j*3,3); 
            cs.Set(i * 64 + j, 0, 0, 0, Weight::bin_to_uint(c_shift.at(j)));
        }
    }
    for (int i = 0; i < och_group_num_; i++) {
        for (int j = 0; j < 64; j++) {
            std::vector<std::string> d_shift(64);
            d_shift.at(j) = param_vector_->at(i*4 + 1).substr(1024 - 4 - j*4,4);
            ds.Set(i * 64 + j, 0, 0, 0, Weight::bin_to_uint(d_shift.at(j)));
        }
    }
}

void Mac::DoConvolution_3x3_fullCH(const Tensor<int> & wt){
    for (int on = 0; on < och_num_; on++) {
        for (int ign = 0; ign < ich_group_num_; ign++) {    //psum demension
            for (int r = 0; r < row_num_i_; r++) {
                for (int c = 0; c < col_num_i_; c++) {
                    int res = 0;
                    for (int ch = 0; ch < ICH_PER_GROUP_fullCH; ch++) {
                        // int result_appro = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),input_->Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro = wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0) * input_->Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c);
                        res += result_appro;
                    }
                    Tensor<int>::ClampToLSB(res, 23);
                    output_->Set(on, ign, r, c, res);
                }
            }
        }
    }
}


void Mac::DoConvolution_3x3_QuarterCH(const Tensor<int> & wt){
    for (int on = 0; on < och_num_; on++) {
        for (int ign = 0; ign < ich_group_num_; ign++) {    //psum demension
            for (int r = 0; r < row_num_o_; r++) {
                for (int c = 0; c < col_num_o_; c++) {
                    int res = 0;
                    for (int ch = 0; ch < ICH_PER_GROUP_quarterCH; ch++) {
                        for (int k1 = 0; k1 < kernel_width_; k1++) {
                            // int result_appro = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_quarterCH+ch, 0, k1),input_->Get(0, ign*ICH_PER_GROUP_quarterCH+ch, r , c + k1));
                            int result_appro = wt.Get(on, ign*ICH_PER_GROUP_quarterCH+ch, 0, k1) * input_->Get(0, ign*ICH_PER_GROUP_quarterCH+ch, r , c + k1);
                            res +=  result_appro ;
                        }
                    }   
                    Tensor<int>::ClampToLSB(res, 23);
                    output_->Set(on, ign, r, c, res);
                }
            }
        }
    }    
}

void Mac::DoConvolution1x1(const Tensor<int> & wt){
    //DoConvolution1x1_dump(wt);
    for (int on = 0; on < och_num_; on++) {
        for (int ign = 0; ign < ich_group_num_; ign++) {    //psum demension
            for (int r = 0; r < row_num_i_; r++) {
                for (int c = 0; c < col_num_i_; c++) {
                    int res = 0;
                    for (int ch = 0; ch < ICH_PER_GROUP_1x1; ch++) {
                        // int result_appro = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),input_->Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro =wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0) * input_->Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c);
                        res += result_appro;
                    }
                    Tensor<int>::ClampToLSB(res, 23);
                    output_->Set(on, ign, r, c, res);
                }
            }
        }
    }
    if (ShouldDumpCurrentNode()) {
        output_->DumpMatrix(GetDumpName("cu_conv1x1_result"));
    }
}

void Mac::DoConvolution1x1_dump(const Tensor<int> & wt){
    Tensor<int> temple_2bitx8bit_1;
    Tensor<int> temple_2bitx8bit_2;
    Tensor<int> temple_2bitx8bit_3;
    Tensor<int> temple_2bitx8bit_4;
    Tensor<int> temple_2bitx8bit_5;

    Tensor<int> temple_2bitx8bit_1_2bit;
    Tensor<int> temple_2bitx8bit_2_2bit;
    Tensor<int> temple_2bitx8bit_3_2bit;
    Tensor<int> temple_2bitx8bit_4_2bit;
    Tensor<int> temple_2bitx8bit_5_2bit;

    temple_2bitx8bit_1.Resize(och_num_, ich_group_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_2.Resize(och_num_, ich_group_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_3.Resize(och_num_, ich_group_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_4.Resize(och_num_, ich_group_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_5.Resize(och_num_, ich_group_num_,row_num_i_, col_num_i_);

    temple_2bitx8bit_1_2bit.Resize(och_num_, ich_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_2_2bit.Resize(och_num_, ich_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_3_2bit.Resize(och_num_, ich_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_4_2bit.Resize(och_num_, ich_num_,row_num_i_, col_num_i_);
    temple_2bitx8bit_5_2bit.Resize(och_num_, ich_num_,row_num_i_, col_num_i_);

    for (int on = 0; on < och_num_; on++) {
        for (int ign = 0; ign < ich_group_num_; ign++) {    //psum demension
            for (int r = 0; r < row_num_i_; r++) {
                for (int c = 0; c < col_num_i_; c++) {
                    for (int ch = 0; ch < ICH_PER_GROUP_1x1; ch++) {
                        int bin_input[8];
                        int dec_input;
                        int dec_2bit_input[5];
                        dec_input = input_->Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c);
                        if(dec_input < 0){
                            dec_input = dec_input + 256;
                        }
                        for(int i_1=0; i_1<=7; i_1++){
                            bin_input[i_1] = dec_input % 2;
                            dec_input = dec_input / 2;
                        }
                        for(int i_2=0; i_2<=4; i_2++)
                        {
                            if(i_2 <= 2){
                                dec_2bit_input[i_2] = bin_input[i_2*2] * 1 + bin_input[i_2*2+1] * 2;
                            } else{
                                dec_2bit_input[i_2] =  bin_input[i_2+3];
                            }
                        }
                        temple_2bitx8bit_1_2bit.Set(on, ign*ICH_PER_GROUP_1x1+ch, r, c, dec_2bit_input[0]);
                        temple_2bitx8bit_2_2bit.Set(on, ign*ICH_PER_GROUP_1x1+ch, r, c, dec_2bit_input[1]);                                            
                        temple_2bitx8bit_3_2bit.Set(on, ign*ICH_PER_GROUP_1x1+ch, r, c, dec_2bit_input[2]);
                        temple_2bitx8bit_4_2bit.Set(on, ign*ICH_PER_GROUP_1x1+ch, r, c, dec_2bit_input[3]);
                        temple_2bitx8bit_5_2bit.Set(on, ign*ICH_PER_GROUP_1x1+ch, r, c, dec_2bit_input[4]);
                    }

                    int res_1 = 0;
                    int res_2 = 0;
                    int res_3 = 0;
                    int res_4 = 0;
                    int res_5 = 0; 

                    for (int ch = 0; ch < ICH_PER_GROUP_1x1; ch++) {
                        int result_appro_1 = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),temple_2bitx8bit_1_2bit.Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro_2 = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),temple_2bitx8bit_2_2bit.Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro_3 = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),temple_2bitx8bit_3_2bit.Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro_4 = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),temple_2bitx8bit_4_2bit.Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        int result_appro_5 = appro_mul(wt.Get(on, ign*ICH_PER_GROUP_1x1+ch, 0, 0),temple_2bitx8bit_5_2bit.Get(0, ign*ICH_PER_GROUP_1x1+ch, r, c));
                        res_1 += result_appro_1;
                        res_2 += result_appro_2;
                        res_3 += result_appro_3;
                        res_4 += result_appro_4;
                        res_5 += result_appro_5;
                    }

                    temple_2bitx8bit_1.Set(on, ign, r, c, res_1);
                    temple_2bitx8bit_2.Set(on, ign, r, c, res_2);
                    temple_2bitx8bit_3.Set(on, ign, r, c, res_3);
                    temple_2bitx8bit_4.Set(on, ign, r, c, res_4);
                    temple_2bitx8bit_5.Set(on, ign, r, c, res_5);
                }
            }
        }
    }
    if (ShouldDumpCurrentNode()) {
        temple_2bitx8bit_1.DumpMatrix(GetDumpName("cu_2bitx8bit_1"));
        temple_2bitx8bit_2.DumpMatrix(GetDumpName("cu_2bitx8bit_2"));
        temple_2bitx8bit_3.DumpMatrix(GetDumpName("cu_2bitx8bit_3"));
        temple_2bitx8bit_4.DumpMatrix(GetDumpName("cu_2bitx8bit_4"));
        temple_2bitx8bit_5.DumpMatrix(GetDumpName("cu_2bitx8bit_5"));
    }
}


void Mac::DoDepthWise(const Tensor<int> & wt){
    for (int ich = 0; ich < ich_num_; ich++) {
        for (int r = 0; r < row_num_o_; r++) {
            for (int c = 0; c < col_num_o_; c++) {
                // int result_appro = appro_mul(wt.Get(0, ich, 0, 0),input_->Get(0, ich, r, c));
                int result_appro = wt.Get(0, ich, 0, 0) * input_->Get(0, ich, r, c);
                Tensor<int>::ClampToLSB(result_appro, 23);
                output_->Set(ich, 0, r, c, result_appro);
            }
        }
    }
}

void Mac::DoBypass(){
    for (int ich = 0; ich < ich_num_; ich++) {
        for (int r = 0; r < row_num_i_; r++) {
            for (int c = 0; c < col_num_i_; c++) {
                output_->Set(ich, 0, r, c, input_->Get(0, ich, r, c));
            }
        }
    }
}

void Mac::DoCoarseShift(const Tensor<int> & cs){
    int psum_dim = 0;
    switch (conv_mode_)
    {
        case CONV_MODE_3x3_fullCH: case CONV_MODE_3x3_QuarterCH:        case CONV_MODE_1x1: psum_dim = ich_group_num_; break;
        case CONV_MODE_BYPASS: case CONV_MODE_DW:                       case CONV_MODE_ADD: psum_dim = 1; break;
        default: std::cout<< "Error: Doesn't have this convolution type!" << std::endl; break;
    }

    for (int on = 0; on < och_num_; on++) {
        int coarse_shift = (cs.Get(on, 0, 0, 0) - 2) * 4; 
        if (coarse_shift < 0) {
            for (int ign = 0; ign < psum_dim; ign++) {
                for (int r = 0; r < row_num_o_; r++) {
                    for (int c = 0; c < col_num_o_; c++) {
                        int tmp = output_->Get(on, ign, r, c) * (int) pow(2, -coarse_shift);
                        if (tmp > CS_WIDTH/2 - 1) tmp = CS_WIDTH/2 - 1;
                        else if (tmp < -CS_WIDTH/2) tmp = -CS_WIDTH/2;
                        output_->Set(on, ign, r, c, tmp);
                    }
                }
            }
        } else if (coarse_shift > 0) {
            for (int ign = 0; ign < psum_dim; ign++) {
                for (int r = 0; r < row_num_o_; r++) {
                    for (int c = 0; c < col_num_o_; c++) {
                        output_->Set(on, ign, r, c, output_->Get(on, ign, r, c) >> coarse_shift);  
                    }
                }
            }
        }
    }
    // Clamp output within -32768---32767
    for (int on = 0; on < och_num_; on++) {
        for (int ign = 0; ign < psum_dim; ign++) {
            for (int r = 0; r < row_num_o_; r++) {
                for (int c = 0; c < col_num_o_; c++) {
                    int tmp = output_->Get(on, ign, r, c);
                    if (tmp > 32767) tmp = 32767;
                    else if (tmp < -32768) tmp = -32768;
                    output_->Set(on, ign, r, c, tmp);  
                }
            }
        }
    }
}

void Mac::CreateOutFM(){
    switch (conv_mode_)
    {
        case CONV_MODE_3x3_fullCH: case CONV_MODE_3x3_QuarterCH: case CONV_MODE_1x1: output_->Resize(och_num_, ich_group_num_, row_num_o_, col_num_o_); break;
        case CONV_MODE_BYPASS: case CONV_MODE_DW: case CONV_MODE_ADD: output_->Resize(och_num_, 1, row_num_o_, col_num_o_); break;
        default: std::cout << "Error: Doesn't have this convolution mode!" << std::endl; break;
    }
}

void Mac::DoAdd(const Tensor<int> & ds){
    assert(input_->GetCh() == 2);
    for (int ch = 0; ch < ich_num_; ++ch) {
        int shift_value = ds.Get(ch, 0, 0, 0) - 6; 
        for (int r = 0; r < row_num_i_; ++r) {
            for (int c = 0; c < col_num_i_; ++c) {
                int res = 0;
                if (shift_value > 0) {
                    res = input_->Get(0, ch, r, c) + (input_->Get(1, ch, r, c) >> shift_value);
                } else if (shift_value == 0) {
                    res = input_->Get(0, ch, r, c) + input_->Get(1, ch, r, c);
                } else {
                    res = input_->Get(0, ch, r, c) + (input_->Get(1, ch, r, c) * pow(2, (-shift_value)));
                }
                Tensor<int>::ClampToLSB(res, 23);
                output_->Set(ch, 0, r, c, res);
            }
        }
    }
}

int  Mac::appro_mul(int a, int b) {
    int bin_a[8];
    int bin_b[8];

    int ori_a = a;
    int ori_b = b;

    if ( a < 0 ){
        a = 256 + a;
    }
    if ( b < 0){
        b = 256 + b;
    }
    for(int i_1=0; i_1<=7; i_1++)
    {
        bin_a[i_1] = a % 2;
        bin_b[i_1] = b % 2;
        a = a /2;
        b = b /2;
    }

    int dec_a[5];
    int dec_b[5];

    int flag_a = 0;
    int flag_b = 0;

    for(int i_2=0; i_2<=4; i_2++)
    {
        if(i_2 <= 2){
            dec_a[i_2] = bin_a[i_2*2] * 1 + bin_a[i_2*2+1] * 2;
            dec_b[i_2] = bin_b[i_2*2] * 1 + bin_b[i_2*2+1] * 2;
        } else{
            dec_a[i_2] =  bin_a[i_2+3];
            dec_b[i_2] =  bin_b[i_2+3];
        }
        if(dec_a[i_2] == 3){
            flag_a = 1;
        }
        if(dec_b[i_2] == 3){
            flag_b = 1;
        }
    }

    int result = 0;

    if((flag_a + flag_b) == 2){
        for(int i_3=0; i_3<=4; i_3++)
        {
            int result_inter = 0;
            for(int i_4=0; i_4<=4; i_4++)
            {
                int temple = 0;
                temple = dec_a[i_3] * dec_b[i_4];
                if (temple == 9) {
                    temple = 7;
                }
                if (i_4 == 4) {
                    temple = -128 * temple;
                } else {
                    temple = temple * pow(2,(i_4*2));
                }
                result_inter += temple;
            }

            if (i_3 == 4) {
                result_inter = -128 * result_inter;
            } else {
                result_inter = result_inter * pow(2,(i_3*2));
            }

            result += result_inter;

        }
    } else{
        result = ori_a * ori_b;
    }

    return result;
}

 /* The function has been unused*/
void Mac::Evaluation() const {
}

/*****************************************************************************
*  打印 Mac 调用次数统计
*****************************************************************************/
void Mac::PrintMacCallCount() {
    std::cout << "========================================" << std::endl;
    std::cout << "Mac Total Call Count: " << mac_call_count_ << std::endl;
    std::cout << "========================================" << std::endl;
}

bool Mac::SanityCheck() {
    return 0;
}

void Mac::Dump(){}

void Mac::CleanUp(){}