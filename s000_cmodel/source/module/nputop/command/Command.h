#ifndef N900_MODEL_COMMAND_H
#define N900_MODEL_COMMAND_H

#include <cstdint>
#include "Define.h"
enum class Opcode{
    NOP, SYNC, INTR, CONV, LOOPCOUNT, LOOP, REGWRITE, REGINC
};

class Command{
private:
    uint16_t index_;
    uint32_t raw_bin_;
    Opcode opcode_;

public:
    Command() = default;
    Command(uint16_t index, uint32_t binary, Opcode opcode);
    Command(const Command & other);
    virtual ~Command() = default;
    Command & operator=(const Command & other);
    void SetRawBin(uint32_t binary);
    uint32_t GetRawBin();
    Opcode GetOpcode();
    uint16_t GetIndex();
};

class NOP : public Command{
public:
    NOP(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::NOP){}
    int loop_count_;
};

class SYNC : public Command{
public:
    SYNC(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::SYNC){}
};

class INTR : public Command{
public:
    INTR(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::INTR){}
    int intr_source_;
};

class CONV : public Command{
public:
    CONV(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::CONV){}
};

class LOOPCOUNT : public Command{
public:
    LOOPCOUNT(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::LOOPCOUNT){}
    int loop_cnt_num_;
};

class LOOP : public Command{
public:
    LOOP(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::LOOP){}
    int address_offset_;
};

class REGWRITE : public Command{
public:
    REGWRITE(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::REGWRITE){}
    uint16_t target_index_;
    int target_value_;
};

class REGINC : public Command{
public:
    REGINC(uint16_t index, uint32_t binary) : Command(index, binary, Opcode::REGINC){}
    uint16_t target_index_;
    int target_value_;
};

#endif //N900_MODEL_COMMAND_H