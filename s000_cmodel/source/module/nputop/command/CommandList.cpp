#include "CommandList.h"
#include <iostream>
#include <cstdint>

void CommandList::Init(const vector<uint8_t>& cmd) {
    cmd_list_.clear();
    size_t vector_size = cmd.size();
    uint64_t vector_idx = 0;
    uint16_t cmd_idx = 0;
    while (vector_idx < vector_size)
    {
        Sptr<Command> command;
        switch (cmd[vector_idx] >> 3) {
        case 0 : command = ParseNOPCMD(cmd, vector_idx, cmd_idx); break;
        case 2 : command = ParseSYNCCMD(cmd, vector_idx, cmd_idx); break;
        case 6 : command = ParseINTRCMD(cmd, vector_idx, cmd_idx); break;
        case 8 : command = ParseCONVCMD(cmd, vector_idx, cmd_idx); break;
        case 12: command = ParseLOOPCOUNTCMD(cmd, vector_idx, cmd_idx); break;
        case 11: command = ParseREGWRITECMD(cmd, vector_idx, cmd_idx); break;
        case 13: command = ParseREGINCCMD(cmd, vector_idx, cmd_idx); break;
        case 14: command = ParseLOOPCMD(cmd, vector_idx, cmd_idx); break;
        default: 
            std::cout << "Error: Unsupported command type!! " 
                      << "Type: " << (int)(cmd[vector_idx] >> 3) 
                      << " (0x" << std::hex << (int)(cmd[vector_idx] >> 3) << std::dec << ")"
                      << ", Position: " << vector_idx 
                      << ", Cmd Index: " << cmd_idx 
                      << ", Raw Byte: 0x" << std::hex << (int)cmd[vector_idx] << std::dec 
                      << std::endl; 
            break;
        }
        cmd_list_.emplace_back(command);
//        cmd_to_ptr_[cmd_idx] = command;
        cmd_idx += 1;
    }
}

Sptr<Command> CommandList::ParseNOPCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<NOP> nop_cmd = std::make_shared<NOP>(cmd_idx, binary);
    nop_cmd->loop_count_  = (int) binary;
    vector_idx += 4;
    Sptr<Command> command(nop_cmd);
    return command;
}


Sptr<Command> CommandList::ParseSYNCCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<SYNC> sync_cmd = std::make_shared<SYNC>(cmd_idx, binary);
    vector_idx += 4;
    Sptr<Command> command(sync_cmd);
    return command;
}

Sptr<Command> CommandList::ParseINTRCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<INTR> intr_cmd = std::make_shared<INTR>(cmd_idx, binary);
    intr_cmd->intr_source_ = ((int) binary) - 12288;   //"12288 is INTR Register's head"
    vector_idx += 4;
    Sptr<Command> command(intr_cmd);
    return command;
}

Sptr<Command> CommandList::ParseCONVCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<CONV> conv_cmd = std::make_shared<CONV>(cmd_idx, binary);
    vector_idx += 4;
    Sptr<Command> command(conv_cmd);
    return command;
}

Sptr<Command> CommandList::ParseLOOPCOUNTCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<LOOPCOUNT> loop_cnt_cmd = std::make_shared<LOOPCOUNT>(cmd_idx, binary);
    loop_cnt_cmd->loop_cnt_num_ = ((int) binary) - 24576;    //"24576 is LOOPCOUNT Register's head"
    vector_idx += 4;
    Sptr<Command> command(loop_cnt_cmd);
    return command;
}

Sptr<Command> CommandList::ParseLOOPCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 8) + (uint32_t)cmd[vector_idx+1];
    Sptr<LOOP> loop_cmd = std::make_shared<LOOP>(cmd_idx, binary);
    loop_cmd->address_offset_ = ((int) binary) - 28672;     //"28672 is LOOP Register's head"
    vector_idx += 4;
    Sptr<Command> command(loop_cmd);
    return command;
}

