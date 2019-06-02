`ifndef __VERT_UCODE_QUICKSORT_INSTS_VH__
 `define __VERT_UCODE_QUICKSORT_INSTS_VH__

//========================================================================== //
// Copyright (c) 2018, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

task inst_default; begin
  da_inst_w  = '0;
end endtask

task inst_j (pc_t dest, cc_t cc = UNCOND); begin
  inst_default();
  //
  da_inst_w.opcode    = JCC;
  da_inst_w.u.jcc.cc  = cc;
  da_inst_w.u.jcc.A   = field_A_t'(dest);
end endtask

task inst_wait; begin
  inst_default();
  //
  da_inst_w.opcode  = CNTRL;
end endtask

task inst_emit; begin
  inst_default();
  //
  da_inst_w.opcode           = CNTRL;
  da_inst_w.u.cntrl.is_emit  = 'b1;
end endtask

task inst_call(pc_t dest); begin
  inst_default();
  //
  da_inst_w.opcode    = CRET;
  da_inst_w.u.cret.a  = field_A_t'(dest);
end endtask

task inst_ret; begin
  inst_default();
  //
  da_inst_w.opcode         = CRET;
  da_inst_w.u.cret.is_ret  = 'b1;
end endtask

task inst_push(reg_t r); begin
  inst_default();
  //
  da_inst_w.opcode            = PP;
  da_inst_w.u.pp.u.push.src1  = r;
end endtask

task inst_pop(reg_t r); begin
  inst_default();
  //
  da_inst_w.opcode          = PP;
  da_inst_w.u.pp.is_pop     = 'b1;
  da_inst_w.u.pp.u.pop.dst  = r;
end endtask

task inst_mov(reg_t dst, reg_t src0); begin
  inst_default();
  //
  da_inst_w.opcode       = MOV;
  da_inst_w.u.mov.dst    = dst;
  da_inst_w.u.mov.u.src  = src0;
end endtask

task inst_movi(reg_t dst, imm_t imm); begin
  inst_default();
  //
  da_inst_w.opcode        = MOV;
  da_inst_w.u.mov.dst     = dst;
  da_inst_w.u.mov.is_imm  = 'b1;
  da_inst_w.u.mov.u.imm   = imm;
end endtask

task inst_movs(reg_t dst, reg_special_t src1); begin
  inst_default();
  //
  da_inst_w.opcode            = MOV;
  da_inst_w.u.mov.dst         = dst;
  da_inst_w.u.mov.is_special  = 'b1;
  da_inst_w.u.mov.u.special   = src1;
end endtask

task inst_addi(reg_t dst, reg_t src0, imm_t imm, bit dst_en = 'b1); begin
  inst_default();
  //
  da_inst_w.opcode          = ARITH;
  da_inst_w.u.arith.dst     = dst;
  da_inst_w.u.arith.src0    = src0;
  da_inst_w.u.arith.is_imm  = 'b1;
  da_inst_w.u.arith.u.imm   = imm;
  da_inst_w.u.arith.wren    = dst_en;
end endtask

task inst_subi(reg_t dst, reg_t src0, imm_t imm, bit dst_en = 'b1); begin
  inst_default();
  //
  da_inst_w.opcode          = ARITH;
  da_inst_w.u.arith.is_sub  = 'b1;
  da_inst_w.u.arith.dst     = dst;
  da_inst_w.u.arith.src0    = src0;
  da_inst_w.u.arith.is_imm  = 'b1;
  da_inst_w.u.arith.u.imm   = imm;
  da_inst_w.u.arith.wren    = dst_en;
end endtask

task inst_sub(reg_t dst, reg_t src0, reg_t src1, bit dst_en = 'b1); begin
  inst_default();
  //
  da_inst_w.opcode          = ARITH;
  da_inst_w.u.arith.is_sub  = 'b1;
  da_inst_w.u.arith.wren    = dst_en;
  da_inst_w.u.arith.dst     = dst;
  da_inst_w.u.arith.src0    = src0;
  da_inst_w.u.arith.u.src1  = src1;
end endtask

task inst_ld(reg_t dst, reg_t src1); begin
  inst_default();
  //
  da_inst_w.opcode           = MEM;
  da_inst_w.u.mem.dst        = dst;
  da_inst_w.u.mem.u.ld.src1  = src1;
end endtask

task inst_st(reg_t src0, reg_t src1); begin
  inst_default();
  //
  da_inst_w.opcode           = MEM;
  da_inst_w.u.mem.is_st      = 'b1;
  da_inst_w.u.mem.u.st.src0  = src0;
  da_inst_w.u.mem.u.st.src1  = src1;
end endtask

task inst_nop; begin
end endtask

`endif //  `ifndef __VERT_UCODE_QUICKSORT_INSTS_VH__

