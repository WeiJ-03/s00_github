/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/10/5 |            | Ning           | Modified EntryFormat            *
*----------------------------------------------------------------------------*
*****************************************************************************/
#ifndef NPU_MEMORY_H
#define NPU_MEMORY_H
#include "../../../utils/src/Tensor.h"
#include <string>
#include <vector>

enum class EntryFormat{
    COL1_CH32
};

class Memory{
private:
    Tensor<int> mem_;

public:
    int entry_width_;
    int entry_depth_;
    int line_;

    Memory() = default;
    Memory(int entry_width, int line, int entry_depth);
    virtual ~Memory() = default;

    void Reset();
    void Resize(int ch, int p, int r, int c);
    Tensor<int> GetTensor();
    void DumpMemory(std::string name);

    Tensor<int> Load(int offset_x, int offset_y, int channel, int channel_st, int row, int col, EntryFormat format);
    Tensor<int> RBMLoad(int offset_x, int offset_y, int channel, int channel_st, int row, int col, EntryFormat format);
    void Store(const Tensor<int> & input, int offset_x, int offset_y, int channel_st, EntryFormat format);
};

#endif