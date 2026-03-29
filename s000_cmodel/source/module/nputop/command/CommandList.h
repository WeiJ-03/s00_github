#ifndef N900_MODEL_COMMANDList_H
#define N900_MODEL_COMMANDList_H

// #include <cstdint>
#include "Command.h"
#include "Define.h"
#include <unordered_map>
#include <vector>
using std::vector;
using std::unordered_map;

class CommandList{
private:
    vector<Sptr<Command>> cmd_list_;
//    unordered_map<int, Sptr<Command>> cmd_to_ptr_;

    Sptr<Command> ParseNOPCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseSYNCCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseINTRCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseCONVCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseLOOPCOUNTCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseLOOPCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseREGWRITECMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);
    Sptr<Command> ParseREGINCCMD(const vector<uint8_t>& cmd, uint64_t& vector_idx, const uint16_t& cmd_idx);

public:
    CommandList() = default;
    ~CommandList() = default;
    void Init(const vector<uint8_t> &);   //Replace constructor

    vector<Sptr<Command>>& GetCMDList() { return cmd_list_; };

public:
    static Sptr<NOP> GetNOPCMD(const Sptr<Command>&);
    static Sptr<SYNC> GetSYNCCMD(const Sptr<Command>&);
    static Sptr<INTR> GetINTRCMD(const Sptr<Command>&);
    static Sptr<CONV> GetCONVCMD(const Sptr<Command>&);
    static Sptr<LOOPCOUNT> GetLOOPCOUNTCMD(const Sptr<Command>&);
    static Sptr<LOOP> GetLOOPCMD(const Sptr<Command>&);
    static Sptr<REGWRITE> GetREGWRITECMD(const Sptr<Command>&);
    static Sptr<REGINC> GetREGINCCMD(const Sptr<Command>&);
};

#endif //N900_MODEL_COMMAND_H
