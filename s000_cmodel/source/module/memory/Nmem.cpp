#include "Nmem.h"

//Nmem::Nmem() {
//
//}

//void Nmem::init(RegisterFile * rf) {
//    regFile = rf;
//}

//void Nmem::spaceAllocate() {
//    nmem_fm.offset_x = (*regFile)["NMEM_FM0"]["offset_x"];
//    nmem_fm.offset_y = (*regFile)["NMEM_FM0"]["offset_y"];
//    nmem_fm.h = (*regFile)["NMEM_FM1"]["h"];
//    nmem_fm.w = (*regFile)["NMEM_FM1"]["w"];
//    nmem_fm.l = (*regFile)["NMEM_FM2"]["l"];
//    nmem_fm.offset_s = (*regFile)["NMEM_FM2"]["offset_s"];
//    nmem_fm.s = (*regFile)["NMEM_FM2"]["s"];
//
//    nmem_ps.offset_x = (*regFile)["NMEM_PS0"]["offset_x"];
//    nmem_ps.offset_y = (*regFile)["NMEM_PS0"]["offset_y"];
//    nmem_ps.h = (*regFile)["NMEM_PS1"]["h"];
//    nmem_ps.w = (*regFile)["NMEM_PS1"]["w"];
//    nmem_ps.l = 0 ;
//    nmem_ps.offset_s = (*regFile)["NMEM_PS2"]["offset_s"];
//    nmem_ps.s = (*regFile)["NMEM_PS2"]["s"];
//
//    nmem_st.offset_x = (*regFile)["NMEM_ST0"]["offset_x"];
//    nmem_st.offset_y = (*regFile)["NMEM_ST0"]["offset_y"];
//    nmem_st.h = (*regFile)["NMEM_ST1"]["h"];
//    nmem_st.w = (*regFile)["NMEM_ST1"]["w"];
//    nmem_st.l = 0 ;
//    nmem_st.offset_s = (*regFile)["NMEM_ST2"]["offset_s"];
//    nmem_st.s = (*regFile)["NMEM_ST2"]["s"];
//
//    nmem_pi.offset_x = (*regFile)["NMEM_PI0"]["offset_x"];
//    nmem_pi.offset_y = (*regFile)["NMEM_PI0"]["offset_y"];
//    nmem_pi.h = (*regFile)["NMEM_PI1"]["h"];
//    nmem_pi.w = (*regFile)["NMEM_PI1"]["w"];
//    nmem_pi.l = 0 ;
//    nmem_pi.offset_s = (*regFile)["NMEM_PI2"]["offset_s"];
//    nmem_pi.s = (*regFile)["NMEM_PI2"]["s"];
//
//    nmem_po.offset_x = (*regFile)["NMEM_PO0"]["offset_x"];
//    nmem_po.offset_y = (*regFile)["NMEM_PO0"]["offset_y"];
//    nmem_po.h = (*regFile)["NMEM_PO1"]["h"];
//    nmem_po.w = (*regFile)["NMEM_PO1"]["w"];
//    nmem_po.l = 0 ;
//    nmem_po.offset_s = (*regFile)["NMEM_PO2"]["offset_s"];
//    nmem_po.s = (*regFile)["NMEM_PO2"]["s"];
//
//    nmem_rdma.offset_x = (*regFile)["NMEM_RDMA0"]["offset_x"];
//    nmem_rdma.offset_y = (*regFile)["NMEM_RDMA0"]["offset_y"];
//    nmem_rdma.h = (*regFile)["NMEM_RDMA1"]["h"];
//    nmem_rdma.w = (*regFile)["NMEM_RDMA1"]["w"];
//    nmem_rdma.l = 0 ;
//    nmem_rdma.offset_s = (*regFile)["NMEM_RDMA2"]["offset_s"];
//    nmem_rdma.s = (*regFile)["NMEM_RDMA2"]["s"];
//
//    nmem_wdma.offset_x = (*regFile)["NMEM_WDMA0"]["offset_x"];
//    nmem_wdma.offset_y = (*regFile)["NMEM_WDMA0"]["offset_y"];
//    nmem_wdma.h = (*regFile)["NMEM_WDMA1"]["h"];
//    nmem_wdma.w = (*regFile)["NMEM_WDMA1"]["w"];
//    nmem_wdma.l = 0 ;
//    nmem_wdma.offset_s = (*regFile)["NMEM_WDMA2"]["offset_s"];
//    nmem_wdma.s = (*regFile)["NMEM_WDMA2"]["s"];
//}

//Tensor Nmem::read(std::string dest, Position p) {
//    Position entryPos;
//    // Change pixel position info to entry position info
//    if (dest == "NMEM_ST") { // Let's assume NMEM_ST only has 1W16C8B
//        entryPos.r = p.r;
//        entryPos.c = p.c;
//        entryPos.ch = p.ch / 16;
//        entryPos.rStep = p.rStep;
//        entryPos.cStep = p.cStep;
//        entryPos.chStep = p.chStep / 16;
//    } else if (dest == "NMEM_PS") {
//        entryPos.r = p.r;
//        entryPos.c = p.c / 16;
//        entryPos.ch = p.ch;
//        entryPos.rStep = p.rStep;
//        entryPos.cStep = p.cStep;
//        entryPos.chStep = p.chStep;
//    } else if (dest == "NMEM_ST") {
//
//    }
//
//
//    Tensor t(p.rStep, p.cStep, p.chStep, 1);
//
//    for (int i = entryPos.r; i < entryPos.r + entryPos.rStep; i++) {
//        for (int j = entryPos.c; j < entryPos.c + entryPos.cStep; j++) {
//            // Translate entry position to intermediate address, (u, v)
//            int u = (entryPos.c / nmem_fm.w) * nmem_fm.h + entryPos.ch * (*regFile)["FMAP0"]["col"] + entryPos.r;
//            int v = entryPos.c / nmem_fm.w;
//
//            int u_prime = u;
//            int v_prime = nmem_fm.s + nmem_fm.offset_s;
//
//            int line = (nmem_fm.offset_y + u_prime) % 32;
//            int entry = nmem_fm.offset_x + nmem_fm.w * (nmem_fm.offset_y + u_prime) / 32 + v_prime;
//
//            // Translate entry position to SRAM logical line and entry offset
//
//
//
//            // Translate SRAM logical line and entry offset to SRAM physical line and entry offset
//
//            // Use SRAM physical line and entry offset to get the real Entries
//
//            // Fill the entries into the tensor
//        }
//
//    }
//
//    // Temp initialization
//    uint32_t temp = 0;
//    for (int i = 0; i < p.rStep; i++) {
//        for (int j = 0; j < p.cStep; j++) {
//            for (int k = 0; k < p.chStep; k++) {
//                t.set(i, j, k, temp++);
//                t[i][j][k] = temp++;
//            }
//        }
//    }
//
//    return t;
//}
//
//void Nmem::write(std::string dest, Position p, Tensor t) {
//
//}

