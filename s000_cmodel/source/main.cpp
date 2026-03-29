// Logistics includes
#include "Define.h"
#include "Config.h"
#include "Parsing.h"
#include "FileIO.h"

// System lib includes
#include <iostream>
#include <cstdlib>
#include <exception>
#include <string>

// Module includes
#include "SoCS900.h"
using std::vector;
using std::string;
using IO::FileIO;

using std::bitset;
using std::ofstream;
using std::endl;
using std::dec;

int main(int argc, char *argv[]) {
    try {
        /****************************************************
         * Set DUMPFILE_DIR from environment variable if available
         ***************************************************/
        const char* env_dumpfile_dir = std::getenv("DUMPFILE_DIR");
        if (env_dumpfile_dir != nullptr) {
            DUMPFILE_DIR = string(env_dumpfile_dir);
        }

        const char* env_dump_node_filter = std::getenv("DUMP_NODE_FILTER");
        if (env_dump_node_filter != nullptr) {
            DUMP_NODE_FILTER = string(env_dump_node_filter);
            std::cout << "Dump node filter enabled: " << DUMP_NODE_FILTER << std::endl;
        }
        
        /****************************************************
         * Parsing Parameters
         * Parse: config.bin
         *        In full model mode:
         *            - Load input.bin, cmd.bin, weight.bin into program
         *            - Parse opflow.bin, generate computation graph
         ***************************************************/
        auto parsedArgs = Parse(argc, argv);

        if (parsedArgs.count("dump-nodes") != 0) {
            DUMP_NODE_FILTER = parsedArgs["dump-nodes"].as<string>();
            std::cout << "Dump node filter from CLI: " << DUMP_NODE_FILTER << std::endl;
        }

        vector<vector<uint8_t>> inputs;
        auto input_string = parsedArgs["data"].as<vector<string>>();
        for (string input : input_string) {
            vector<uint8_t> in = FileIO::LoadBinVector_for_cmd(input);
            inputs.emplace_back(in);
        }
        vector<uint8_t> cmd = FileIO::LoadBinVector_for_cmd(parsedArgs["instruction"].as<string>());
        vector<uint8_t> weight = FileIO::LoadBinVector(parsedArgs["weight"].as<string>());
        vector<uint8_t> lut = FileIO::LoadBinVector(parsedArgs["lut"].as<string>());

        // ofstream f1;
        // f1.open( "./dumpfile/weight_init_main.txt", std::ios::out);
        // for(int i=0; i < weight.size();i++){
        //     string uio;
        //     uio = bitset<8>(weight.at(i)).to_string();
        //     f1 << uio << endl;
        // }
        // f1.close();     

        Config config = Env::ParseConfig(parsedArgs["config"].as<string>());
        std::map<int, string> json;
        if (parsedArgs.count("json") != 0) {
            json = FileIO::LoadJson(parsedArgs["json"].as<string>());
        }
        string golden_address = parsedArgs["golden"].as<string>();


        /****************************************************
         * Initialization:
         * Create blocks: N900 SoC
         * In init, all the building blocks are created and connected.
         *   - All the initialization would be done in this phase
         *      1. In single layer mode:
         *          - Input, cmd, weight data sent to NPU internal buffer directly, based on the information
         *            in config.bin
         *      2. In full model mode:
         *          - Input, cmd, weight data sent to DRAM
         *          - Graph sent to CPU
         ***************************************************/
        Sptr<Graph> graph;
        SoCN900 soc;
        if (!config.single_layer) { // full model
            graph = Env::ParseGraph(parsedArgs["opflow"].as<string>());
            soc.Init(config, graph, inputs, cmd, weight, json, golden_address, lut);
        } else {
            soc.Init(config, inputs, cmd, weight, lut);
            std::cout << "In the mode of signle_layer!" << std::endl;
            std::exit(1);
        }


        /****************************************************
         * Simulation starts
         *   1. In single layer mode:
         *       - NPU runs and dumps computation results
         *   2. In full model mode,
         *       - CPU executes the computation graph
         ***************************************************/
        soc.Run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    } catch (const std::string& e) {
        std::cerr << "Fatal exception: " << e << std::endl;
        return 1;
    } catch (const char* e) {
        std::cerr << "Fatal exception: " << e << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Fatal exception: unknown" << std::endl;
        return 1;
    }
}