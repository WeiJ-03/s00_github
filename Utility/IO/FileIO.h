#ifndef N900_MODEL_FILEIO_H
#define N900_MODEL_FILEIO_H
#include <string>
#include <vector>
#include "nlohmann/json.hpp"
using json = nlohmann::json;

namespace IO {
    class FileIO {
    public:
        static std::vector<uint8_t> LoadBinVector(std::string file);
        static std::vector<uint8_t> LoadBinVector_for_cmd(std::string file);
        static std::map<int, std::string> LoadJson(std::string file);
        template <typename T>
        static void SaveBinVector(std::vector<int> data, std::string filename);
        FileIO() = default;
        ~FileIO() = default;

    private:
        static char const PATH_DELIMITER = '/';
    };
}

#endif //N900_MODEL_FILEIO_H
