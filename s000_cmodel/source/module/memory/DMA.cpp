#include "DMA.h"
#include "Memory.h"
#include <cassert>
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

#define     HALF_16BIT      32767
#define     HALF_8BIT       127

DMA::DMA(DRAM& dram, NPU& npu): dram_(dram), npu_(npu) {}


void DMA::CDMA(Sptr<CpuCDMANode> const& cpu_cdma_node) {
    vector<uint8_t> cmd_buffer = dram_.Load(cpu_cdma_node->start_addr_, cpu_cdma_node->len_);
    if (cpu_cdma_node->len_ > 4096){
        std::cout << "cmd sram overflow" << std::endl;
        std::cout << "cdma length is " << cpu_cdma_node->len_ << std::endl;
        exit(1);
    }
    npu_.WriteCommand(cmd_buffer);
}


void DMA::WDMA(Sptr<CpuWDMANode> const& cpu_wdma_node) {
    vector<uint8_t> wt_buffer = dram_.Load(cpu_wdma_node->start_addr_, cpu_wdma_node->len_);
    // ofstream ofs("01.txt");
    // for (int i = 0;i<(int)wt_buffer.size();i++){
    //     int out = wt_buffer[i];
    //     ofs << out << endl;
    // }
    // ofs.close();
    npu_.WriteWeight(wt_buffer);
}


void DMA::FromDramToTensorHelper(Sptr<CpuDIDMANode> const& didma, int start_addr, Tensor<int>& t, EntryFormat format, Sptr<DataNode> const& data_node) {
    int real_start_addr = start_addr + didma->offset_;
    int row_start_addr = 0;

    int col = 0, ch_num = 0;

    col = didma->row_len_ / ENTRY_BYTE;
    ch_num = ICH_PER_GROUP;

    for (int chg = 0; chg < didma->ch_pitch_num_; chg++) {
        row_start_addr = real_start_addr + chg * didma->ch_pitch_len_;
        for (int r = 0; r < didma->row_pitch_num_; r++) {
            vector<uint8_t> one_row = dram_.Load(row_start_addr, didma->row_len_);
            int idx = 0;
            for (int c = 0; c < col; c++) {
                for (int i = 0; i < ch_num; i++) {
                    uint8_t tmp = one_row[idx++];
                    int value = (int) tmp;
                    int real_ch = i;
                    t(chg * ch_num + real_ch, 0, r, c) = value;
                }
            }
            row_start_addr += didma->row_pitch_len_;
        } // End of finishing all row pitches
    } // End of finishing all ch pitches
    
}

/*****************************************************************************                                                                          *
*  @function    DMA::DIDMA()                                                 *
*  @author      Ning                                                         *                                                                          *
*----------------------------------------------------------------------------*
*  2023/06/08 |            | Rui          | Modified EntryFormat format and do Add*
*----------------------------------------------------------------------------*
*****************************************************************************/
void DMA::DIDMA(Sptr<CpuDIDMANode> const& cpu_didma_node, Sptr<DataNode> const& data_node, vector<int> didma_shift_param) {
    // 1. Establish result Tensor and parameters
    EntryFormat format = EntryFormat::COL1_CH32;
    int off_x = cpu_didma_node->sram_offset_x_;
    int off_y = cpu_didma_node->sram_offset_y_;
    int channel = 0;
    int row = cpu_didma_node->row_pitch_num_;
    int col = 0;
    assert(cpu_didma_node->row_len_ % ENTRY_BYTE == 0); // Each entry has 64 bits = 8 bytes

    if (data_node->format_ == 0) {
        format = EntryFormat::COL1_CH32;
        channel = cpu_didma_node->ch_pitch_num_ * ICH_PER_GROUP; // Each entry has 64 bits = 8 bytes = 1 column x 8 ch
        col = cpu_didma_node->row_len_ / ENTRY_BYTE;      // Each entry has 64 bits = 8 bytes = 1 column x 8 ch
    } else {
        std::cout << "DIDMA Data Node format Error!" << std::endl;
        std::exit(1);        
    }
    Tensor<int> result(channel, 1, row, col);

    // 2. Fetch data from DRAM and organize it into Tensor format
    FromDramToTensorHelper(cpu_didma_node, data_node->addr_, result, format, data_node);

    // 3. Deliver Tensor data to NPU, NPU should take Tensor format: if doAdd = 1, do addition
    if (cpu_didma_node->doAdd == 1) {
        std::cout << "doAdd must be 0 , can not be 1!" << std::endl;
        std::exit(1);
    } else if (cpu_didma_node->doAdd == 2) {
        std::cout << "doAdd must be 0 , can not be 2!" << std::endl;
        std::exit(1);
    }

    for (int ch = 0; ch<channel; ch++){
        for (int r = 0; r < row ; r++){
            for (int c = 0; c < col; c++){
                int tmp = result(ch,0,r,c);
                if (tmp>127) tmp = -(256-tmp);
                result.Set(ch,0,r,c,tmp);
            }
        }
    }

    result.DumpMatrix("DIDMA_input");
    if (cpu_didma_node->dmem_ == 0) npu_.bank_a_->Store(result, off_x, off_y, 0, format);
    else npu_.bank_b_->Store(result, off_x, off_y, 0, format);
}

