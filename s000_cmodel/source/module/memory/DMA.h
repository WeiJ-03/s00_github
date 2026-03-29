#ifndef N900_MODEL_DMA_H
#define N900_MODEL_DMA_H
#include "Graph.h"
#include "NPU.h"
#include "DRAM.h"
#include "Define.h"
#include "spdlog/spdlog.h"

class DMA {
private:
    static const int ICH_PER_GROUP          = 32;

private:
    DRAM& dram_;
    NPU& npu_;
    void FromDramToTensorHelper(Sptr<CpuDIDMANode> const& didma, int start_addr, Tensor<int>& t, EntryFormat format, Sptr<DataNode> const& data_node);
    void FromDramToLUTHelper(int start_addr, int dma_len, Tensor<int>& t);
    void FromTensorToDramHelper(Sptr<CpuDODMANode> const& dodma, int start_addr, Tensor<int>& t, EntryFormat format, Sptr<DataNode> const& data_node);
    Sptr<spdlog::logger> logger_;

public:
    void CDMA(Sptr<CpuCDMANode> const& cpu_cdma_node);
    void WDMA(Sptr<CpuWDMANode> const& cpu_wdma_node);
    void DIDMA(Sptr<CpuDIDMANode> const& cpu_didma_node, Sptr<DataNode> const& data_node, vector<int> didma_shift_param);
    void DODMA(Sptr<CpuDODMANode> const& cpu_dodma_node, Sptr<DataNode> const& data_node);
    void LUTDMA(Sptr<CpuLUTDMANode> const& cpu_lutdma_node);
    void L2Normalization(Sptr<CpuL2NNode> const& cpu_l2n_node, Sptr<DataNode> const& input_node, Sptr<DataNode> const& output_node);
    DMA() = delete;
    DMA(DRAM& dram, NPU& npu);
    ~DMA() = default;
};

#endif //N900_MODEL_DMA_H
