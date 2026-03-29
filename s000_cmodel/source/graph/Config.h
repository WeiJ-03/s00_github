#ifndef N900_MODEL_CONFIG_H
#define N900_MODEL_CONFIG_H
#include <string>
#include <fstream>
#include <iostream>
#include "Graph.h"

struct Config {
    bool single_layer = false;
    int in_ch, in_row, in_col;
    int in_bank;  // 0: BankA, 1: BankB
    int in_format;
    int in_offset1_x, in_offset1_y;
    int in_offset2_x, in_offset2_y;

    int out_ch, out_row, out_col;
    int out_bank; // 0: BankA, 1: BankB
    int out_format;
    int out_offset_x, out_offset_y;
    int out_channel_offset;
    int weight_node_addr_, cmd_node_addr_;
    int idata_nodes_num_, odata_nodes_num_;
    std::vector<int> idata_nodes_idx_;
    std::vector<int> odata_nodes_idx_;
};


class Env {
public:
    Env() = default;
    ~Env() = default;

private:
    static vector<uint8_t> ReadBinFile(std::string const& filename, int& length) {
        vector<uint8_t> result;
        try {
            std::ifstream file(filename, std::ifstream::binary);
            file.exceptions(std::ifstream::eofbit | std::ifstream::failbit | std::ifstream::badbit);
            file.seekg(0, file.end);
            length = file.tellg();
            file.seekg(0, file.beg);

            char* buffer = new char[length];
            file.read(buffer, length);
            if (!file) std::cout << "Error: only " << file.gcount() << " could be read" << std::endl;
            file.close();

            for (int i = 0; i < length; i++) {
                result.push_back((uint8_t) buffer[i]);
            }
            delete[] buffer;
        } catch (std::exception const& e) {
            std::cout << e.what() << std::endl;
        }
        return result;
    }

    /**
     *  Extract the information from the buffer and put into config object
     *  @param config: The reference to the config object
     *  @param buffer: The pointer to the buffer start address
     *  @param length: The length of the buffer
     */
    static void ExtractBufferInfo(Config& config, vector<uint8_t> const& buffer, int length) {
        int num_of_words = length / 4;
        for (int i = 0; i < num_of_words; i++) {
            int value = 0;
            for (int j = 0; j < 4; j++) {
                value += ((int) buffer[i * 4 + j]) << 8 * (3-j);
            }
            if (i == 0) config.single_layer = (value == 0);
            else if (config.single_layer == 1) {
                switch (i) {
//                    case 0: break;
                    case 1: config.in_ch = value; break;
                    case 2: config.in_row = value; break;
                    case 3: config.in_col = value; break;
                    case 4: config.in_bank = value; break;
                    case 5: config.in_format = value; break;
                    case 6: config.in_offset1_x = value; break;
                    case 7: config.in_offset1_y = value; break;
                    case 8: config.in_offset2_x = value; break;
                    case 9: config.in_offset2_y = value; break;
                    case 10: config.out_ch = value; break;
                    case 11: config.out_row = value % 65536; break;   //% 65536 means value low 16 bits is out_row
                    case 12: config.out_col = value % 65536; break;
                    case 13: config.out_bank = value; break;
                    case 14: config.out_format = value; break;
                    case 15: config.out_offset_x = value; break;
                    case 16: config.out_offset_y = value; break;
                    case 17: config.out_channel_offset = value; break;
                    default:
                        std::cout << "This line shouldn't be reached! (Env private static ExtractBufferInfo) "
                                  << std::endl;
                }
            } else {
                if (i == 1) config.cmd_node_addr_ = value;
                else if (i == 2) config.weight_node_addr_ = value;
                else if (i == 3) config.idata_nodes_num_ = value;
                else if (i > 3 && i <= 3 + config.idata_nodes_num_) config.idata_nodes_idx_.emplace_back(value);
//                else if (i > 2)config.data_nodes_idx_.emplace_back(value);
                else if (i == 4 + config.idata_nodes_num_) config.odata_nodes_num_ = value;
                else config.odata_nodes_idx_.emplace_back(value);
            }
            // if (!config.single_layer) return;
        }
    }


public:
    /**
     *  Parsing the config_file and fill in the values
     *  @param config_file: The absolute path for config.bin
     *  @return: The constructed Config object
     */
    static Config ParseConfig(std::string const& config_file) {
        int length = 0;
        vector<uint8_t> buffer = ReadBinFile(config_file, length);
        Config config;
        ExtractBufferInfo(config, buffer, length);
        return config;
    }

    /**
     *  Parsing the opflow_file and create the graph
     *  @param opflow_file: The absolute path for opflow.bin
     *  @return: A shared_ptr pointing to uhe constructed graph object
     */
    static Sptr<Graph> ParseGraph(std::string const& opflow_file) {
        int length = 0;
        vector<uint8_t> buffer = ReadBinFile(opflow_file, length);
        Graph* graph_ptr = new Graph(buffer);
        return Sptr<Graph>(graph_ptr);
    }
};

#endif //N900_MODEL_CONFIG_H
