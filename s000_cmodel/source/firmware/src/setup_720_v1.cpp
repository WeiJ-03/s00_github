//
// Created by yiyu on 12/4/19.
//

#include "setup_720_v1.h"

using namespace setup;

void Setup720V1::Parse() {
    // parse header
    Advance(5);  // crc, version, models, model type, application type
    HeaderBuilder hb(builder);
    hb.add_dram_start(Advance());
    hb.add_dram_len(Advance());
    hb.add_cmd_start(Advance());
    hb.add_cmd_len(Advance());
    hb.add_weight_start(Advance());
    hb.add_weight_len(Advance());
    hb.add_input_start(Advance());
    hb.add_input_len(Advance());
    Advance(2);  // input_num, output_num
    auto header = hb.Finish();

    ParseNode();
    auto content = CreateContent(builder, header, builder.CreateVector(nodes));

    FinishContentBuffer(builder, content);
    std::vector<uint32_t>().swap(input);  // clear memory when we are done
}

void Setup720V1::ParseNode() {
    uint32_t id;
    while (true) {
        try {
            id = Advance();
        } catch (SetupOutOfRange&) {
            // the binary has been consumed
            return;
        }
        switch (id) {
            case 0: {
                // npu
                auto npu_opt = CreateNpuOptions(builder, Advance());
                auto node = CreateNode(builder, NodeOptions_Npu, npu_opt.Union());
                nodes.push_back(node);
                break;
            }
            case 1: {
                // cpu
                Advance(15 - 1);  // skip everything in cpu node
                // todo: generate cpu node based on this
                CpuOptionsBuilder cpu_opt(builder);

                std::vector<flatbuffers::Offset<IOOptions>> io_vector;
                // output
                io_vector.push_back(ParseDataNode());
                cpu_opt.add_output(builder.CreateVector(io_vector));
                io_vector.clear();

                // input
                io_vector.push_back(ParseDataNode());
                cpu_opt.add_input(builder.CreateVector(io_vector));
                auto node = CreateNode(builder, NodeOptions_Cpu, cpu_opt.Finish().Union());
                nodes.push_back(node);
                break;
            }
            case 2: {
                // output
                IOOptionsBuilder io(builder);
                auto super_count = Advance();
                if (super_count > 1) {
                    throw std::runtime_error("what does super node mean?");
                }
                io.add_format(Advance());
                Advance(3);  // row, col, ch start
                io.add_height(Advance());
                io.add_width(Advance());
                io.add_channel(Advance());
                io.add_index(Advance());
                io.add_radix(Advance());
                io.add_scale(AdvanceFloat());
                io.add_addr(ParseSuperNode());
                auto node = CreateNode(builder, NodeOptions_Out, io.Finish().Union());
                nodes.push_back(node);
                break;
            }
            case 6:
            case 5: {
                // input
                IOOptionsBuilder io(builder);
                io.add_index(Advance());
                io.add_format(Advance());
                io.add_height(Advance());
                io.add_width(Advance());
                io.add_channel(Advance());
                io.add_addr(Advance());
                Advance();  // todo: we might be able to check len here
                io.add_radix(Advance());
                auto node = CreateNode(builder, NodeOptions_In, io.Finish().Union());
                nodes.push_back(node);
                break;
            }
            default:
                throw std::runtime_error("unknown node");
        }
    }
}

flatbuffers::Offset<setup::IOOptions> Setup720V1::ParseDataNode() {
    IOOptionsBuilder io(builder);
    Advance();  // node id
    auto super_count = Advance();
    if (super_count > 1) {
        throw std::runtime_error("what does super node mean?");
    }
    io.add_format(Advance());
    io.add_radix(Advance());
    io.add_scale(AdvanceFloat());
    Advance(3);  // row, col, ch start
    io.add_height(Advance());
    io.add_width(Advance());
    io.add_channel(Advance());
    io.add_addr(ParseSuperNode());
    return io.Finish();
}

uint32_t Setup720V1::ParseSuperNode() {
    Advance();             // node id
    auto ret = Advance();  // addr
    Advance(6);            // row, col, ch dimension
    return ret;
}
