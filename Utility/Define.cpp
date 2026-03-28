#include <string>

int EXE_CLK = 0;
double DMA_CLK = 0;
//double DMA_CLK_NEW = 0;
double DMA_635_OLD = 0;
double DMA_635_NEW = 0;
double DMA_SETUP_TIME = 635;
int WT_CLK = 0;
//int WT_FIFO_CLK = 36;     // NPU 600, DMA 500, 10bit weight
//int WT_FIFO_CLK = 30;     // NPU 600, DMA 600, weight 10 bit
int WT_FIFO_CLK = 32;     // NPU 600, DMA 600, weight 10 bit， 70ns delay in 500M
// 1920 bits = 15 * 128 bits, cycle num = 15 * 2 = 30 cycle, 4KB/1920 = 17.066, 17.066 * 30 + (70 / 2) = 547 cycle, 547 -> 512(30 * 17.066), 547 / 17.066 = 32.05
//int WT_FIFO_CLK = 19;     // NPU 600, DMA 600, weight 6 bit， 70ns delay in 500M
// 1152 bits = 9 * 128 bits, cycle num = 9 * 2 = 18 cycle, 4KB/1152 = 28.444, 28.444 * 18 + (70 / 2) = 547 cycle, 547 -> 512(18 * 28.444), 547 / 28.444 = 19.23
//int WT_FIFO_CLK = 7;     // NPU 600, DMA 600, weight 2 bit， 70ns delay in 500M
// 384 bits = 3 * 128 bits, cycle num = 3 * 2 = 6 cycle, 4KB/384 = 85.333, 85.333 * 6 + (70 / 2) = 547 cycle, 547 -> 512(85.333 * 6), 547 / 85.333 = 6.41

//int WT_FIFO_CLK = 9;     // NPU 600, DMA 600, weight 10 bit， 70ns delay in 500M
int MAIN_FREQ = 600000000;
int DMA_FREQ = 600000000;
std::string DUMPFILE_DIR = "./dumpfile";