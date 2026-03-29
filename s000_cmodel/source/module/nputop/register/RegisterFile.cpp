#include "RegisterFile.h"
#include <iostream>

RegisterFile::RegisterFile() {

}


RegisterFile::~RegisterFile() {

}


// Add/Remove register
void RegisterFile::AddReg(std::string reg_name, Register reg) {
    if (reg_map_.find(reg_name) == reg_map_.end()) {
        reg_map_.insert(std::pair<std::string, Register>(reg_name, reg));
    }
}

void RegisterFile::RemoveReg(std::string reg_name) {
    std::map<std::string, Register>::iterator it = reg_map_.find(reg_name);
    if (it != reg_map_.end()) {
        reg_map_.erase(it);
    }
}

Register & RegisterFile::operator[](std::string reg_name) {
    if (reg_map_.find(reg_name) != reg_map_.end()) {
        return reg_map_[reg_name];
    } else {
        throw "RegisterFile: RegisterNotExist in Reg_map_";
    }
}

// Set register field for a specific register
void RegisterFile::SetRegField(std::string reg_name, std::string reg_field, int value) {
    if (reg_map_.find(reg_name) != reg_map_.end()) {
        reg_map_[reg_name].SetFields(reg_field, value);
    } else {
        throw "RegisterFile: RegisterNotExist in Reg_map_";
    }

}

int RegisterFile::GetRegField(std::string reg_name, std::string reg_field) {
    if (reg_map_.find(reg_name) != reg_map_.end()) {
        reg_map_[reg_name].GetFields(reg_field);
    } else {
        throw "RegisterFile: RegisterNotExist in Reg_map_";
    }
}

 void RegisterFile::BuildRegisterFile() {
    // Creat register and put it into RegisterFile
    Register reg[16];

    reg[  0] = Register(  0, "reg_CONV_MODE"  ); reg[  0].AddFields("mode",0).AddFields("trim",0);
                                                 reg[  0].AddFields("AB_order",0).AddFields("full_ch",0).AddFields("ch_st",0).AddFields("upsample",0);
    reg[  1] = Register(  1, "reg_FM_ROW"     ); reg[  1].AddFields("row",0);
    reg[  2] = Register(  2, "reg_FM_COL"     ); reg[  2].AddFields("col",0);
    reg[  3] = Register(  3, "reg_FM_ICH"     ); reg[  3].AddFields("ich",0);
    reg[  4] = Register(  4, "reg_FM_OCH_ST"  ); reg[  4].AddFields("och_st",0);
    reg[  5] = Register(  5, "reg_FM_OCH_ED"  ); reg[  5].AddFields("och_ed",0);
    reg[  6] = Register(  6, "NL_MODE"  );       reg[  6].AddFields("pool_mode",0).AddFields("pool_kernel",0);
                                                 reg[  6].AddFields("pool_stride",0).AddFields("relu_mode",0).AddFields("pool_en",0).AddFields("relu_en",0);
                                                 reg[  6].AddFields("bn_en",0).AddFields("nl_en",0).AddFields("nl_order",0).AddFields("act_LUT_en",0);
    reg[  7] = Register(  7, "POOL_PAD"  );      reg[  7].AddFields("r",0).AddFields("l",0);
                                                 reg[  7].AddFields("b",0).AddFields("t",0);
    reg[  8] = Register(  8, "reg_MEM_IN1"    ); reg[  8].AddFields("offset_x",0).AddFields("offset_y",0);
    reg[  9] = Register(  9, "reg_MEM_IN2"    ); reg[  9].AddFields("offset_x",0).AddFields("offset_y",0);
    reg[  10] = Register( 10, "reg_MEM_OUT"    ); reg[  10].AddFields("offset_x",0).AddFields("offset_y",0);
    reg[  11] = Register( 11, "reg_CROP"       ); reg[  11].AddFields("col_st",0).AddFields("row_st",0);
    reg[  12] = Register( 12, "reg_CROP_ROW"   ); reg[ 12].AddFields("row_out",0);
    reg[  13] = Register( 13, "reg_CROP_COL"   ); reg[ 13].AddFields("col_out",0);
    reg[  14] = Register( 14, "reg_PAD1"       ); reg[ 14].AddFields("t",0).AddFields("b",0);
    reg[  15] = Register( 15, "reg_PAD2"       ); reg[ 15].AddFields("l",0).AddFields("r",0);
    
    this -> AddReg( "reg_CONV_MODE"       ,   reg[  0]);
    this -> AddReg( "reg_FM_ROW"          ,   reg[  1]);
    this -> AddReg( "reg_FM_COL"          ,   reg[  2]);
    this -> AddReg( "reg_FM_ICH"          ,   reg[  3]);
    this -> AddReg( "reg_FM_OCH_ST"       ,   reg[  4]);
    this -> AddReg( "reg_FM_OCH_ED"       ,   reg[  5]);
    this -> AddReg( "NL_MODE"             ,   reg[  6]);
    this -> AddReg( "POOL_PAD"            ,   reg[  7]);
    this -> AddReg( "reg_MEM_IN1"         ,   reg[  8]);
    this -> AddReg( "reg_MEM_IN2"         ,   reg[  9]);
    this -> AddReg( "reg_MEM_OUT"         ,   reg[  10]);
    this -> AddReg( "reg_CROP"            ,   reg[  11]);
    this -> AddReg( "reg_CROP_ROW"        ,   reg[  12]);
    this -> AddReg( "reg_CROP_COL"        ,   reg[  13]);
    this -> AddReg( "reg_PAD1"            ,   reg[  14]);
    this -> AddReg( "reg_PAD2"            ,   reg[  15]);
}

