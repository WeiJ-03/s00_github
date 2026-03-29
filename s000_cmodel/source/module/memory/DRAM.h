#ifndef N900_MODEL_DRAM_H
#define N900_MODEL_DRAM_H

#include <vector>
#include "Define.h"

struct DramSegment {
    int start_addr_ = 0;
    int len_ = 0;
    std::vector<uint8_t> buffer_;
};

class DRAM {
private:
    std::vector<Sptr<DramSegment>> segments_;
//    bool OverlapCheck(int start_addr, int len);
    static const int DRAM_MAX = 1610612736;
public:
    DRAM() = default;
    ~DRAM() = default;
    DRAM(const DRAM&& dram) = delete;
    DRAM& operator=(const DRAM& dram) = delete;
    void Allocate(int start_addr, int len);
    void Store(int star_addr, std::vector<uint8_t> const& data);
    std::vector<uint8_t> Load(int star_addr, int len);
};

#endif //N900_MODEL_DRAM_H

