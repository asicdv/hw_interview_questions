`ifndef SIMD_PKG_VH
`define SIMD_PKG_VH

//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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

package simd_pkg;

  // Enumeration of permissible SIMD ALU opcodes.
  //
  typedef enum logic [3:0] { OP_SEL0      = 4'b0000,
                             OP_SEL1      = 4'b0001,
                             OP_ADD32     = 4'b0010,
                             OP_SUB32     = 4'b0011,
                             OP_ADD16     = 4'b0100,
                             OP_SUB16     = 4'b0101,
                             OP_ADD8      = 4'b0110,
                             OP_SUB8      = 4'b0111,
                             OP_ADDSUB16  = 4'b1000,
                             OP_SUBADD16  = 4'b1001,
                             OP_ADDSUB8   = 4'b1010,
                             OP_SUBADD8   = 4'b1011
                           } op_t;

  //
  typedef logic [7:0]  byte_t;
  typedef byte_t [3:0]  word_t;
  typedef logic [3:0]  cntrl_t;

  typedef struct packed {
    byte_t       b;
    logic        c;
  } simd_byte_t;

  typedef struct packed {
    simd_byte_t  [3:0] b;
  } simd_word_t;

endpackage // simd_pkg
`endif
