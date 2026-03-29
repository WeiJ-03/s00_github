#ifndef NPU_REGISTERFILE_H
#define NPU_REGISTERFILE_H
#include "Register.h"
#include <map>

class RegisterFile {
private:
    std::map<std::string, Register> reg_map_;

public:
    RegisterFile();
    ~RegisterFile();


    // Add/Remove register
    void AddReg(std::string reg_name, Register reg);
    void RemoveReg(std::string reg_name);

    // Set register field for a specific register
    Register & operator[](std::string reg_name);
    void SetRegField(std::string reg_name, std::string reg_field, int value);
    int GetRegField(std::string reg_name, std::string reg_field);

    void BuildRegisterFile();
    void WriteRegisterFile(uint16_t target_idx, uint16_t taregt_value);
};

#endif //NPU_REGISTERFILE_H
