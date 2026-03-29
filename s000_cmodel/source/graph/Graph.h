#ifndef N900_MODEL_GRAPH_H
#define N900_MODEL_GRAPH_H

#include <vector>
#include <string>
#include <unordered_map>
#include <cstddef>
#include "Node.h"
#include "Define.h"

using std::vector;
using std::string;
using std::unordered_map;

class Graph {
private:
    vector<Sptr<Node>> node_list_;
    unordered_map<int, Sptr<Node>> node_idx_to_ptr_;
    int GetOneWordValue(std::vector<uint8_t> const& opflow, unsigned long& idx);

    Sptr<Node> ParseCPUNode(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseNPUNode(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseDataNode(std::vector<uint8_t> const& opflow, unsigned long& idx);

    Sptr<Node> ParseCPUOpCDMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpWDMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpDIDMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpDODMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpReshape(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpSoftmax(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpConcat(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpPDMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpCPUShift(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpLUTDMA(std::vector<uint8_t> const& opflow, unsigned long& idx);
    Sptr<Node> ParseCPUOpL2N(std::vector<uint8_t> const& opflow, unsigned long& idx);

public:
    Graph() = delete;
    ~Graph() = default; // Since we always use shared pointer, no need to delete nodes manually
    explicit Graph(std::vector<uint8_t> const& opflow);

    vector<Sptr<Node>>& GetNodeList() { return node_list_; };
    Sptr<Node> GetNode(int node_idx) { return node_idx_to_ptr_.at(node_idx); };
    void PrintGraph();

public:
    static Sptr<CpuCDMANode> GetCPUCDMANode(const Sptr<Node>&);
    static Sptr<CpuWDMANode> GetCPUWDMANode(const Sptr<Node>&);
    static Sptr<CpuDIDMANode> GetCPUDIDMANode(const Sptr<Node>&);
    static Sptr<CpuDODMANode> GetCPUDODMANode(const Sptr<Node>&);
    static Sptr<CpuConcatNode> GetCPUConcatNode(const Sptr<Node>&);
    static Sptr<CpuPDMANode> GetCPUPDMANode(const Sptr<Node>&);
    static Sptr<CpuCPUShiftNode> GetCPUCPUShiftNode(const Sptr<Node>&);
    static Sptr<CpuLUTDMANode> GetCPULUTDMANode(const Sptr<Node>&);
    static Sptr<CpuL2NNode> GetCPUL2NNode(const Sptr<Node>&);
    static Sptr<CpuReshapeNode> GetCPUReshapeNode(const Sptr<Node>&);
    static Sptr<CpuSoftmaxNode> GetCPUSoftmaxNode(const Sptr<Node>&);
    static Sptr<DataNode> GetDataNode(const Sptr<Node>&);
    static Sptr<NPUNode> GetNPUNode(const Sptr<Node>&);
};
#endif //N900_MODEL_GRAPH_H
