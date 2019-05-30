`ifndef __READ_MODIFY_WRITE_PKG_VH__
`define __READ_MODIFY_WRITE_PKG_VH__

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

package read_modify_write_pkg;

  //
  typedef logic [4:0] reg_t;
  typedef logic [31:0] word_t;

  //
  typedef enum logic [3:0] {   OP_NOP = 4'b0000
                             , OP_AND = 4'b0001
                             , OP_NOT = 4'b0010
                             , OP_OR  = 4'b0011
                             , OP_XOR = 4'b0100
                             , OP_ADD = 4'b0101
                             , OP_SUB = 4'b0110
                             , OP_MOV0 = 4'b0111
                             , OP_MOV1 = 4'b1000
                             , OP_MOVI = 4'b1001
                            } opcode_t;

  //
  typedef struct packed {
    reg_t [1:0]     ra;
    reg_t           wa;
    opcode_t        op;
  } inst_t;
  localparam int INST_W  = $bits(inst_t);

  //
  typedef logic [31:0] imm_t;
  localparam int IMM_W  = $bits(imm_t);

  localparam int FIFO_W  = (INST_W + IMM_W);

endpackage // read_modify_write_pkg

`endif
