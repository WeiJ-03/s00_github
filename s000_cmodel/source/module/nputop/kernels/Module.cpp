#include "Module.h"
#include <string>

Module::Module(std::string name){
    name_ = name;
}

//DumpMode not define
// void Module::SetDumpMode(DumpMode mode){
//     dump_mode_ = mode;
// }

void Module::SetDumpPrefix(std::string prefix){
    dump_ = prefix;
}

void Module::SetRegFile(const Sptr<RegisterFile> RegisterFile){
    reg_file_ = RegisterFile;
}