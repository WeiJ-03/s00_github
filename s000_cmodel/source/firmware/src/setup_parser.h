//
// Created by yiyu on 12/4/19.
//

#ifndef BTF_SETUP_PARSER_H
#define BTF_SETUP_PARSER_H

#include "setup_generated.h"

class SetupOutOfRange : std::exception {
    const char* what() const noexcept override;
};

class SetupParser {
   public:
    SetupParser();
    virtual ~SetupParser() = default;
    void Load(const char* input_path);
    void Save(const char* output);

    virtual void Parse() = 0;
    uint8_t* Get();

   protected:
    uint32_t Advance();
    void Advance(uint32_t n);
    float AdvanceFloat();

   protected:
    // file reader
    std::vector<uint32_t> input;
    uint64_t input_pos;

    // flat buffer builder
    flatbuffers::FlatBufferBuilder builder;

    // node vector
    std::vector<flatbuffers::Offset<setup::Node>> nodes;
};

#endif  // BTF_SETUP_PARSER_H
