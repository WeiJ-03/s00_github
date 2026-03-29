#include "Register.h"

Register::Register(): index_(0), name_("") {

}

Register::Register(int idx, std::string name): index_(idx), name_(name) {

}

Register::Register(const Register & other) {
    index_ = other.index_;
    name_ = other.name_;
    fields_ = other.fields_; // Invoke std::map's assignment operator
}

Register::~Register() {

}

Register & Register::operator=(const Register & other) {
    if (this == &other) {
        return *this;
    }

    index_ = other.index_;
    name_ = other.name_;
    fields_ = other.fields_; // Invoke std::map's assignment operator

    return *this;
}

int Register::operator[](std::string field_name) {
    if (fields_.find(field_name) != fields_.end()) {
        return fields_[field_name];
    } else {
        throw "Register: FieldNotExist";
    }
}

Register & Register::AddFields(std::string field_name, int value) {
    if (fields_.find(field_name) == fields_.end()) {
        fields_.insert(std::pair<std::string, int>(field_name, value));
    }
    return *this;
}

Register & Register::SetFields(std::string field_name, int value) {
    if (fields_.find(field_name) != fields_.end()) {
        fields_[field_name] = value;
    } else {
        throw "Register: FieldNotExist";
    }
    return *this;
}

int Register::GetFields(std::string field_name) {
    int result;
    if (fields_.find(field_name) != fields_.end()) {
        result = fields_[field_name];
    } else {
        return -1; // "Register: FieldNotExist";
    }
    return result;
}

