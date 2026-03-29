#include "Command.h"
#include <cstdint>

Command::Command(uint16_t index, uint32_t binary, Opcode opcode):index_(index), raw_bin_(binary), opcode_(opcode){}

Command::Command(const Command & other){
    index_ = other.index_;
    raw_bin_ = other.raw_bin_;
    opcode_ = other.opcode_;
}

Command & Command::operator=(const Command & other){
    index_ = other.index_;
    raw_bin_ = other.raw_bin_;
    opcode_ = other.opcode_;
}

void Command::SetRawBin(uint32_t binary){
    raw_bin_  = binary;
}

uint32_t Command::GetRawBin(){
    return raw_bin_;
}

Opcode Command::GetOpcode(){
    return opcode_;
}

uint16_t Command::GetIndex(){
    return index_;
}