/*****************************************************************************
*  Change History :                                                          *
*  <Date>     | <Version> | <Author>       | <Description>                   *
*----------------------------------------------------------------------------*
*  2022/10/6 |  1.0       | Ning           | Unified the shift format        *
*  2022/10/6 |  1.0       | Ning           | Reduce the two sets of registers*
*----------------------------------------------------------------------------*
*****************************************************************************/
void RegisterFile::WriteRegisterFile(uint16_t target_idx, uint16_t target_value) {
    switch (target_idx) {
        case  0:(*this)["reg_CONV_MODE"].SetFields("mode", target_value&0x000f);
                (*this)["reg_CONV_MODE"].SetFields("trim", (target_value&0x0300)>>8);
                (*this)["reg_CONV_MODE"].SetFields("AB_order", (target_value&0x1000)>>12).SetFields("full_ch",(target_value&0x2000)>>13);
                (*this)["reg_CONV_MODE"].SetFields("ch_st", (target_value&0x4000)>>14).SetFields("upsample", (target_value>>15));break;
        case  1:(*this)["reg_FM_ROW"   ].SetFields("row", target_value); break;
        case  2:(*this)["reg_FM_COL"   ].SetFields("col", target_value); break;
        case  3:(*this)["reg_FM_ICH"   ].SetFields("ich", target_value); break;
        case  4:(*this)["reg_FM_OCH_ST"].SetFields("och_st", target_value); break;
        case  5:(*this)["reg_FM_OCH_ED"].SetFields("och_ed", target_value); break;
        case  6:(*this)["NL_MODE"].SetFields("pool_mode", target_value&0x0003);
                (*this)["NL_MODE"].SetFields("pool_kernel", (target_value&0x0004)>>2);
                (*this)["NL_MODE"].SetFields("pool_stride", (target_value&0x0018)>>3).SetFields("relu_mode",(target_value&0x0060)>>5);
                (*this)["NL_MODE"].SetFields("pool_en", (target_value&0x0080)>>7).SetFields("relu_en", (target_value&0x0100)>>8);
                (*this)["NL_MODE"].SetFields("bn_en", (target_value&0x0200)>>9);
                (*this)["NL_MODE"].SetFields("nl_en", (target_value&0x0400)>>10);
                (*this)["NL_MODE"].SetFields("nl_order", (target_value&0x3800)>>11);
                (*this)["NL_MODE"].SetFields("act_LUT_en", (target_value&0xC000)>>14);
                break;        
        case  7:(*this)["POOL_PAD"].SetFields("r", target_value&0x0003);
                (*this)["POOL_PAD"].SetFields("l", (target_value&0x000C)>>2);
                (*this)["POOL_PAD"].SetFields("b", (target_value&0x0030)>>4).SetFields("t",(target_value&0x00C0)>>6);
                break;
        case  8:(*this)["reg_MEM_IN1"  ].SetFields("offset_x", (target_value&0x3ff0)>>4).SetFields("offset_y", target_value&0x000f); break;
        case  9:(*this)["reg_MEM_IN2"  ].SetFields("offset_x", (target_value&0x3ff0)>>4).SetFields("offset_y", target_value&0x000f); break;
        case  10:(*this)["reg_MEM_OUT"  ].SetFields("offset_x", (target_value&0x3ff0)>>4).SetFields("offset_y", target_value&0x000f); break;
        case  11:(*this)["reg_CROP"     ].SetFields("col_st", target_value&0x00ff).SetFields("row_st", (target_value&0xff00)>>8); break;
        case  12:(*this)["reg_CROP_ROW" ].SetFields("row_out", target_value); break;
        case  13:(*this)["reg_CROP_COL" ].SetFields("col_out", target_value); break;
        case  14:(*this)["reg_PAD1"     ].SetFields("b", target_value&0x00ff).SetFields("t", (target_value&0xff00)>>8); break;
        case  15:(*this)["reg_PAD2"     ].SetFields("r", target_value&0x00ff).SetFields("l", (target_value&0xff00)>>8); break;
        default: std::cout << "Error: Doesn't have this register index!" << std::endl; break;
    }
}

