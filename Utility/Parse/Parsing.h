#ifndef NPU_PARSING_H
#define NPU_PARSING_H
#include "FileManager.h"
#include "cxxopts.hpp"
using std::string;
using std::map;
using std::vector;
using cxxopts::Options;

/*
 * Parse and validate the arguments
 */
// Needed: config.bin, command.bin, input.bin, weight.bin, opflow.bin (optional)
cxxopts::ParseResult Parse(int argc, char *argv[]) {
    try {
        std::unordered_set<std::string> type_set = {"Generic", "kl720", "kl520"};

        cxxopts::Options options("S900 Cmodel", "S900 Cmodel");
        options.positional_help("[optional args]").show_positional_help();

        options.allow_unrecognised_options()
                .add_options()
                        ("c, config", "Input config.bin file", cxxopts::value<std::string>())
                        ("i, instruction", "Instruction command.bin file", cxxopts::value<std::string>())
                        ("d, data", "Input data input.bin file", cxxopts::value<std::vector<std::string>>())
                        ("w, weight", "Weight weight.bin file", cxxopts::value<std::string>())
                        ("f, opflow", "Operation flow opflow.bin file", cxxopts::value<std::string>())
                        ("l, lut", "LUT lut.bin file", cxxopts::value<std::string>())
                        ("j, json", "Map data node and bin file", cxxopts::value<std::string>())
                        ("g, golden", "golden address", cxxopts::value<std::string>())
                        ("h, help", "optional, print help")
                        ("o, output",
                         "optional, path_to_folder to save the outputs; CREATE THE FOLDER BEFORE CALLING THE BINARY",
                         cxxopts::value<std::string>());
        auto parsedArgs = options.parse(argc, argv);

        if (parsedArgs.count("help")) {
            std::cout << options.help() << std::endl;
            exit(0);
        }

        // parse input arguments
        if (parsedArgs.count("config") == 0) {
            std::cout << "\n  [Error] Input config file is not specified \n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }

        if (parsedArgs.count("instruction") == 0) {
            std::cout << "\n  [Error] Instruction file is not specified \n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }

        if (parsedArgs.count("data") == 0) {
            std::cout << "\n  [Error] Input data file is not specified \n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }

        if (parsedArgs.count("weight") == 0) {
            std::cout << "\n  [Error] Weight file is not specified \n" << std::endl;
            std::cout << options.help() << std::endl;
            exit(1);
        }

        if (parsedArgs.count("opflow") == 0) {
            std::cout << "\n  [Info] Opflow is not specified, will validate this option later \n" << std::endl;
        } else {
            std::cout << "\n  [Info] Opflow is specified \n" << std::endl;
        }

        if (parsedArgs.count("lut") == 0) {
            std::cout << "\n  [Info] Lut is not specified, will validate this option later \n" << std::endl;
        } else {
            std::cout << "\n  [Info] Lut is specified \n" << std::endl;
        }

        if (parsedArgs.count("json") == 0) {
            std::cout << "\n  [Info] Json is not specified, will validate this option later \n" << std::endl;
        } else {
            std::cout << "\n  [Info] Json is specified \n" << std::endl;
        }


        // parse output directory
        if (parsedArgs.count("output")) {
            if (!IO::FileManager::IsDirectory(parsedArgs["output"].as<std::string>())) {
                std::cout
                        << "*******************************\nOutput " << parsedArgs["output"].as<std::string>()
                        << " is not a existing directory\n*******************************\n"
                        << std::endl;
                std::cout << options.help() << std::endl;
                exit(1);
            }
            std::cout << "output = " << parsedArgs["output"].as<std::string>() << std::endl;
        }
        return parsedArgs;
    } catch (const cxxopts::OptionException &e) {
        std::cout << "error parsing options: " << e.what() << std::endl;
        exit(1);
    }
}


#endif //NPU_PARSING_H
