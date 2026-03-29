#include "NPU.h"
#include <vector>

using std::vector;
using std::make_shared;

const int NPU::ICH_PER_GROUP       = 32;
const int NPU::BANK_LINE           = 16;
const int NPU::BANK_COL            = 1024;

namespace {
const char* OpcodeToString(Opcode opcode) {
    switch (opcode) {
        case Opcode::NOP: return "NOP";
        case Opcode::SYNC: return "SYNC";
        case Opcode::INTR: return "INTR";
        case Opcode::CONV: return "CONV";
        case Opcode::LOOPCOUNT: return "LOOPCOUNT";
        case Opcode::LOOP: return "LOOP";
        case Opcode::REGWRITE: return "REGWRITE";
        case Opcode::REGINC: return "REGINC";
        default: return "UNKNOWN";
    }
}

const char* ConvModeToString(int mode) {
    switch (mode) {
        case 0: return "BYPASS";
        case 1: return "CONV3x3_FULLCH";
        case 2: return "CONV3x3_QUARTERCH";
        case 3: return "DEPTHWISE";
        case 4: return "CONV1x1";
        case 5: return "ADD";
        default: return "UNKNOWN_CONV_MODE";
    }
}

const char* NlOrderToString(int order) {
    switch (order) {
        case 0: return "bn_relu_pool";
        case 1: return "bn_pool_relu";
        case 2: return "relu_bn_pool";
        case 3: return "relu_pool_bn";
        case 4: return "pool_bn_relu";
        case 5: return "pool_relu_bn";
        default: return "unknown_nl_order";
    }
}
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/

void NPU::Init() {
    // In init, all the building blocks are created and connected
    //   - All the necessary hardware blocks are created and connected
    //   - All the initialization would be done init this phase
    //   - The input data will be inserted to NMEM
    //   - Created weight file buffer
    //   - Created instruction buffer

    //create object
    reg_file_ = make_shared<RegisterFile>();
    weight_   = make_shared<Weight>();
    lut_mem_  = make_shared<Memory>(1, 16, 256);
    psum_mem_ = make_shared<Memory>(32, 8, 2048);       
    rbm_out_  = make_shared<Tensor<int>>();
    mac_out_  = make_shared<Tensor<int>>();
    trim_out_ = make_shared<Tensor<int>>();
    psum_out_ = make_shared<Tensor<int>>();
    lut_out_  = make_shared<Tensor<int>>();
    bn_out_   = make_shared<Tensor<int>>();
    relu_out_ = make_shared<Tensor<int>>();
    pool_out_ = make_shared<Tensor<int>>();
    bank_a_   = make_shared<Memory>(ICH_PER_GROUP, BANK_LINE, BANK_COL);
    bank_b_   = make_shared<Memory>(ICH_PER_GROUP, BANK_LINE, BANK_COL);
    program_cnt_ = 0;
    //build register
    reg_file_->BuildRegisterFile();
    //set bank
    rbm_.SetBank(bank_a_, bank_b_);
    wbm_.SetBank(bank_a_, bank_b_);
    psum_.SetPMEM(psum_mem_);
    lut_.SetLUTMEM(lut_mem_);
    mac_.SetLUTMem(lut_mem_);
    //set input and output
    rbm_.SetOutputFM(rbm_out_);
    mac_.SetInputFM(rbm_out_);
    mac_.SetOutputFM(mac_out_);
    trim_.SetInputFM(mac_out_);
    trim_.SetOutputFM(trim_out_);
    psum_.SetInputFM(trim_out_);
    psum_.SetOutputFM(psum_out_);
    bn_.SetInputFM(lut_out_);
    bn_.SetOutputFM(bn_out_);
    relu_.SetInputFM(bn_out_);
    relu_.SetOutputFM(relu_out_);
    lut_.SetInputFM(relu_out_);
    lut_.SetOutputFM(lut_out_);
    pool_.SetInputFM(relu_out_);
    pool_.SetOutputFM(pool_out_);
    wbm_.SetInputFM(pool_out_);
    //set weight
    mac_.param_vector_ = std::make_shared<vector<string>>();
    mac_.SetWeight(weight_);
    psum_.SetWeight(mac_.param_vector_);
    relu_.SetWeight(mac_.param_vector_);
    bn_.SetWeight(mac_.param_vector_);
    pool_.SetWeight(mac_.param_vector_);
    //set registerFile
    rbm_.SetRegFile(reg_file_);
    wbm_.SetRegFile(reg_file_);
    mac_.SetRegFile(reg_file_);
    trim_.SetRegFile(reg_file_);
    psum_.SetRegFile(reg_file_);
    lut_.SetRegFile(reg_file_);
    bn_.SetRegFile(reg_file_);
    relu_.SetRegFile(reg_file_);
    pool_.SetRegFile(reg_file_);
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void NPU::ModuleReorg() {
    if ((*reg_file_)["reg_CONV_MODE"]["AB_order"] == 0){
        rbm_.SetBank(bank_a_, bank_b_);
        wbm_.SetBank(bank_b_, bank_a_);
    } else {
        rbm_.SetBank(bank_b_, bank_a_);
        wbm_.SetBank(bank_a_, bank_b_);
    }

    psum_.SetPMEM(psum_mem_);

    rbm_.SetOutputFM(rbm_out_);
    mac_.SetInputFM(rbm_out_);
    mac_.SetOutputFM(mac_out_);
    // mac_.SetWeight()
    trim_.SetInputFM(mac_out_);
    trim_.SetOutputFM(trim_out_);
    psum_.SetInputFM(trim_out_);
    psum_.SetOutputFM(psum_out_);

    if (((*reg_file_)["NL_MODE"]["act_LUT_en"] == 1) && ((*reg_file_)["NL_MODE"]["relu_en"] == 1)){
        std::cout << "Relu & LUT can not be active together!" << std::endl;
        std::exit(1);
    }

    if ((*reg_file_)["NL_MODE"]["nl_en"] == 1){
        switch ((*reg_file_)["NL_MODE"]["nl_order"])
        {
        case 0:    //bn_relu_pool
            bn_.SetInputFM(psum_out_);
            bn_.SetOutputFM(bn_out_);

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(bn_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(bn_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }

            pool_.SetInputFM(relu_out_);
            pool_.SetOutputFM(pool_out_);
            wbm_.SetInputFM(pool_out_); break;
        case 1:    //bn_pool_relu
            bn_.SetInputFM(psum_out_);
            bn_.SetOutputFM(bn_out_);
            pool_.SetInputFM(bn_out_);
            pool_.SetOutputFM(pool_out_);

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(pool_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(pool_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }

            wbm_.SetInputFM(relu_out_); break;
        case 2:    //relu_bn_pool

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(psum_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(psum_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }
        
            bn_.SetInputFM(relu_out_);
            bn_.SetOutputFM(bn_out_);
            pool_.SetInputFM(bn_out_);
            pool_.SetOutputFM(pool_out_);
            wbm_.SetInputFM(pool_out_); break;
        case 3:    //relu_pool_bn

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(psum_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(psum_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }

            pool_.SetInputFM(relu_out_);
            pool_.SetOutputFM(pool_out_);
            bn_.SetInputFM(pool_out_);
            bn_.SetOutputFM(bn_out_);
            wbm_.SetInputFM(bn_out_); break;
        case 4:    //pool_bn_relu
            pool_.SetInputFM(psum_out_);
            pool_.SetOutputFM(pool_out_);
            bn_.SetInputFM(pool_out_);
            bn_.SetOutputFM(bn_out_);

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(bn_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(bn_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }

            wbm_.SetInputFM(relu_out_); break;
        case 5:    //pool_relu_bn
            pool_.SetInputFM(psum_out_);
            pool_.SetOutputFM(pool_out_);

            if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                relu_.SetInputFM(pool_out_);
                relu_.SetOutputFM(relu_out_);
            }else{
                lut_.SetInputFM(pool_out_);
                lut_.SetOutputFM(lut_out_);
                relu_out_ = lut_out_;
            }

            bn_.SetInputFM(relu_out_);
            bn_.SetOutputFM(bn_out_);
            wbm_.SetInputFM(bn_out_); break;
        default: std::cout << "Error: Unsupported nl order!!"; break;
        }
    } else {
        wbm_.SetInputFM(psum_out_);
    }

}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000 all modes  *
*----------------------------------------------------------------------------*
*****************************************************************************/
void NPU::Run() {
    const vector<Sptr<Command>>& cmd = cmd_.GetCMDList();
    while (program_cnt_ < cmd.size())
    {
        Sptr<Command>cmd_ptr = cmd.at(program_cnt_);
        Opcode opcode = cmd_ptr->GetOpcode();
        std::cout << "[NPU_CMD] pc=" << program_cnt_
                  << ", cmd_idx=" << cmd_ptr->GetIndex()
                  << ", opcode=" << OpcodeToString(opcode)
                  << ", raw=0x" << std::hex << cmd_ptr->GetRawBin() << std::dec
                  << std::endl;
        if (opcode == Opcode::REGWRITE) {    //write registerFile
            Sptr<REGWRITE> reg_write_cmd = CommandList::GetREGWRITECMD(cmd_ptr);
            reg_target_idx_ = (reg_write_cmd->target_index_) - 2;    //"- 2 means register idx start from 2"
            reg_target_value_ = reg_write_cmd->target_value_;
            register_value_.at(reg_target_idx_) = reg_target_value_;
            reg_file_ -> WriteRegisterFile(reg_target_idx_, reg_target_value_);
            program_cnt_++;
        } else if (opcode == Opcode::REGINC) {    //inc registerFile
            Sptr<REGINC> reg_inc_cmd = CommandList::GetREGINCCMD(cmd_ptr);
            reg_target_idx_ = (reg_inc_cmd->target_index_) - 2;      //"- 2 means register idx start from 2"
            reg_target_value_ = reg_inc_cmd->target_value_;
            int sum = register_value_.at(reg_target_idx_) + reg_target_value_;
            register_value_.at(reg_target_idx_) = (sum > 65536) ? sum % 65536 : sum;
            reg_file_ -> WriteRegisterFile(reg_target_idx_, register_value_.at(reg_target_idx_));
            program_cnt_++;
        } else if (opcode == Opcode::CONV) {
            if(program_cnt_ == 75){
                int aaa = 0;
            }
            const int conv_mode = (*reg_file_)["reg_CONV_MODE"]["mode"];
            const int trim = (*reg_file_)["reg_CONV_MODE"]["trim"];
            const int full_ch = (*reg_file_)["reg_CONV_MODE"]["full_ch"];
            const int ch_st = (*reg_file_)["reg_CONV_MODE"]["ch_st"];
            const int upsample = (*reg_file_)["reg_CONV_MODE"]["upsample"];
            const int ich = (*reg_file_)["reg_FM_ICH"]["ich"];
            const int och_st = (*reg_file_)["reg_FM_OCH_ST"]["och_st"];
            const int och_ed = (*reg_file_)["reg_FM_OCH_ED"]["och_ed"];
            const int row = (*reg_file_)["reg_FM_ROW"]["row"];
            const int col = (*reg_file_)["reg_FM_COL"]["col"];
            const int nl_en = (*reg_file_)["NL_MODE"]["nl_en"];
            const int nl_order = (*reg_file_)["NL_MODE"]["nl_order"];
            const int relu_en = (*reg_file_)["NL_MODE"]["relu_en"];
            const int lut_en = (*reg_file_)["NL_MODE"]["act_LUT_en"];
            std::cout << "[NPU_CONV] pc=" << program_cnt_
                      << ", mode=" << ConvModeToString(conv_mode) << "(" << conv_mode << ")"
                      << ", trim=" << trim
                      << ", full_ch=" << full_ch
                      << ", ch_st=" << ch_st
                      << ", ich=" << ich
                      << ", och_st=" << och_st
                      << ", och_ed=" << och_ed
                      << ", row=" << row
                      << ", col=" << col
                      << ", upsample=" << upsample
                      << ", nl_en=" << nl_en
                      << ", nl_order=" << NlOrderToString(nl_order) << "(" << nl_order << ")"
                      << ", relu_en=" << relu_en
                      << ", lut_en=" << lut_en
                      << std::endl;
            ModuleReorg();
            rbm_.Run();
            mac_.Run();//cu
            trim_.Run();
            psum_.Run();
            switch ((*reg_file_)["NL_MODE"]["nl_order"])
            {
            case 0:    //bn_relu_pool
                bn_.Run();
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                pool_.Run(); break;
            case 1:    //bn_pool_relu
                bn_.Run();
                pool_.Run();
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                break;
            case 2:    //relu_bn_pool
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                bn_.Run();
                pool_.Run(); break;
            case 3:    //relu_pool_bn
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                pool_.Run();
                bn_.Run(); break;
            case 4:    //pool_bn_relu
                pool_.Run();
                bn_.Run();
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                break;
            case 5:    //pool_relu_bn
                pool_.Run();
                if ((*reg_file_)["NL_MODE"]["relu_en"] == 1){
                    relu_.Run();
                } else {
                    lut_.Run();
                }
                bn_.Run(); break;
            default: std::cout << "Error: Unsupported nl order!!"; break;
            }            

            if (ShouldDumpCurrentNode()) {
                const std::string node_suffix = "node" + std::to_string(CURRENT_NPU_NODE_INDEX)
                                              + "_pc" + std::to_string(program_cnt_);
                if (rbm_out_) {
                    rbm_out_->DumpMatrix("node_input_" + node_suffix);
                }

                Sptr<Tensor<int>> node_output;
                if ((*reg_file_)["NL_MODE"]["nl_en"] == 0) {
                    node_output = psum_out_;
                } else {
                    const int nl_order_out = (*reg_file_)["NL_MODE"]["nl_order"];
                    const bool relu_enabled = ((*reg_file_)["NL_MODE"]["relu_en"] == 1);
                    switch (nl_order_out) {
                        case 0: node_output = pool_out_; break;                          // bn_relu_pool
                        case 1: node_output = relu_enabled ? relu_out_ : lut_out_; break; // bn_pool_relu
                        case 2: node_output = pool_out_; break;                          // relu_bn_pool
                        case 3: node_output = bn_out_; break;                            // relu_pool_bn
                        case 4: node_output = relu_enabled ? relu_out_ : lut_out_; break; // pool_bn_relu
                        case 5: node_output = bn_out_; break;                            // pool_relu_bn
                        default: break;
                    }
                }

                if (node_output) {
                    node_output->DumpMatrix("node_output_" + node_suffix);
                }
            }

            if ((*reg_file_)["reg_CONV_MODE"]["full_ch"]) {
                wbm_.Run();
            }
            program_cnt_++;
        } else if (opcode == Opcode::LOOPCOUNT) {
            Sptr<LOOPCOUNT> loop_count_cmd = CommandList::GetLOOPCOUNTCMD(cmd_ptr);
            loop_count_num_ = loop_count_cmd->loop_cnt_num_;
            program_cnt_++;
        } else if (opcode == Opcode::LOOP) {
            Sptr<LOOP> loop_cmd = CommandList::GetLOOPCMD(cmd_ptr);
            addr_offset_ = loop_cmd->address_offset_;
            if (loop_count_num_ == 0) program_cnt_++;
            else {
                loop_count_num_ -= 1;
                program_cnt_ -= (addr_offset_ - 1);
            }
        } else if (opcode == Opcode::INTR) {
            Sptr<INTR> intr_cmd = CommandList::GetINTRCMD(cmd_ptr);
            intr_source_ = intr_cmd->intr_source_;
            program_cnt_++;
        } else if (opcode == Opcode::SYNC) {
            program_cnt_++;
            return;
        } else if (opcode == Opcode::NOP) {
            program_cnt_++;
        }
    } 
}

NPU::NPU(std::string name){
    name_ = name;
}

void NPU::ResetProgramCnt() {
    program_cnt_ = 0;
}

void NPU::WriteCommand(const vector<uint8_t> & cmd){
    cmd_.Init(cmd);
}

void NPU::WriteWeight(const vector<uint8_t> & weight){
    weight_->Init(weight);
}

void NPU::WriteData(const Tensor<int>& input, int offset_x, int offset_y, int channel_st, EntryFormat format){
    if ((*reg_file_)["reg_CONV_MODE"]["AB_order"] == 0){
        bank_a_->Store(input, offset_x, offset_y, channel_st, format);
    } else {
        bank_b_->Store(input, offset_x, offset_y, channel_st, format);
    }
}

void NPU::Dump(){}