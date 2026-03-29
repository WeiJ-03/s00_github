#include "Memory.h"
#include <string>
#include <vector>
#include <sstream>
#include <stdexcept>

using std::string;

Memory::Memory(int entry_width, int line, int entry_depth): 
entry_width_(entry_width), entry_depth_(entry_depth), line_(line), mem_(entry_width, 1, line, entry_depth){}

void Memory::Reset(){}

void Memory::Resize(int ch, int p, int r, int c) {}

void Memory::DumpMemory(string name){
    mem_.DumpMatrix(name);
}

Tensor<int> Memory::GetTensor(){
    return mem_;
}

Tensor<int> Memory::RBMLoad(int offset_x, int offset_y, int channel, int channel_st, int row, int col, EntryFormat format) {
    if (format != EntryFormat::COL1_CH32) {
        std::cout << "SRAM Load format Error!" << std::endl;
        std::exit(1); 
    }
    Tensor<int> dout(channel, 1, row, col);
    int overflow_cnt = 0;
    int first_i = -1;
    int first_j = -1;
    int first_k = -1;
    int first_ch = -1;
    int first_r = -1;
    int first_c = -1;
    int first_linear_ch = -1;
    for (int i = 0; i < channel; i++) {
        int linear_ch = i + channel_st;
        int ch = linear_ch % entry_width_;
        for (int j = 0; j < row; j++) {
            int r = ((linear_ch / entry_width_) * row + offset_y + j) % line_;
            for (int k = 0; k < col; k++) {
                int col_addr = (((linear_ch / entry_width_) * row + offset_y + j) / line_) * col + offset_x + k;
                if (col_addr >= entry_depth_){
                    if (overflow_cnt == 0) {
                        first_i = i;
                        first_j = j;
                        first_k = k;
                        first_ch = ch;
                        first_r = r;
                        first_c = col_addr;
                        first_linear_ch = linear_ch;
                    }
                    overflow_cnt++;
                    dout.Set(i, 0, j, k, 0);
                } else {
                    dout.Set(i, 0, j, k, mem_(ch, 0, r, col_addr));
                }
            }
        }
    }
    if (overflow_cnt > 0) {
        std::cout << "####################################################" << std::endl;
        std::cout << "############---RBM Read Overflow---#################" << std::endl;
        std::cout << "RBM params: offset_x=" << offset_x
                  << ", offset_y=" << offset_y
                  << ", channel=" << channel
                  << ", channel_st=" << channel_st
                  << ", row=" << row
                  << ", col=" << col << std::endl;
        std::cout << "Memory spec: entry_width=" << entry_width_
                  << ", line=" << line_
                  << ", entry_depth=" << entry_depth_ << std::endl;
        std::cout << "First overflow at loop(i,j,k)=(" << first_i << "," << first_j << "," << first_k << ")"
                  << ", linear_ch=" << first_linear_ch
                  << ", mapped(ch,r,c)=(" << first_ch << "," << first_r << "," << first_c << ")" << std::endl;
        std::cout << "Total overflow points: " << overflow_cnt << std::endl;
        std::cout << "####################################################" << std::endl;
    }
    return dout;
}

Tensor<int> Memory::Load(int offset_x, int offset_y, int channel, int channel_st, int row, int col, EntryFormat format) {
    if (format != EntryFormat::COL1_CH32) {
        std::cout << "SRAM Load format Error!" << std::endl;
        std::exit(1); 
    }
    Tensor<int> dout(channel, 1, row, col);
    for (int i = 0; i < channel; i++) {
        int ch = (i + channel_st) % entry_width_;
        for (int j = 0; j < row; j++) {
            int r = (((i + channel_st) / entry_width_) * row + offset_y + j) % line_;
            for (int k = 0; k < col; k++) {
                int c = ((((i + channel_st) / entry_width_) * row + offset_y + j) / line_) * col + offset_x + k;
                dout.Set(i, 0, j, k, mem_(ch, 0, r, c));
            }
        }
    }
    return dout;
}

void Memory::Store(const Tensor<int> & input, int offset_x, int offset_y, int channel_st, EntryFormat format) {   //const Tensor<int> & input error
    if (format != EntryFormat::COL1_CH32) {
        std::cout << "SRAM Store format Error!" << std::endl;
        std::exit(1); 
    }
    int row = input.GetRow();
    int col = input.GetCol();
    int channel  = input.GetCh();
    for (int i = channel_st; i < channel_st + channel; i++) {
        int ch = i % entry_width_;    //SRAM channel addr
        for (int j = 0; j < row; j++) {
            int r = ((i / entry_width_) * row + offset_y + j) % line_; //SRAM row addr
            for (int k = 0; k < col; k++) {
                int c = (((i / entry_width_) * row + offset_y + j) / line_) * col + offset_x + k;   //SRAM col addr
                if (c < 0 || c >= entry_depth_) {
                    std::ostringstream oss;
                    oss << "Memory::Store col overflow: c=" << c
                        << " out of [0," << (entry_depth_ - 1) << "]"
                        << ", i=" << i
                        << ", j=" << j
                        << ", k=" << k
                        << ", ch=" << ch
                        << ", r=" << r
                        << ", input_ch=" << channel
                        << ", input_row=" << row
                        << ", input_col=" << col
                        << ", offset_x=" << offset_x
                        << ", offset_y=" << offset_y
                        << ", channel_st=" << channel_st
                        << ", mem_entry_width=" << entry_width_
                        << ", mem_line=" << line_
                        << ", mem_entry_depth=" << entry_depth_;
                    throw std::runtime_error(oss.str());
                }
                mem_.Set(ch, 0, r, c, input.Get(i-channel_st, 0, j, k));
            }
        }
    }

}