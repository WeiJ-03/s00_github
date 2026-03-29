#ifndef BTF_SETUP_520_V2_H
#define BTF_SETUP_520_V2_H

#include "setup_parser.h"

class Setup520V2 : public SetupParser {
   public:
    void Parse() override;
    ~Setup520V2() override = default;

   private:
    void ParseNode();
    flatbuffers::Offset<setup::IOOptions> ParseDataNode();
    uint32_t ParseSuperNode();
};

#endif  // BTF_SETUP_720_V1_H
