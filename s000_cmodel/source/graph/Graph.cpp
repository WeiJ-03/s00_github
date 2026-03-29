#include <iostream>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include "Graph.h"

//第一次读 node_index
// 第二次读 node_type
// 如果不是 DataNode，再读 next_node_idx
// 然后再按 node_type 进入 CPU/NPU/Data 的子解析函数继续读

Graph::Graph(vector<uint8_t> const& opflow) {
    assert(opflow.size() % 4 == 0);
    auto num_of_words = opflow.size() / 4;
    decltype(opflow.size()) idx = 0;
    while (idx < opflow.size()) {
        int node_index = GetOneWordValue(opflow, idx);
        int node_type = GetOneWordValue(opflow, idx);
        int next_node_idx = (node_type != 2) ? GetOneWordValue(opflow, idx) : -1;
        Sptr<Node> node;
        switch (node_type) {
            case 0: node = ParseCPUNode(opflow, idx); break;
            case 1: node = ParseNPUNode(opflow, idx); break;
            case 2: node = ParseDataNode(opflow, idx); break;
            default: std::cout << "Error: Unsupported node type!!" << std::endl; exit(1);
        }
        node->index_ = node_index;
        node->next_node_idx_ = next_node_idx;
        node_list_.push_back(node);
        node_idx_to_ptr_[node_index] = node;
    }
}

int Graph::GetOneWordValue(vector<uint8_t> const& opflow, unsigned long& idx) {
    int value = 0;
    for (int j = 0; j < 4; j++) {
        value += ((int) opflow[idx++]) << 8*(3-j);
    }
    return value;
}

Sptr<Node> Graph::ParseCPUNode(vector<uint8_t> const& opflow, unsigned long& idx) {
    Sptr<Node> node;
    int cpu_node_type = GetOneWordValue(opflow, idx);

    switch (cpu_node_type) {
        case 0: node = ParseCPUOpCDMA(opflow, idx); break;
        case 1: node = ParseCPUOpWDMA(opflow, idx); break;
        case 2: node = ParseCPUOpDIDMA(opflow, idx); break;
        case 3: node = ParseCPUOpDODMA(opflow, idx); break;
        case 4: node = ParseCPUOpConcat(opflow, idx); break;
        case 5: node = ParseCPUOpPDMA(opflow, idx); break;
        case 6: node = ParseCPUOpCPUShift(opflow, idx); break;
        case 7: node = ParseCPUOpLUTDMA(opflow, idx); break;
        case 8: node = ParseCPUOpL2N(opflow, idx); break;
        case 9: node = ParseCPUOpReshape(opflow, idx); break;
        case 10: node = ParseCPUOpSoftmax(opflow, idx); break;
        default: throw std::runtime_error("Unsupported CPU node type");
    }
    return node;
}

