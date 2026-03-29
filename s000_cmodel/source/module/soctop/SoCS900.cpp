#include "SoCS900.h"
#include "Define.h"
#include "Graph.h"
#include "FileIO.h"
#include <utility>
#include <vector>
#include <iostream>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <cctype>
#include <fstream>

using std::string;
using std::vector;
using std::ofstream;
using std::endl;
using std::dec;

namespace {
const char* OpcodeIDToString(OpcodeID opcode) {
    switch (opcode) {
        case OpcodeID::CPU_CDMA: return "CPU_CDMA";
        case OpcodeID::CPU_WDMA: return "CPU_WDMA";
        case OpcodeID::CPU_DIDMA: return "CPU_DIDMA";
        case OpcodeID::CPU_DODMA: return "CPU_DODMA";
        case OpcodeID::CPU_CONCAT: return "CPU_CONCAT";
        case OpcodeID::CPU_PDMA: return "CPU_PDMA";
        case OpcodeID::CPU_CPUSHIFT: return "CPU_CPUSHIFT";
        case OpcodeID::CPU_LUTDMA: return "CPU_LUTDMA";
        case OpcodeID::CPU_L2N: return "CPU_L2N";
        case OpcodeID::CPU_RESHAPE: return "CPU_RESHAPE";
        case OpcodeID::CPU_SOFTMAX: return "CPU_SOFTMAX";
        case OpcodeID::NPU_OP: return "NPU_OP";
        case OpcodeID::NOP: return "NOP";
        default: return "UNKNOWN_OPCODE";
    }
}
}



