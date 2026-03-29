//
// Created by yiyu on 12/4/19.
// I think in 720 V1, the setup bin cannot set multiple inputs correctly
//

#ifndef BTF_SETUP_720_V1_H
#define BTF_SETUP_720_V1_H

#include "setup_parser.h"

class Setup720V1 : public SetupParser {
   public:
    void Parse() override;
    ~Setup720V1() override = default;

   private:
    void ParseNode();
    flatbuffers::Offset<setup::IOOptions> ParseDataNode();
    uint32_t ParseSuperNode();
};

#endif  // BTF_SETUP_720_V1_H