Sptr<Command> CommandList::ParseREGWRITECMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    uint16_t target_index = 0;
    int target_value = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 24) + (((uint32_t)cmd[vector_idx+1]) << 16) + (((uint32_t)cmd[vector_idx+2]) << 8) + (uint32_t)cmd[vector_idx+3];
    target_index = (((uint16_t)cmd[vector_idx]) << 8) + (uint16_t)cmd[vector_idx+1] - 22528;    //"22428 is REGWRITE Register's head"
    target_value = (((int)cmd[vector_idx+2]) << 8) + (int)cmd[vector_idx+3];
    Sptr<REGWRITE> reg_write_cmd = std::make_shared<REGWRITE>(cmd_idx, binary);
    reg_write_cmd->target_index_ = target_index;
    reg_write_cmd->target_value_ = target_value;
    vector_idx += 4;
    Sptr<Command> command(reg_write_cmd);
    return command;
}

Sptr<Command> CommandList::ParseREGINCCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx){
    uint32_t binary = 0;
    uint16_t target_index = 0;
    int target_value = 0;
    binary = (((uint32_t)cmd[vector_idx]) << 24) + (((uint32_t)cmd[vector_idx+1]) << 16) + (((uint32_t)cmd[vector_idx+2]) << 8) + (uint32_t)cmd[vector_idx+3];
    target_index = (((uint16_t)cmd[vector_idx]) << 8) + (uint16_t)cmd[vector_idx+1] - 26624;     //"26624 is REGINC Register's head"
    target_value = (((int)cmd[vector_idx+2]) << 8) + (int)cmd[vector_idx+3];
    Sptr<REGINC> reg_inc_cmd = std::make_shared<REGINC>(cmd_idx, binary);
    reg_inc_cmd->target_index_ = target_index;
    reg_inc_cmd->target_value_ = target_value;
    vector_idx += 4;
    Sptr<Command> command(reg_inc_cmd);
    return command;
}


Sptr<NOP> CommandList::GetNOPCMD(const Sptr<Command>& command) {
    // Sptr<NOP> nop_command(dynamic_cast<NOP*>(command.get()));
    Sptr<NOP> nop_command = std::dynamic_pointer_cast<NOP>(command);
    return nop_command;
}

Sptr<SYNC> CommandList::GetSYNCCMD(const Sptr<Command>& command) {
    Sptr<SYNC> sync_command = std::dynamic_pointer_cast<SYNC>(command);
    return sync_command;
}

Sptr<INTR> CommandList::GetINTRCMD(const Sptr<Command>& command) {
    Sptr<INTR> intr_command = std::dynamic_pointer_cast<INTR>(command);
    return intr_command;
}

Sptr<CONV> CommandList::GetCONVCMD(const Sptr<Command>& command) {
    Sptr<CONV> conv_command = std::dynamic_pointer_cast<CONV>(command);
    return conv_command;
}

Sptr<LOOPCOUNT> CommandList::GetLOOPCOUNTCMD(const Sptr<Command>& command) {
    Sptr<LOOPCOUNT> loop_count_command = std::dynamic_pointer_cast<LOOPCOUNT>(command);
    return loop_count_command;
}

Sptr<LOOP> CommandList::GetLOOPCMD(const Sptr<Command>& command) {
    Sptr<LOOP> loop_command = std::dynamic_pointer_cast<LOOP>(command);
    return loop_command;
}

Sptr<REGWRITE> CommandList::GetREGWRITECMD(const Sptr<Command>& command) {
    Sptr<REGWRITE> regwrite_command = std::dynamic_pointer_cast<REGWRITE>(command);
    return regwrite_command;
}

Sptr<REGINC> CommandList::GetREGINCCMD(const Sptr<Command>& command) {
    Sptr<REGINC> reginc_command = std::dynamic_pointer_cast<REGINC>(command);
    return reginc_command;
}