SoCN900::SoCN900(): dma_(dram_, npu_) {
    ///dram_ = std::make_shared<DRAM>();
    ///npu_ = std::make_shared<NPU>();
    ///dma_ = std::make_shared<DMA>(*dram_, *npu_);
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
Tensor<int> SoCN900::GetTensor(const std::vector<uint8_t> &data, int channel, int row, int col) {
    assert(channel * row * col == (int) data.size() / 2);
    Tensor<int> tensor(channel, 1, row, col);

    int idx = 0;
    for (int i = 0; i < channel; i++){
        for (int j = 0; j < row; j++){
            for (int k = 0; k < col; k++){
                tensor.Set(i, 0, j, k, data.at(idx++));
            }
        }
    }
    return tensor;
}

// TODO:: To be defined and implemented
void SoCN900::Reshape(Sptr<CpuReshapeNode> node, Sptr<DataNode> input_node, Sptr<DataNode> output_node) {

}

// TODO:: To be defined and implemented
void SoCN900::Softmax(Sptr<CpuSoftmaxNode> node, Sptr<DataNode> input_node, Sptr<DataNode> output_node) {

}

void SoCN900::Concat(vector<Sptr<DataNode>> input_nodes, Sptr<DataNode> output_node) {
    vector<uint8_t> concat_data;
    for (auto node : input_nodes) {
        int len = (((node->channel_+7)/8)*8) * node->height_ * node->width_ * 1;     //* 2 means each pixel is 16 bits
        vector<uint8_t> tmp = dram_.Load(node->addr_, len);
        concat_data.insert(concat_data.end(), tmp.begin(), tmp.end());
    }
    dram_.Store(output_node->addr_, concat_data);
}

bool SoCN900::DRAMAllocationCheck(const Sptr<DataNode>& data_node) {   //if allocated return true
    return !(nodes_allocated_.find(data_node->index_) == nodes_allocated_.end());
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
Tensor<int> SoCN900::DataNodeToTensor(Sptr<DataNode> data_node) {
    int ch_num = 32;
    int col = data_node -> width_;
    int row = data_node -> height_;

    Tensor<int> result((((data_node->channel_+31)/32)*32), 1, row, col);
    vector<uint8_t> buffer = dram_.Load(data_node->addr_, (((data_node->channel_+31)/32)*32) * row * col * 1);
    std::cout << buffer.size() << std::endl;
    int idx = 0;
    for (int chg = 0; chg < (data_node->channel_+31)/32; chg++) {
        for (int r = 0; r < row; r++) {
            for (int c = 0; c < col; c++) {
                for (int i = 0; i < ch_num; i++){
                    uint8_t a = buffer[idx++];
                    int value = (int) a;
                    result(chg * ch_num + i, 0, r, c) = value;
                }
            }
        }
    }
    return result;
    
}
/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/07/25 |  1.0      | Rui           | print opflow                     *
*----------------------------------------------------------------------------*
*****************************************************************************/
void SoCN900::DumpOpflow(vector<Sptr<Node>>& node_list){
    ofstream f;
    f.open( "./dumpfile/opflow.txt", std::ios::out);
    for (Sptr<Node> node: node_list) {
        if (node->node_type_ == NodeType::DATA_NODE) {
            f << "Data Node"<< endl;
        } else if (node->node_type_ == NodeType::NPU_NODE) {
            f << "NPU Node"<< endl;
        } else if (node->node_type_ == NodeType::CPU_NODE) {
            switch (node->opcodeID_) {
                case OpcodeID::CPU_CDMA: 
                    f << "CDMA Node"<< endl;
                    break;
                case OpcodeID::CPU_WDMA:          
                    f << "WDMA Node"<< endl;
                    break;
                case OpcodeID::CPU_DIDMA: {
                    f << "DIDMA Node"<< endl;
                    break;
                }
                case OpcodeID::CPU_DODMA: {
                    f << "DODMA Node"<< endl;
                    f << "-------------------------------------------"<< endl;
                    break;
                }
                case OpcodeID::CPU_LUTDMA: {
                    f << "LUTDMA Node"<< endl;
                    break;
                }
                default: f << "----Unkonwn Node----"<< endl;
            }
        }
    }
    f.close(); 
}





/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void SoCN900::DataNodecomparison(Sptr<CpuDODMANode> node, Sptr<DataNode> output_node) {
    if (json_.size()) {
        int start_row = node->offset_ / 32 / output_node->width_; 
        if (start_row + node->row_pitch_num_ == output_node->height_) {
            Tensor<int> node_res = DataNodeToTensor(output_node);
            node_res.DumpMatrix("TARGET_MATRIX");

            auto json_it = json_.find(output_node->index_);
            if (json_it == json_.end()) {
                logger_->warn("Skip compare DataNode {}: no golden mapping in json", output_node->index_);
                return;
            }

            string golden_stem = json_it->second;
            string golden_stem_lower = golden_stem;
            std::transform(golden_stem_lower.begin(), golden_stem_lower.end(), golden_stem_lower.begin(),
                           [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            if (golden_stem_lower.find("bypass") != string::npos) {
                logger_->info("Skip compare DataNode {}: bypass output (golden key: {})",
                              output_node->index_, golden_stem);
                return;
            }

            // must be absolute path.
            string golden_name = golden_stem + ".bin";
            string filename = golden_dir_ + golden_name;
            std::ifstream golden_file(filename);
            if (!golden_file.good()) {
                logger_->warn("Skip compare DataNode {}: golden file not found: {}",
                              output_node->index_, filename);
                return;
            }
            logger_->info("Start compare DataNode {} with golden file: {} (path: {})",
                          output_node->index_, golden_name, filename);
            vector<uint8_t> golden = IO::FileIO::LoadBinVector_for_cmd(filename);
            int channel = output_node->channel_;
            int row = output_node->height_;
            int col = output_node->width_;
            Tensor<int> gold_res(1, 1, 1, channel*row*col);
            Tensor<int> compare(1, 1, 1, channel*row*col);
            Tensor<int> target_out(1,1,1, channel*row*col);
            int idx = 0;
            int error_cnt = 0;
            for (int i = 0; i < channel/32; i++) {
                for (int j = 0; j < row; j++) {
                    for (int k = 0; k < col; k++) {
                        for (int ch = 0; ch < 32; ch++){
                            int value = (int) golden[idx];
                            int target = (int)node_res.Get(i*32+ch, 0, j, k);

                            if (value > 127) value = -(256 - value);
                            if (target > 127) target = -(256 - target);
                            gold_res.Set(0, 0, 0, idx, value);
                            target_out.Set(0, 0, 0, idx, target);

                            if (value != target) {
                                logger_->info("[{}] DataNode: {}, channel: {}, row: {}, col: {} is wrong! golden:{} target:{}",idx, output_node->index_, i*32+ch, j, k,value,target);
                                compare.Set(0, 0, 0, idx, 0);
                                error_cnt++;
                            } else { 
                                // logger_->info("[{}] DataNode: {}, channel: {}, row: {}, col: {} is Right! golden:{}",idx, output_node->index_, i*8+real_ch, j, k,value);
                                compare.Set(0, 0, 0, idx, 1);
                            }
                            idx++;
                        }
                    }
                }
            }
            gold_res.DumpMatrix("GOLDEN_MATRIX");

            logger_->info("Comparsion end");
            if(error_cnt == 0){
                logger_->info("--------------------------------------------cmodel no error!--------------------------------------------------------" );              
                ofstream f;
                f.open("./result_single_case.txt", std::ios::out);
                f << dec << 1;
                f.close();
            } else {
                ofstream f;
                f.open("./result_single_case.txt", std::ios::out);
                f << dec << 0;
                f.close();                
            }
        
        }
    }
    else return;
}


void SoCN900::DumpOutputDataNode(Sptr<DataNode> data_node) {
    Tensor<int> result = DataNodeToTensor(data_node);
    string suffix = std::to_string(data_node->index_);
    result.DumpMatrix(suffix);
    result.DumpSeq(suffix);
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/01 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void SoCN900::Init(Config& config, Sptr<Graph> graph, std::vector<std::vector<uint8_t>> const& inputs, vector<uint8_t> const& cmd, vector<uint8_t> const& weight, std::map<int, string> const& json, string golden_address,vector<uint8_t> const& lut) {
    std::cout << "SoCS000 Init: ONNX Model" << std::endl;
    golden_dir_ = golden_address;

    // Set Logger
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_st>();
    console_sink->set_level(spdlog::level::debug);
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_st>("logs/s000_log.txt", true);
    file_sink->set_level(spdlog::level::debug);
    spdlog::sinks_init_list sink_list = { file_sink, console_sink };
    spdlog::logger logger("s000_log", sink_list.begin(), sink_list.end());
    logger.set_level(spdlog::level::debug);
    spdlog::set_default_logger(std::make_shared<spdlog::logger>("s000_log", spdlog::sinks_init_list({console_sink, file_sink})));
    logger_ = spdlog::get("s000_log");

    graph_ = graph;
    config_ = config;
    json_ = json;
    // 1. Initialize all the modules
    npu_.Init();

    // 2. Put cmd, weight into DRAM. Input data will be handled separately
//    dram_.Allocate(Graph::GetCPUCDMANode(graph_->GetNode(config_.cmd_node_idx_))->start_addr_, (int) cmd.size());
//    dram_.Store(Graph::GetCPUCDMANode(graph_->GetNode(config_.cmd_node_idx_))->start_addr_, cmd);
    int cmd_size = (int) cmd.size();
    dram_.Allocate(config_.cmd_node_addr_, (int) cmd.size());
    dram_.Store(config_.cmd_node_addr_, cmd);
   //17028    
    dram_.Allocate(config_.weight_node_addr_, (int) weight.size());
    dram_.Store(config_.weight_node_addr_, weight);

    // if (lut.size() != 0) {
    //     dram_.Allocate((int)1080033280, (int) lut.size());
    //     dram_.Store((int)1080033280, lut);    
    // }
    if (lut.size() != 0) {
        int lut_addr = 1080033280; // default 0x4060_0000
        for (Sptr<Node> node: graph_->GetNodeList()) {
            if (node->node_type_ == NodeType::CPU_NODE && node->opcodeID_ == OpcodeID::CPU_LUTDMA) {
                auto cpu_lutdma_node = Graph::GetCPULUTDMANode(node);
                if (cpu_lutdma_node) {
                    lut_addr = cpu_lutdma_node->lut_start_addr_;
                    break;
                }
            }
        }
        dram_.Allocate(lut_addr, (int) lut.size());
        dram_.Store(lut_addr, lut);
    }


    // if (config_.weight_node_addr_+(int) weight.size()>data_start_addr){
    //     std::cout << "weight in dram overflow" << std::endl;
    //     std::cout << "weight start addr is " << config_.weight_node_addr_ << std::endl;
    //     std::cout << "weight length is " << (int) weight.size() << std::endl;
    //     exit(1);
    // }

    // 3. Allocate and Store all the input data nodes
    vector<int> data_nodes_idx = config_.idata_nodes_idx_;
    int inputs_idx = 0;
    for (int idx : config_.idata_nodes_idx_) {
        Sptr<DataNode> data_node = Graph::GetDataNode(graph_->GetNode(idx));
        int height = data_node->height_;
        int width = data_node->width_;
        int channel = data_node->channel_;
        //Allocate
        dram_.Allocate(data_node->addr_, height * width * (((channel+ICH_PER_GROUP-1) / ICH_PER_GROUP) * ICH_PER_GROUP) * 1);  //channel group up number to 32
        nodes_allocated_.insert(idx);
      
        dram_.Store(data_node->addr_, inputs.at(inputs_idx));
        inputs_idx++;
    }
}


void SoCN900::Init(Config & config, std::vector<std::vector<uint8_t>> const& inputs, vector<uint8_t> const& cmd, vector<uint8_t> const& weight, std::vector<uint8_t> const& lut) {
    std::cout << "SoCS900 Init: Single Layer" << std::endl;
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2023/06/16 |  1.0      | Rui           | important change s000            *
*----------------------------------------------------------------------------*
*****************************************************************************/
void SoCN900::Run() {
    std::cout << "SoCS000 Run" << std::endl;

    // Single layer mode
    if (config_.single_layer) {
        std::cerr << 'No single layer any more!' << std::endl;
        std::exit(1);
    }

    // Full model mode
    vector<Sptr<Node>>& node_list = graph_->GetNodeList();
    DumpOpflow(node_list);
    logger_->info("Start Traversal Graph.");
    for (size_t node_pos = 0; node_pos < node_list.size(); ++node_pos) {
        Sptr<Node> node = node_list[node_pos];
        if (node->node_type_ == NodeType::DATA_NODE) {
            logger_->info("Current node is {} node, node type is DATA_NODE", node->index_);
            continue;
        } else if (node->node_type_ == NodeType::NPU_NODE) {
            int prev_output_node = -1;
            string prev_layer_name = "UNKNOWN";
            for (size_t lookback = node_pos; lookback > 0; --lookback) {
                Sptr<Node> prev_node = node_list[lookback - 1];
                if (prev_node->node_type_ == NodeType::NPU_NODE) {
                    break;
                }
                if (prev_node->node_type_ == NodeType::CPU_NODE && prev_node->opcodeID_ == OpcodeID::CPU_DODMA) {
                    auto prev_dodma_node = Graph::GetCPUDODMANode(prev_node);
                    prev_output_node = prev_dodma_node->output_node_idx_;
                    auto json_it = json_.find(prev_output_node);
                    if (json_it != json_.end()) {
                        prev_layer_name = json_it->second;
                    }
                    break;
                }
            }

            int next_output_node = -1;
            string next_layer_name = "UNKNOWN";
            for (size_t lookahead = node_pos + 1; lookahead < node_list.size(); ++lookahead) {
                Sptr<Node> next_node = node_list[lookahead];
                if (next_node->node_type_ == NodeType::NPU_NODE) {
                    break;
                }
                if (next_node->node_type_ == NodeType::CPU_NODE && next_node->opcodeID_ == OpcodeID::CPU_DODMA) {
                    auto dodma_node = Graph::GetCPUDODMANode(next_node);
                    next_output_node = dodma_node->output_node_idx_;
                    auto json_it = json_.find(next_output_node);
                    if (json_it != json_.end()) {
                        next_layer_name = json_it->second;
                    }
                    break;
                }
            }

            logger_->info("Current node is {} node, node type is NPU_NODE, opcode is {}, next node is {}, prev output layer: {} (node {}), next output layer: {} (node {})",
                          node->index_, OpcodeIDToString(node->opcodeID_), node->next_node_idx_,
                          prev_layer_name, prev_output_node, next_layer_name, next_output_node);
            CURRENT_NPU_NODE_INDEX = node->index_;
            npu_.Run();
        } else if (node->node_type_ == NodeType::CPU_NODE) {
            switch (node->opcodeID_) {
                case OpcodeID::CPU_CDMA: logger_->info("Current node is {} node, node type is CPU_NODE, CDMA", node->index_);
                    dma_.CDMA(Graph::GetCPUCDMANode(node)); npu_.ResetProgramCnt(); break;
                case OpcodeID::CPU_WDMA:          
                    logger_->info("Current node is {} node, node type is CPU_NODE, WDMA", node->index_);
                    dma_.WDMA(Graph::GetCPUWDMANode(node));
                    break;
                case OpcodeID::CPU_DIDMA: {
                    logger_->info("Current node is {} node, node type is CPU_NODE, DIDMA", node->index_);
                    auto cpu_didma_node = Graph::GetCPUDIDMANode(node);
                    auto input_data_node = Graph::GetDataNode(graph_->GetNode(cpu_didma_node->input_node_idx_));
                    if (!DRAMAllocationCheck(input_data_node))
                        throw "DIDMA Input Node Doesn't Allocate!!";
                    dma_.DIDMA(cpu_didma_node, input_data_node, didma_shift_param_);
                    break;
                }
                case OpcodeID::CPU_DODMA: {
                    logger_->info("Current node is {} node, node type is CPU_NODE, DODMA", node->index_);
                    auto cpu_dodma_node = Graph::GetCPUDODMANode(node);
                    auto output_data_node = Graph::GetDataNode(graph_->GetNode(cpu_dodma_node->output_node_idx_));
                    //Allocate output data node
                    if (!DRAMAllocationCheck(output_data_node)) {
                        dram_.Allocate(output_data_node->addr_, output_data_node->height_ * output_data_node->width_ * (((output_data_node->channel_+7) / 8) * 8));  
                        nodes_allocated_.insert(output_data_node->index_);
                    }
                    dma_.DODMA(cpu_dodma_node, output_data_node);
                    DataNodecomparison(cpu_dodma_node, output_data_node);
                    break;
                }
                case OpcodeID::CPU_CONCAT: {
                    std::cerr << "No CONCAT anymore!" << std::endl;
                    std::exit(1);
                    logger_->info("Current node is {} node, node type is CPU_NODE, CONCAT", node->index_);
                    break;
                }
                case OpcodeID::CPU_PDMA: {
                    logger_->info("Current node is {} node, node type is CPU_NODE, PDMA", node->index_);
                    vector<int>().swap(didma_shift_param_);
                    auto cpu_pdma_node = Graph::GetCPUPDMANode(node);
                    vector<uint8_t> shift = dram_.Load(cpu_pdma_node->start_addr_, cpu_pdma_node->len_);
                    assert(shift.size() % 8 == 0);
                    for (unsigned long i = 0; i < shift.size() / 8; ++i) {     //16 channel get 1 channel group
                        for (int j = 0; j < 8; ++j) {
                            didma_shift_param_.emplace_back(shift.at(i * 8 + 7 - j));
                        }
                    }
                    break;
                }
                case OpcodeID::CPU_LUTDMA: {
                    logger_->info("Current node is {} node, node type is CPU_NODE, LUTDMA", node->index_);
                    auto cpu_lutdma_node = Graph::GetCPULUTDMANode(node);
                    dma_.LUTDMA(cpu_lutdma_node);
                    break;
                }
                case OpcodeID::CPU_L2N: {
                    std::cerr << "No CPU_L2N anymore!" << std::endl;
                    std::exit(1);
                    break;
                }
                case OpcodeID::CPU_RESHAPE: { 
                    std::cerr << "No CPU_RESHAPE anymore!" << std::endl;
                    std::exit(1);
                    break;
                }
                case OpcodeID::CPU_SOFTMAX: {
                    std::cerr << "No CPU_SOFTMAX anymore!" << std::endl;
                    std::exit(1);
                    break;
                }
                default: std::cout << "Unsupported CPU operation, abort" << std::endl;
                exit(1);
            }
        }
    }
}