void DMA::FromTensorToDramHelper(Sptr<CpuDODMANode> const& dodma, int start_addr, Tensor<int>& t, EntryFormat format, Sptr<DataNode> const& data_node) {
    int real_start_addr = start_addr + dodma->offset_;
    int row_start_addr = 0;
    vector<uint8_t> one_row(dodma->row_len_);

    int col = 0, ch_num = 0;
    col = dodma->row_len_ / 32;
    ch_num = 32;

    for (int chg = 0; chg < dodma->ch_pitch_num_; chg++) {
        row_start_addr = real_start_addr + chg * dodma->ch_pitch_len_;
        for (int r = 0; r < dodma->row_pitch_num_; r++) {
            int idx = 0;
            for (int c = 0; c < col; c++) {
                for (int i = 0; i < ch_num; i++) {
                    int a = t(chg * ch_num + i, 0, r, c);
                    if (a<0) a = 256 + a;
                    uint8_t value = (uint8_t) a;
                    one_row[idx++] = value; 
                }
            }
            dram_.Store(row_start_addr, one_row);
            row_start_addr += dodma->row_pitch_len_;
        } // End of finishing all row pitches
    } // End of finishing all ch pitches
    
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void DMA::DODMA(Sptr<CpuDODMANode> const& cpu_dodma_node, Sptr<DataNode> const& data_node) {
    // 1. Establish result Tensor and parameters
    EntryFormat format = EntryFormat::COL1_CH32;
    int off_x = cpu_dodma_node->sram_offset_x_;
    int off_y = cpu_dodma_node->sram_offset_y_;
    int ch = 0;
    int row = cpu_dodma_node->row_pitch_num_;
    int col = 0;
    assert(cpu_dodma_node->row_len_ % ENTRY_BYTE == 0); // Each entry has 256 bis = 32 bytes

    if (data_node->format_ == 0) {
        format = EntryFormat::COL1_CH32;
        ch = cpu_dodma_node->ch_pitch_num_ * ICH_PER_GROUP; // Each entry has 256 bis = 32 bytes = 1 column x 32 ch
        col = cpu_dodma_node->row_len_ / ENTRY_BYTE;     // Each entry has 256 bis = 32 bytes = 1 column x 32 ch
    } else {
        std::cerr << "data_node->format_ should be 0!" << std::endl;
        std::exit(1);
    }

    // 2. Fetch data from SRAM
    Tensor<int> result;
    if (cpu_dodma_node->smem_ == 0) result = npu_.bank_a_->Load(off_x, off_y, ch, 0, row, col, format);
    else result = npu_.bank_b_->Load(off_x, off_y, ch, 0, row, col, format);
    FromTensorToDramHelper(cpu_dodma_node, data_node->addr_, result, format, data_node);
}


/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/15 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void DMA::LUTDMA(Sptr<CpuLUTDMANode> const& cpu_lutdma_node) {
    int lut_start_addr = cpu_lutdma_node->lut_start_addr_;
    int dma_len = 256 * 16;
    Tensor<int> result(1, 1, 1, 256); // lut SRAM size
    // Fetch data from DRAM and organize it into Tensor format
    FromDramToLUTHelper(lut_start_addr, dma_len, result);
    npu_.lut_mem_->Store(result, 0, 0, 0, EntryFormat::COL1_CH32);
}


void DMA::FromDramToLUTHelper(int start_addr, int dma_len, Tensor<int>& t) {
    vector<uint8_t> one_row = dram_.Load(start_addr, dma_len);
    int idx = 15;
    for (int j = 0; j < 256; j++) {
        int value = (int) one_row[idx];
        if (value >= 128){
            value = value - 256;
        }
        t.Set(0, 0, 0, j, value);
        idx = idx + 16;
    }
}


void DMA::L2Normalization(const Sptr<CpuL2NNode> &cpu_l2n_node, const Sptr<DataNode> &input_node, const Sptr<DataNode> &output_node) {
    int weight_start_addr = cpu_l2n_node->start_addr_;
    int channel = (input_node->channel_ + 7) / 8 * 8; int row = input_node->height_; int col = input_node->width_; int ch_num = 8;
    int ch_pitch_num = channel / ch_num;
    // Get L2N weight
    vector<uint8_t> weight = dram_.Load(weight_start_addr, channel * 8);// len = channel *8 ??
    vector<float> L2N_parameter;
    vector<int> x_radix; vector<int> y_radix; int idx = 0;
    for (int c = 0; c < channel; ++c) { // get L2N_parameter
        uint32_t p = (uint32_t) weight[idx++];
        for (int i = 0; i < 3; ++i)
            p = (p << 8) + (uint32_t) weight[idx++];
        L2N_parameter.emplace_back(reinterpret_cast<float &> (p));
    }
    for (int c = 0; c < channel; ++c) { // get x_radix
        int x = (int) weight[idx++];
        // x = x * (int) pow(2, 8) + (int) weight[idx++];
        if (x > HALF_8BIT) x = -(pow(2, 8) - x);
        x_radix.emplace_back(x);
    }
    for (int c = 0; c < channel; ++c) { // get y_radix
        int y = (int) weight[idx++];
        // y = y * (int) pow(2, 8) + (int) weight[idx++];
        if (y > HALF_8BIT) y = -(pow(2, 8) - y);
        y_radix.emplace_back(y);
    }
    // do L2N
    vector<uint8_t> input_v = dram_.Load(input_node->addr_, channel * row * col * 2);
    vector<uint8_t> output_v;
    Tensor<float> input_t(channel, 1, row, col);
    Tensor<int> output_t(channel, 1, row, col);
    idx = 0;
    if (input_node->format_ == 1) {
        for (int chg = 0; chg < ch_pitch_num; chg++) {
            for (int r = 0; r < row; r++) {
                for (int c = 0; c < col; c++) {
                    for (int i = 0; i < ch_num; i++) {
                        int curr_ch = chg * ch_num + i;
                        // int16_t hi = input_v[idx++];
                        // int16_t lo = input_v[idx++];
                        // int16_t a = (hi << 8u) + lo;
                        uint8_t tmp = input_v[idx++];
                        int value = (int) tmp;
                        float temp_res = (float) value * (float) pow(2, -x_radix.at(curr_ch));
                        input_t(curr_ch, 0, r, c) = temp_res;
                    }
                }
            }
        }
    } else if (input_node->format_ == 3) {
        int chunk_num = input_node->chunk_number_;
        vector<int> chunk_start = input_node->chunk_start_;
        for (int cn = 0; cn < chunk_num; ++cn) {
            int row_st = chunk_start[cn];
            int row_ed = (cn == chunk_num - 1) ? row - 1 : chunk_start[cn + 1];
            int row_real = row_ed - row_st + 1;
            int block_num = ch_pitch_num * row_real; int block_col = block_num / 8; int block_mod = block_num % 8;
            for (int bn = 0; bn < block_num; ++bn) {
                int x_idx = 0; int y_idx = 0;
                if (bn < (block_col + 1) * block_mod) {
                    y_idx = bn / (block_col + 1); x_idx = bn % (block_col + 1);
                } else {
                    y_idx = (bn - (block_col + 1) * block_mod) / block_col + block_mod;
                    x_idx = (bn - (block_col + 1) * block_mod) % block_col;
                }
                int ch_idx = (x_idx * 8 + y_idx) / row_real;
                int curr_row = (x_idx * 8 + y_idx) % row_real + row_st;
                for (int c = 0; c < col; ++c) {
                    for (int ch = 0; ch < ch_num; ++ch) {
                        int curr_ch = ch_idx * 8 + ch;
                        uint8_t a = input_v[idx++];
                        // int curr_ch = ch_idx * 32 + ch;
                        // int16_t hi = input_v[idx++];
                        // int16_t lo = input_v[idx++];
                        // int16_t a = (hi << 8u) + lo;
                        int value = (int) a;
                        float temp_res = (float) value * (float) pow(2, -x_radix.at(curr_ch));
                        input_t(curr_ch, 0, curr_row, c) = temp_res;
                    }
                }
            }
        }
    } else {
        throw "Error: Invalid Format for L2N";
    }
    Tensor<float> norm_data(1, 1, row, col);
    for (int r = 0; r < row; ++r) {
        for (int c = 0; c < col; ++c) {
            float nd = 0;
            for (int ch = 0; ch < channel; ++ch) {
                nd += input_t(ch, 0, r, c) * input_t(ch, 0, r, c);
            }
            norm_data(0, 0, r, c) = std::sqrt(nd);
        }
    }
    for (int ch = 0; ch < channel; ++ch) {
        for (int r = 0; r < row; ++r) {
            for (int c = 0; c < col; ++c) {
                float l2_norm_output = input_t(ch, 0, r, c) / norm_data(0, 0, r, c) * L2N_parameter.at(ch);
                int int_temp_res = (int) std::round(l2_norm_output * (float) pow(2, y_radix.at(ch)));
                output_t(ch, 0, r, c) = int_temp_res;
            }
        }
    }
    if (input_node->format_ == 1) {
        for (int chg = 0; chg < ch_pitch_num; chg++) {
            for (int r = 0; r < row; r++) {
                for (int c = 0; c < col; c++) {
                    for (int i = 0; i < ch_num; i++) {
                        int curr_ch = chg * ch_num + i;
                        uint8_t value = (uint8_t) output_t(curr_ch, 0, r, c);
                        // uint8_t hi = (uint8_t) (value / ((int) pow(2,8)));
                        // uint8_t lo = (uint8_t) (value % ((int) pow(2,8)));
                        output_v[idx++] = value; //output_v[idx++] = lo;
                    }
                }
            }
        }
    } else {
        int chunk_num = input_node->chunk_number_;
        vector<int> chunk_start = input_node->chunk_start_;
        for (int cn = 0; cn < chunk_num; ++cn) {
            std::cout << "Current chunk number:" << cn << std::endl;
            int row_st = chunk_start[cn];
            int row_ed = (cn == chunk_num - 1) ? row - 1 : chunk_start[cn + 1];
            int row_real = row_ed - row_st + 1;
            int block_num = ch_pitch_num * row_real; int block_col = block_num / 16; int block_mod = block_num % 8;
            for (int bn = 0; bn < block_num; ++bn) {
                int x_idx = 0; int y_idx = 0;
                if (bn < (block_col + 1) * block_mod) {
                    y_idx = bn / (block_col + 1); x_idx = bn % (block_col + 1);
                } else {
                    y_idx = (bn - (block_col + 1) * block_mod) / block_col + block_mod;
                    x_idx = (bn - (block_col + 1) * block_mod) % block_col;
                }
                int ch_idx = (x_idx * 8 + y_idx) / row_real;
                int curr_row = (x_idx * 8 + y_idx) % row_real + row_st;
                for (int c = 0; c < col; ++c) {
                    for (int ch = 0; ch < ch_num; ++ch) {
                        int curr_ch = ch_idx * 4 + ch; //int curr_ch = ch_idx * 32 + ch;
                        uint8_t value = (uint8_t) output_t(curr_ch, 0, curr_row, c);
                        output_v.emplace_back(value);
                    }
                }
            }
        }
    }
    output_t.DumpSeq("L2N");
    dram_.Store(output_node->addr_, output_v);
}