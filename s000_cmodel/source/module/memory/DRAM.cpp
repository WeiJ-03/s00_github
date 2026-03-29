#include "DRAM.h"
#include <algorithm>
#include <iostream>
#include <stdexcept>
using std::vector;

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/06/03 |  1.0      | Rui           |  all not change || data format change no matter with dram 
*----------------------------------------------------------------------------*
*****************************************************************************/


void DRAM::Allocate(int start_addr, int len) {
    Sptr<DramSegment> seg(new DramSegment());
    auto it_0 = segments_.begin();
    while (it_0 != segments_.end()) {
        if ((*it_0)->start_addr_ + (*it_0)->len_ - 1 < start_addr || start_addr + len - 1 < (*it_0)->start_addr_) {
            ++it_0;
            continue;
        } else {
            if ((*it_0)->start_addr_ < start_addr) {
                len += (start_addr - (*it_0)->start_addr_);
                start_addr = (*it_0)->start_addr_;
            }
            if ((*it_0)->start_addr_ + (*it_0)->len_ - 1 > start_addr + len - 1) {
//                len += ((*it_0)->start_addr_ + (*it_0)->len_ - 1 - (start_addr + len - 1));
                len += ((*it_0)->start_addr_ + (*it_0)->len_ - (start_addr + len));
            }
            ++it_0;
        }
    }
    seg->start_addr_ = start_addr;
    seg->len_ = len;
    seg->buffer_.resize(len, 0);
    auto it_1 = segments_.begin();
    while (it_1 != segments_.end()) {
        if ((*it_1)->start_addr_ + (*it_1)->len_ - 1 < start_addr || start_addr + len - 1 < (*it_1)->start_addr_) {
            ++it_1;
            continue;
        } else {
            for (int i = 0; i < (*it_1)->len_; i++) {
                seg->buffer_[(*it_1)->start_addr_ - seg->start_addr_ + i] = (*it_1)->buffer_[i];
            }
            it_1 = segments_.erase(it_1);    //delete seg
        }
    }
    segments_.push_back(seg);

    std::sort(segments_.begin(), segments_.end(), [](Sptr<DramSegment> const& s1, Sptr<DramSegment> const& s2) -> bool {
        return s1->start_addr_ < s2->start_addr_;
    });   //[capture list(empyt)](parameter list) -> return type {function body}
}

void DRAM::Store(int start_addr, vector<uint8_t> const& data) {
    int len = (int) data.size();
    if (start_addr+len >= DRAM_MAX){
        std::cout << "DRAM overflow" << std::endl;
        std::cout << "start address is " << start_addr << std::endl;
        std::cout << "Length is " << len << std::endl;
        std::cout << "DRAM max address is 0x5000_0000" << std::endl;
        throw std::runtime_error("DRAM overflow");
    }
    auto it = std::find_if(segments_.begin(), segments_.end(), [&](const Sptr<DramSegment>& seg){
        return start_addr >= seg->start_addr_ && start_addr + len - 1 <= seg->start_addr_ + seg->len_ - 1;
    });
    if (it == segments_.end()) {
        std::cout << "DRAM store to unallocated range" << std::endl;
        std::cout << "start address is " << start_addr << std::endl;
        std::cout << "Length is " << len << std::endl;
        throw std::runtime_error("DRAM store out of allocated segments");
    }

    auto& seg = *it;
    for (int i = 0; i < len; i++) {
        seg->buffer_[start_addr - seg->start_addr_ + i] = data[i];
    }
}

vector<uint8_t> DRAM::Load(int start_addr, int len) {
    vector<uint8_t> buffer(len, 0);
    auto it = std::find_if(segments_.begin(), segments_.end(), [&](const Sptr<DramSegment>& seg){
        return start_addr >= seg->start_addr_ && start_addr + len - 1 <= seg->start_addr_ + seg->len_ - 1;
    });
    if (it == segments_.end()) {
        std::cout << "DRAM load from unallocated range" << std::endl;
        std::cout << "start address is " << start_addr << std::endl;
        std::cout << "Length is " << len << std::endl;
        throw std::runtime_error("DRAM load out of allocated segments");
    }

    auto& seg = *it;
    for (int i = 0; i < len; i++) {
        buffer[i] = seg->buffer_[start_addr - seg->start_addr_ + i];
    }
    return buffer;
}


