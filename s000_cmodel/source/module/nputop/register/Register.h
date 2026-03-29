#ifndef NPU_REGISTER_H
#define NPU_REGISTER_H
#include <string>
#include <map>

class Register {
private:
    int index_;
    std::string name_;
    std::map<std::string, int> fields_;

public:
    Register();
    Register(int idx, std::string name);
    // Copy constructor
    Register(const Register & other);
    // Destructor
    virtual ~Register();
    // Assignment operator
    Register & operator=(const Register & other);

    virtual void SetIndex(int idx) {index_ = idx;};
    virtual int GetIndex() {return index_;};

    int operator[](std::string field);
    virtual Register & AddFields(std::string field_name, int value);
    virtual Register & SetFields(std::string field_name, int value);
    virtual int GetFields(std::string field_name);
};

#endif //NPU_REGISTER_H
