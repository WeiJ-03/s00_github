#ifndef NPU_WEIGHT_H
#define NPU_WEIGHT_H
// #include "Tensor.h"
#include "Define.h"
#include "RegisterFile.h"
#include "Tensor.h"
#include <vector>
#include <deque>
#include <string>
using std::vector;
using std::deque;

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/10/8 |  1.0       | Ning           | WEIGHT_WIDTH 128->64            *
*  2022/10/8 |  1.0       | Ning           | add WEIGHT_WIDTH_byte           *
*----------------------------------------------------------------------------*
*****************************************************************************/
class Weight {
private:
    static const int WEIGHT_FIFO_IN = 128;
    static const int AXI_WIDTH = 16;
    static const int BLOCK_BIT_NUM = 64;

    
    static const int WEIGHT_PER_BLOCK = 5;   //one block has 5 128bits, and gets 64 10bits numbers
    static const int WEIGHT_PER_FREAM = 211;  //one block has 211 128bits, 211 = 5(block) * 42(1 frame has 42 blocks) + 1(header)
    static const int UINT8_PER_FRAME = 3376;  //FRAME * WIDTH / 8
    static const int NUM_PER_FEAME  = 14;   //(211 - 1) / 5 / 3

    deque<std::string> slice_128bit_;
    vector<std::string> header_;
    int header_idx_ = 64;
    bool Caculate_header_size(std::string header);
public:
    Sptr<Tensor<int>> weight_;   //1, 1, depth, 256/8(BYTE)
    Weight();
    Weight(const vector<uint8_t> & wt);
    Weight(const Tensor<int> & weight);

    void Init(const vector<uint8_t> & wt);
    static int bin_to_int(std::string weight);
    static int bin_to_uint(std::string weight);
};

#endif //NPU_WEIGHT_H
