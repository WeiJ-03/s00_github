#include "FileIO.h"
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
using std::vector;
using std::string;
using std::map;

// TODO
vector<uint8_t> IO::FileIO::LoadBinVector(string filename) {
    vector<uint8_t> data;
    std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    size_t cnt = 0;
    if (file) {
        uint8_t tmp = 0;
        while (cnt < length) {
            file.read(reinterpret_cast<char *>(& tmp), sizeof(uint8_t));
            data.emplace_back(tmp);
            cnt += 1;
            file.seekg(cnt);
        }
    } else {
        std::cout << "ERROR: Cannot open file " << filename << std::endl;
    }
    return data;
}


vector<uint8_t> IO::FileIO::LoadBinVector_for_cmd(string filename) {
    vector<uint8_t> data;
    std::ifstream file(filename, std::ios_base::in | std::ios_base::binary);
    file.seekg(0, std::ios::end);
    size_t length = file.tellg();
    file.seekg(0, std::ios::beg);
    size_t cnt = 0;
    // assert (length%4==0);
    if (file) {
        uint8_t tmp = 0;
        while (cnt < length) {
            vector<uint8_t> sixteen_byte_data;
            for (int i=0; i<4; i++) {
                file.read(reinterpret_cast<char *>(& tmp), sizeof(uint8_t));
                sixteen_byte_data.emplace_back(tmp);
                cnt += 1;
                file.seekg(cnt);
            }
            for (int i=0; i<4; i++) {
                data.emplace_back(sixteen_byte_data[3-i]);
            }
        }
    } else {
        std::cout << "ERROR: Cannot open file " << filename << std::endl;
    }
    return data;
}
//map<int, string> IO::FileIO::LoadJson(std::string filename) {
//    Json::Reader reader;
//    Json::Value root;
//    std::ifstream file(filename, std::ios::binary);
//    map<int, string> json;
//    if (!reader.parse(file, root)){
//        std::cout << "Json file open error" << std::endl;
//        file.close();
//    }else{
//        Json::Value::Members member = root.getMemberNames();
//        for (auto key : member) {
//            json[std::stoi(key)] = root[key].asString();
//        }
//    }
//    return json;
//}

map<int, string> IO::FileIO::LoadJson(std::string filename) {
    json j;
    map<int, string> map_json;
    std::ifstream file(filename, std::ios::binary);
    if (file) {
        file >> j;
        for (json::iterator it = j.begin(); it != j.end(); ++it) {
            map_json[std::stoi(it.key())] = it.value();
        }
    }
    return map_json;
}

// TODO
template <typename T>
void IO::FileIO::SaveBinVector(vector<int> data, string filename) {
    std::ofstream file(filename, std::ios_base::binary);
    vector<T> tmp(data.size());
    for (size_t i = 0; i < data.size(); i++) tmp[i] = (T) data[i];
    T *p = tmp.data();
    file.write((char *) p, sizeof(T) * tmp.size());
}
