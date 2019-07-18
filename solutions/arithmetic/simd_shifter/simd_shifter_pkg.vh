//========================================================================== //
// Copyright (c) 2019, Stephen Henry
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

`ifndef __SIMD_SHIFTER_PKG_VH__
`define __SIMD_SHIFTER_PKG_VH__

package simd_shifter_pkg;

  typedef enum logic [1:0] {  MODE_1B  = 2'b00,
                              MODE_2B  = 2'b01,
                              MODE_4B  = 2'b10,
                              MODE_8B  = 2'b11 } mode_t;

  typedef enum logic [2:0] {  OP_NOP   = 3'b000,
                              OP_SLL   = 3'b001,
                              OP_SRL   = 3'b010,
                              OP_SLA   = 3'b100,
                              OP_SRA   = 3'b101 } op_t;

  typedef logic [5:0] shift_t;

  typedef union packed {
    logic [63:0]      w64;
    logic [1:0][31:0] w32;
    logic [3:0][15:0] w16;
    logic [7:0][7:0]  w8;
  } word_t;

endpackage // simd_shifter_pkg

`endif
