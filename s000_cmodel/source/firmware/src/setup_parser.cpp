//
// Created by yiyu on 12/4/19.
//

#include "setup_parser.h"

#include <fstream>

const char* SetupOutOfRange::what() const noexcept { return "setup bin has no entry"; }

SetupParser::SetupParser() : input_pos(0), builder(1024) {}

void SetupParser::Load(const char* input_path) {
    std::ifstream file;
    file.open(input_path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("setup bin not found");
    }
    input.resize(file.tellg() / sizeof(uint32_t));

    file.clear();
    file.seekg(0, std::ios::beg);
    file >> std::noskipws;
    file.read(reinterpret_cast<char*>(input.data()), input.size() * sizeof(uint32_t));
    file.close();
}

void SetupParser::Save(const char* output) {
    std::ofstream file;
    file.open(output, std::ios::binary);
    file.write(reinterpret_cast<char*>(builder.GetBufferPointer()), builder.GetSize());
    file.close();
}

uint8_t* SetupParser::Get() { return builder.GetBufferPointer(); }

uint32_t SetupParser::Advance() {
    if (input_pos >= input.size()) {
        throw SetupOutOfRange();
    }
    uint32_t tmp = input[input_pos];
    input_pos++;
    return tmp;
}

void SetupParser::Advance(uint32_t n) {
    input_pos += n;
    if (input_pos > input.size()) {
        throw SetupOutOfRange();
    }
}

float SetupParser::AdvanceFloat() {
    auto scale = Advance();
    return *reinterpret_cast<float*>(&scale);
}
