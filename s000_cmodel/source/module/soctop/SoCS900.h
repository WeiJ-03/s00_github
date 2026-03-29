#ifndef N900_MODEL_SOCN900_H
#define N900_MODEL_SOCN900_H

#include "Config.h"
#include "DRAM.h"
#include "DMA.h"
#include "NPU.h"
#include "Define.h"
#include <set>
#include <map>
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h" // or "../stdout_sinks.h" if no colors needed
#include "spdlog/sinks/basic_file_sink.h"

class Graph;

class SoCN900 {
private:
    static const int ICH_PER_GROUP   = 32;
    static const int cmd_start_addr  = 1073741824; //4000_0000
    static const int wt_start_addr   = 1075838976; //5000_0000
    static const int data_start_addr = 1342177280; //4100_0000

private:
    //Sptr<DRAM> dram_{nullptr};
    //Sptr<NPU> npu_{nullptr};
    //Sptr<DMA> dma_{nullptr};
    DRAM dram_;
    NPU npu_;
    DMA dma_;
    Sptr<Graph> graph_{nullptr};
    Config config_;
    std::set<int> nodes_allocated_;
    vector<int> didma_shift_param_;
    std::map<int, string> json_;
    Sptr<spdlog::logger> logger_;
    string  golden_dir_;
    

private:
    Tensor<int> GetTensor(std::vector<uint8_t> const& data, int channel, int row, int col);
    /** CPU Computation Operation **/
    void Reshape(Sptr<CpuReshapeNode> node, Sptr<DataNode> input_node, Sptr<DataNode> output_node);
    void Softmax(Sptr<CpuSoftmaxNode> node, Sptr<DataNode> input_node, Sptr<DataNode> output_node);
    void Concat(vector<Sptr<DataNode>> input_nodes, Sptr<DataNode> output_node);
    bool DRAMAllocationCheck(const Sptr<DataNode>& data_node);
    void DataNodecomparison(Sptr<CpuDODMANode> node, Sptr<DataNode> output_node);
    Tensor<int> DataNodeToTensor(Sptr<DataNode> data_node);
    void DumpOutputDataNode(Sptr<DataNode> data_node);
    void DumpOpflow(vector<Sptr<Node>>& node_list);

public:
    void Init(Config& config, Sptr<Graph> graph, std::vector<std::vector<uint8_t>> const& inputs, std::vector<uint8_t> const& cmd, std::vector<uint8_t> const& weight, std::map<int, string> const& json, string golden_address, vector<uint8_t> const& lut);
    void Init(Config& config, std::vector<std::vector<uint8_t>> const& inputs, std::vector<uint8_t> const& cmd, std::vector<uint8_t> const& weight, std::vector<uint8_t> const& lut);
    void Run();

    SoCN900();
    ~SoCN900() = default;
};

#endif //N900_MODEL_SOCN900_H
