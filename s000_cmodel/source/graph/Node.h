#ifndef N900_MODEL_NODE_H
#define N900_MODEL_NODE_H

#include <iostream>
#include <vector>

enum class OpcodeID {
    CPU_CDMA,
    CPU_WDMA,
    CPU_DIDMA,
    CPU_DODMA,
    CPU_CONCAT,
    CPU_PDMA,
    CPU_CPUSHIFT,
    CPU_LUTDMA,
    CPU_L2N,
    CPU_RESHAPE,
    CPU_SOFTMAX,
    NPU_OP,   // Reserved for NPU
    NOP       // Reserved for data node
};

enum class NodeType {
   DATA_NODE,
   CPU_NODE,
   NPU_NODE
};

struct Node {
    int index_;
    NodeType node_type_;
    OpcodeID opcodeID_;
    int next_node_idx_ = 0;
    Node(NodeType node_type, OpcodeID opcodeID): node_type_(node_type), opcodeID_(opcodeID) {}
    virtual ~Node() = default;
    virtual void PrintNode() {};
};


struct CpuCDMANode : public Node {
    CpuCDMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_CDMA) {}
    int start_addr_ = 0;
    int len_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU CDMA Node " << std::endl;
        std::cout << "       Start Address = " << start_addr_ << std::endl;
        std::cout << "       Length = " << len_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuWDMANode : public Node {
    CpuWDMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_WDMA) {}
    int start_addr_ = 0;
    int len_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU WDMA Node " << std::endl;
        std::cout << "       Start Address = " << start_addr_ << std::endl;
        std::cout << "       Length = " << len_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuDIDMANode : public Node {
    CpuDIDMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_DIDMA) {}
    int input_node_idx_ = 0;
    int offset_ = 0;
    int row_pitch_len_ = 0;
    int row_pitch_num_ = 0;
    int row_len_ = 0;
    int ch_pitch_len_ = 0;
    int ch_pitch_num_ = 0;
    int sram_offset_x_ = 0;
    int sram_offset_y_ = 0;
    int dmem_ = 0;  // 0: BankA, 1: BankB
    int doAdd = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU DIDMA Node " << std::endl;
        std::cout << "       Input Node Index = " << input_node_idx_ << std::endl;
        std::cout << "       Offset = " << offset_ << std::endl;
        std::cout << "       Row Pitch Length = " << row_pitch_len_ << std::endl;
        std::cout << "       Row Pitch Number = " << row_pitch_num_ << std::endl;
        std::cout << "       Row Length = " << row_len_ << std::endl;
        std::cout << "       Channel Pitch Length = " << ch_pitch_len_ << std::endl;
        std::cout << "       Channel Pitch Number = " << ch_pitch_num_ << std::endl;
        std::cout << "       SRAM Offset: x = " << sram_offset_x_ << "  y = " << sram_offset_y_ << std::endl;
        std::cout << "       Destination Memory = " << dmem_ << std::endl;
        std::cout << "       Do Addition Flag = " << doAdd << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuDODMANode : public Node {
    CpuDODMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_DODMA) {}
    int output_node_idx_ = 0;
    int offset_ = 0;
    int row_pitch_len_ = 0;
    int row_pitch_num_ = 0;
    int row_len_ = 0;
    int ch_pitch_len_ = 0;
    int ch_pitch_num_ = 0;
    int sram_offset_x_ = 0;
    int sram_offset_y_ = 0;
    int smem_ = 0; // 0: BankA, 1: BankB

    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU DODMA Node " << std::endl;
        std::cout << "       Input Node Index = " << output_node_idx_ << std::endl;
        std::cout << "       Offset = " << offset_ << std::endl;
        std::cout << "       Row Pitch Length = " << row_pitch_len_ << std::endl;
        std::cout << "       Row Pitch Number = " << row_pitch_num_ << std::endl;
        std::cout << "       Row Length = " << row_len_ << std::endl;
        std::cout << "       Channel Pitch Length = " << ch_pitch_len_ << std::endl;
        std::cout << "       Channel Pitch Number = " << ch_pitch_num_ << std::endl;
        std::cout << "       SRAM Offset: x = " << sram_offset_x_ << "  y = " << sram_offset_y_ << std::endl;
        std::cout << "       Source Memory = " << smem_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuReshapeNode : public Node {
    CpuReshapeNode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_RESHAPE) {}
    int input_node_idx_ = 0;
    int output_node_idx_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU Reshape Node " << std::endl;
        std::cout << "       Input Data Node Index = " << input_node_idx_ << std::endl;
        std::cout << "       Output Data Node Index = " << output_node_idx_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuSoftmaxNode : public Node {
    CpuSoftmaxNode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_SOFTMAX) {}
    int input_node_idx_ = 0;
    int output_node_idx_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU Softmax Node " << std::endl;
        std::cout << "       Input Data Node Index = " << input_node_idx_ << std::endl;
        std::cout << "       Output Data Node Index = " << output_node_idx_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuConcatNode : public Node {
    CpuConcatNode() : Node(NodeType::CPU_NODE, OpcodeID::CPU_CONCAT) {}
    int axis_ = 0;   //0: channel, 1: row, 2: col
    int num_of_input_nodes_ = 0;
    std::vector<int> input_nodes_idx_;
    int output_node_idx_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU Concatenation Node " << std::endl;
        std::cout << "   Concatenation axis is " << axis_ << std::endl;
        for (int i = 0; i < num_of_input_nodes_; ++i) {
            std::cout << "       Input Data Nodes Index = " << input_nodes_idx_.at(i) << std::endl;
        }
        std::cout << "       Output Data Node Index = " << output_node_idx_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuPDMANode : public Node {
    CpuPDMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_PDMA) {}
    int start_addr_ = 0;
    int len_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU CDMA Node " << std::endl;
        std::cout << "       Start Address = " << start_addr_ << std::endl;
        std::cout << "       Length = " << len_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuCPUShiftNode : public Node {
    CpuCPUShiftNode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_CPUSHIFT) {}
    int input_node_idx_ = 0;
    int output_node_idx_ = 0;
    int shift_en_ = 0;
    int start_addr_ = 0;
    int len_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU LUTDMA Node " << std::endl;
        std::cout << "       Input Node Idx_ = " << input_node_idx_ << std::endl;
        std::cout << "       Output Node Idx_ = " << output_node_idx_ << std::endl;
        std::cout << "       Shift EN = " << shift_en_ << std::endl;
        std::cout << "       Start Addr = " << start_addr_ << std::endl;
        std::cout << "       Lenth = " << len_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuLUTDMANode : public Node {
    CpuLUTDMANode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_LUTDMA) {}
    int lut_start_addr_ = 0;
    // int dma_len_ = 0;
//    int dma_num_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU LUTDMA Node " << std::endl;
        std::cout << "       LUT Start Address = " << lut_start_addr_ << std::endl;
        // std::cout << "       DMA Length = " << dma_len_ << std::endl;
//        std::cout << "       DMA NUM = " << dma_num_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct CpuL2NNode : public Node {
    CpuL2NNode(): Node(NodeType::CPU_NODE, OpcodeID::CPU_L2N) {}
    int input_node_idx_ = 0;
    int output_node_idx_ = 0;
    int start_addr_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   CPU L2N Node " << std::endl;
        std::cout << "       L2N Start Address = " << start_addr_ << std::endl;
        std::cout << "       Input Node Idx_ = " << input_node_idx_ << std::endl;
        std::cout << "       Output Node Idx_ = " << output_node_idx_ << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct NPUNode : public Node {
    NPUNode(): Node(NodeType::NPU_NODE, OpcodeID::NPU_OP) {}
    int dummy_ = 0;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   NPU Node " << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

struct DataNode : public Node {
    DataNode(): Node(NodeType::DATA_NODE, OpcodeID::NOP) {}
    int addr_ = 0;

    // 0: Entry format 2 col x 4ch, used for CONV3x3RGBA
    // 1: Entry format 1 col x 8ch, used for other modes
    // 2: Sequential format
    int format_ = 0;

    int height_ = 0;
    int width_ = 0;
    int channel_ = 0;
    int chunk_number_;
    std::vector<int> chunk_start_;
    void PrintNode() override {
        std::cout << "------------  Node: " << index_ << "   ------------" << std::endl;
        std::cout << "   Data Node " << std::endl;
        std::cout << "       Address = " << addr_ << std::endl;
        std::cout << "       Format = " << format_ << std::endl;
        std::cout << "       Height = " << height_ << std::endl;
        std::cout << "       Width = " << width_ << std::endl;
        std::cout << "       Channel = " << channel_ << std::endl;
        std::cout << "       Chunk_number_ = " << chunk_number_ << std::endl;
        for (auto num : chunk_start_)
            std::cout << "       Chunk_start_ = " << num << std::endl;
        std::cout << "       Next Node Index = " << next_node_idx_ << std::endl;
        std::cout << "---------------------------------------------------" << std::endl;
        std::cout << std::endl;
    };
};

#endif //N900_MODEL_NODE_H