Sptr<Node> Graph::ParseCPUOpCDMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuCDMANode* cpu_cdma_node = new CpuCDMANode();
    cpu_cdma_node->start_addr_ = GetOneWordValue(opflow, idx);
    cpu_cdma_node->len_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_cdma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpWDMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuWDMANode* cpu_wdma_node = new CpuWDMANode();
    cpu_wdma_node->start_addr_ = GetOneWordValue(opflow, idx);
    cpu_wdma_node->len_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_wdma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpDIDMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuDIDMANode* cpu_didma_node = new CpuDIDMANode();
    cpu_didma_node->input_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->offset_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->row_pitch_len_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->row_pitch_num_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->row_len_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->ch_pitch_len_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->ch_pitch_num_ = GetOneWordValue(opflow, idx);
    int sram_offset = GetOneWordValue(opflow, idx); // [14:4] = offset_x, [3:0] == offset_y
    cpu_didma_node->sram_offset_x_ = sram_offset / 16;
    cpu_didma_node->sram_offset_y_ = sram_offset % 16;
    cpu_didma_node->dmem_ = GetOneWordValue(opflow, idx);
    cpu_didma_node->doAdd = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_didma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpDODMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuDODMANode* cpu_dodma_node = new CpuDODMANode();
    cpu_dodma_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->offset_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->row_pitch_len_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->row_pitch_num_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->row_len_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->ch_pitch_len_ = GetOneWordValue(opflow, idx);
    cpu_dodma_node->ch_pitch_num_ = GetOneWordValue(opflow, idx);
    int sram_offset = GetOneWordValue(opflow, idx); // [14:4] = offset_x, [3:0] == offset_y
    cpu_dodma_node->sram_offset_x_ = sram_offset / 16;
    cpu_dodma_node->sram_offset_y_ = sram_offset % 16;
    cpu_dodma_node->smem_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_dodma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpReshape(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuReshapeNode* cpu_reshape_node = new CpuReshapeNode();
    cpu_reshape_node->input_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_reshape_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_reshape_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpSoftmax(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuSoftmaxNode* cpu_softmax_node = new CpuSoftmaxNode();
    cpu_softmax_node->input_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_softmax_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_softmax_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpConcat(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuConcatNode* cpu_concat_node = new CpuConcatNode();
    cpu_concat_node->axis_ = GetOneWordValue(opflow, idx);
    cpu_concat_node->num_of_input_nodes_ = GetOneWordValue(opflow, idx);
    int input_node = 0;
    while (input_node < cpu_concat_node->num_of_input_nodes_) {
        cpu_concat_node->input_nodes_idx_.emplace_back(GetOneWordValue(opflow, idx));
        input_node++;
    }
    cpu_concat_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_concat_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpPDMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuPDMANode* cpu_pdma_node = new CpuPDMANode();
    cpu_pdma_node->start_addr_ = GetOneWordValue(opflow, idx);
    cpu_pdma_node->len_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_pdma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpCPUShift(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuCPUShiftNode* cpu_cpushift_node = new CpuCPUShiftNode();
    cpu_cpushift_node->input_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_cpushift_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_cpushift_node->shift_en_ = GetOneWordValue(opflow, idx);
    cpu_cpushift_node->start_addr_ = GetOneWordValue(opflow, idx);
    cpu_cpushift_node->len_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_cpushift_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpLUTDMA(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuLUTDMANode* cpu_lutdma_node = new CpuLUTDMANode();
    cpu_lutdma_node->lut_start_addr_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_lutdma_node);
    return node;
}

Sptr<Node> Graph::ParseCPUOpL2N(vector<uint8_t> const& opflow, unsigned long& idx) {
    CpuL2NNode* cpu_l2n_node = new CpuL2NNode();
    cpu_l2n_node->input_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_l2n_node->output_node_idx_ = GetOneWordValue(opflow, idx);
    cpu_l2n_node->start_addr_ = GetOneWordValue(opflow, idx);
    Sptr<Node> node(cpu_l2n_node);
    return node;
}

Sptr<Node> Graph::ParseNPUNode(vector<uint8_t> const& opflow, unsigned long& idx) {
    NPUNode* npu_node = new NPUNode();
    Sptr<Node> node(npu_node);
    return node;
}

Sptr<Node> Graph::ParseDataNode(vector<uint8_t> const& opflow, unsigned long& idx) {
    DataNode* data_node = new DataNode();
    data_node->addr_ = GetOneWordValue(opflow, idx);
    data_node->format_ = GetOneWordValue(opflow, idx);
    data_node->height_ = GetOneWordValue(opflow, idx);
    data_node->width_ = GetOneWordValue(opflow, idx);
    data_node->channel_ = GetOneWordValue(opflow, idx);
    if (data_node->format_ == 3) {
        data_node->chunk_number_ = GetOneWordValue(opflow, idx);
        for (int i = 0; i < data_node->chunk_number_; ++i)
            data_node->chunk_start_.emplace_back(GetOneWordValue(opflow, idx));
    }
    Sptr<Node> node(data_node);
    return node;
}

void Graph::PrintGraph() {
    std::cout << "------------------------           PRINTING GRAPH            ------------------------" << std::endl;
    std::cout << "----------      Number of nodes in graph: " << node_list_.size() << "      ----------" << std::endl;
    std::cout << std::endl;
    for (Sptr<Node> node_ptr: node_list_) {
        node_ptr->PrintNode();
    }
}


Sptr<CpuCDMANode> Graph::GetCPUCDMANode(const Sptr<Node>& node) {
    Sptr<CpuCDMANode> cpu_cdma_node = std::dynamic_pointer_cast<CpuCDMANode>(node);
    return cpu_cdma_node;
}
Sptr<CpuWDMANode> Graph::GetCPUWDMANode(const Sptr<Node>& node) {
    Sptr<CpuWDMANode> cpu_wdma_node = std::dynamic_pointer_cast<CpuWDMANode>(node);
    return cpu_wdma_node;
}

Sptr<CpuDIDMANode> Graph::GetCPUDIDMANode(const Sptr<Node>& node) {
    Sptr<CpuDIDMANode> cpu_didma_node = std::dynamic_pointer_cast<CpuDIDMANode>(node);
    return cpu_didma_node;
}

Sptr<CpuDODMANode> Graph::GetCPUDODMANode(const Sptr<Node>& node) {
    Sptr<CpuDODMANode> cpu_dodma_node = std::dynamic_pointer_cast<CpuDODMANode>(node);
    return cpu_dodma_node;
}

Sptr<CpuConcatNode> Graph::GetCPUConcatNode(const Sptr<Node>& node) {
    Sptr<CpuConcatNode> cpu_concat_node = std::dynamic_pointer_cast<CpuConcatNode>(node);
    return cpu_concat_node;
}

Sptr<CpuPDMANode> Graph::GetCPUPDMANode(const Sptr<Node>& node) {
    Sptr<CpuPDMANode> cpu_pdma_node = std::dynamic_pointer_cast<CpuPDMANode>(node);
    return cpu_pdma_node;
}

Sptr<CpuReshapeNode> Graph::GetCPUReshapeNode(const Sptr<Node>& node) {
    Sptr<CpuReshapeNode> cpu_reshape_node = std::dynamic_pointer_cast<CpuReshapeNode>(node);
    return cpu_reshape_node;
}

Sptr<CpuSoftmaxNode> Graph::GetCPUSoftmaxNode(const Sptr<Node>& node) {
    Sptr<CpuSoftmaxNode> cpu_softmax_node = std::dynamic_pointer_cast<CpuSoftmaxNode>(node);
    return cpu_softmax_node;
}

Sptr<DataNode> Graph::GetDataNode(const Sptr<Node>& node) {
//    Sptr<DataNode> data_node( dynamic_cast<DataNode*>(node.get()) );
    Sptr<DataNode> data_node = std::dynamic_pointer_cast<DataNode>(node);
    return data_node;
}

Sptr<NPUNode> Graph::GetNPUNode(const Sptr<Node>& node) {
    Sptr<NPUNode> npu_node = std::dynamic_pointer_cast<NPUNode>(node);
    return npu_node;
}

Sptr<CpuCPUShiftNode> Graph::GetCPUCPUShiftNode(const Sptr<Node>& node) {
    Sptr<CpuCPUShiftNode> cpu_cpushift_node = std::dynamic_pointer_cast<CpuCPUShiftNode>(node);
    return cpu_cpushift_node;
}

Sptr<CpuLUTDMANode> Graph::GetCPULUTDMANode(const Sptr<Node>& node) {
    Sptr<CpuLUTDMANode> cpu_lutdma_node = std::dynamic_pointer_cast<CpuLUTDMANode>(node);
    return cpu_lutdma_node;
}

Sptr<CpuL2NNode> Graph::GetCPUL2NNode(const Sptr<Node>& node) {
    Sptr<CpuL2NNode> cpu_l2n_node = std::dynamic_pointer_cast<CpuL2NNode>(node);
    return cpu_l2n_node;
}