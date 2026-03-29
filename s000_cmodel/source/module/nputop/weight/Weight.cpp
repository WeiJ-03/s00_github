#include "Weight.h"
#include <utility>
#include <cstdint>
#include <string>
#include <cmath>
#include <bitset>
#include <cassert>
#include <iostream>
#include <fstream>

using std::string;
using std::bitset;
using std::ofstream;
using std::endl;
using std::dec;

int Weight::bin_to_int(string weight){
    int dec = 0;
    int res = 0;
    // for (auto i = 0; i < weight.size(); i++){
    //     dec = (weight[i] == '1') ? dec + pow(2, weight.size()-1-i) : dec;
    // }
    int len = (int)weight.size();
    for (auto i = 0; i < len; i++){
        dec = (weight[i] == '1') ? dec + pow(2, len-i-1) : dec;
    }
    if (weight[0] == '0') res = dec;
    else res = -(pow(2, weight.size()) - dec);
    return res;
}

int Weight::bin_to_uint(string weight){
    int dec = 0;
    int len = (int)weight.size();
    for (auto i = 0; i < len; i++){
        dec = (weight[i] == '1') ? dec + pow(2, len-1-i) : dec;
    }
    return dec;
}

Weight::Weight() {
    weight_ = std::make_shared<Tensor<int>>();
    header_.resize(16);
}

Weight::Weight(const Tensor<int> & weight) {
    //这里之间copy了一份tensor出来，目的是什么？
    *weight_ = weight;
}

Weight::Weight(const vector<uint8_t> & wt) {
    std::cout << "this weight construction function should not be run, it's the same as Weight::Init,please check more" << std::endl;
    std::exit(1);
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/6/25 |  1.0       | Rui            | firt case pass important change *
*----------------------------------------------------------------------------*
*****************************************************************************/
void Weight::Init(const vector<uint8_t> & wt) {
    assert(wt.size() % AXI_WIDTH == 0);  // The AXI width is 32bit 32/8 = 8byte
    // get 32 bit data and header
    // vector<string> slice_32bit;
    int block_num = (int)(wt.size() / AXI_WIDTH);
    for (int i = 0; i < block_num; i++) {
        string slice;
        slice = "";
        for (int j = 0; j < AXI_WIDTH; j++) {   //get header for first 8 uint_8 per frame
            slice += bitset<8>(wt.at(i * AXI_WIDTH + j)).to_string();
        }
        slice_128bit_.emplace_back(slice);
    }

    // ofstream f1;
    // f1.open( "./dumpfile/weight_init.txt", std::ios::out);
    // for(int i=0; i < wt.size();i++){
    //     string uio;
    //     uio = bitset<8>(wt.at(i)).to_string();
    //     f1 << uio << endl;
    // }
    // f1.close(); 


    // ofstream f;
    // f.open( "./dumpfile/slice_128bit.txt", std::ios::out);
    // for(int i=0; i < slice_128bit_.size();i++){
    //     f << slice_128bit_.at(i)<< endl;
    // }
    // f.close(); 

    // int header_idx = 16;
    // vector<string> header(4);
    vector<vector<string>> data(4, vector<string>(16)); 
    vector<int> w;
    // assert(slice_32bit.size()%2 == 0);
    while(true) {
        if (header_idx_ == 64) {
            for (int j = 0;j<16;j++){
                header_.at(j) = slice_128bit_.at(0).substr(j*8,8);
                if(header_.at(j) != "11111111"){
                    int bbb = 0;
                }
            }
            slice_128bit_.pop_front();
            header_idx_ = 0; 
        }
        int head = bin_to_uint(header_.at(header_idx_/4).substr(6-(header_idx_%4)*2, 2));
        //int head = 3;
        header_idx_ += 1;
        assert(head < 4 && head >= 0);
        if (head == 0) continue;
        for (int h = 0; h <= head; ++h){
            for (int j = 0; j < 16; j++)
                data.at(h).at(j) = slice_128bit_.at(0).substr(j*8,8); 
            slice_128bit_.pop_front();
        } 
        for (int k = BLOCK_BIT_NUM - 1; k >= 0 ; k--) {    
            int result = 0;
            string tmp;
            for (int h = head; h >= 0; h--) {
                tmp += data.at(h).at(k/4).substr((k%4)*2, 2); 
            }
            result = bin_to_int(tmp);
            w.emplace_back(result);
        }
        if (slice_128bit_.size()==0){
            break;
        }
    }

    assert(w.size() % (WEIGHT_FIFO_IN) == 0); //32 * 8 = 256 bit

    vector<int> w_reshape_1(w.size());
    for(int i=0; i < w.size(); i++){
        if(i % 128 <= 63){
            w_reshape_1.at(i) = w.at(i+64);
        }else {
            w_reshape_1.at(i) = w.at(i-64);
        }
    }

    vector<int> w_reshape_2(w.size());
    for(int i=0; i < w.size(); i++){
        w_reshape_2.at(i) = w_reshape_1.at((i/64) * 64 + 63 - i%64);
    }


    weight_->Resize(1, 1, w.size() / (WEIGHT_FIFO_IN), (WEIGHT_FIFO_IN));
    for (int i = 0; i < (int) w.size(); ++i) {
        weight_->Set(0, 0,i / WEIGHT_FIFO_IN, i % WEIGHT_FIFO_IN, w_reshape_2.at(i));
    }
}

bool Weight::Caculate_header_size(string header){
    vector<string> myheader(4);
    int bolck_size = 0;
    for (int j = 0;j<4;j++)
        myheader.at(j) = header.substr(j*8,8);
    for (int i=0;i<16;i++){
        int head = bin_to_uint(myheader.at(i/4).substr(6-(i%4)*2, 2));
        if (head == 3) bolck_size += 4;
        else if (head == 2) bolck_size += 3;
        else if (head == 1) bolck_size += 2;
        else if (head == 0) bolck_size += 0;
        else {
            std::cout << "head must be less than 4!" << std::endl;
            std::exit(1);
        }
    }
    return (slice_128bit_.size()>=bolck_size+1);
}