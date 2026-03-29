#ifndef NPU_MODULE_H
#define NPU_MODULE_H
#include <string>
#include "RegisterFile.h"
#include "Define.h"

class Module {
protected:
    std::string name_;
    // DumpMode dump_mode_;
    std::string dump_;
    Sptr<RegisterFile> reg_file_;
    
public:
    Module() = default;
    Module(std::string name);
    virtual ~Module() = default;
    void virtual Dump() = 0;
    void virtual Run() = 0;
    void virtual CleanUp() = 0;
    // void SetDumpMode(DumpMode mode);
    void SetDumpPrefix(std::string prefix);
    void SetRegFile(const Sptr<RegisterFile> registerFile);
};

#endif