#ifndef N900_MODEL_DEFINE_H
#define N900_MODEL_DEFINE_H

#include <memory>
#include <string>


#define     ENTRY_BYTE                  32

#define     IFM_BIT                     8
#define     OFM_BIT                     16

#define     MAC_ROW                     8
#define     MAC_COL                     4

#define     LUT_DEPTH                   256
#define     LUT_CH                      4





template <typename T>
using Sptr = std::shared_ptr<T>;

template <typename T>
using Uptr = std::unique_ptr<T>;

static const int CMD_START_ADDR = 0x10000000; // 0x1000_0000;
static const int WEIGHT_START_ADDR = 0x20000000; // 0x2000_0000;
static const int DATA_START_ADDR = 0x30000000; // 0x3000_0000;

extern int EXE_CLK;
extern double DMA_CLK;
//extern double DMA_CLK_NEW;
extern double DMA_635_OLD;
extern double DMA_635_NEW;
extern double const DMA_SETUP_TIME;
extern int WT_CLK;
extern int WT_FIFO_CLK;
extern int MAIN_FREQ;
extern int DMA_FREQ;
extern std::string DUMPFILE_DIR;
extern int CURRENT_NPU_NODE_INDEX;
extern std::string DUMP_NODE_FILTER;

bool ShouldDumpCurrentNode();

 
#endif //N900_MODEL_DEFINE_H
