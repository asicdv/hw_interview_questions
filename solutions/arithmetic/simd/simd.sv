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

`include "simd_pkg.vh"

module simd
(
   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Cntrl                                                                   //
   //                                                                         //
   //======================================================================== //

   , input                                   pass
   , input  simd_pkg::op_t                   op
   , input  simd_pkg::word_t                 A
   , input  simd_pkg::word_t                 B
   //
   , output simd_pkg::word_t                 Y_r
   , output logic                            valid_r
);
  import simd_pkg::*;

  // ------------------------------------------------------------------------ //
  //
  logic                                      valid_w;
  logic                                      simd_Y_en;
  simd_pkg::simd_word_t                      simd_A, simd_B, simd_Y_w, simd_Y_r;
  simd_pkg::cntrl_t                          cntrl_A, cntrl_B;

  // Function to conditionally invert individual bytes within the word based
  // upon some opcode.
  function word_t cond_invert(op_t op, word_t w);
    begin
      logic [3:0] inv;
      word_t r;
      case (op)
        OP_SUB32,
        OP_SUB16,
        OP_SUB8:     inv  = 4'b1111;
        OP_ADDSUB16: inv  = 4'b0011;
        OP_SUBADD16: inv  = 4'b1100;
        OP_ADDSUB8:  inv  = 4'b0101;
        OP_SUBADD8:  inv  = 4'b1010;
        default:     inv  = 4'b0000;
      endcase // case (op)
      for (int i = 0; i < 4; i++)
        r [i] = {8{inv[i]}} ^ w[i];
      return r;
    end
  endfunction // cond_invert

  // Function to pack data word and control bits into SIMD word.
  function simd_word_t pack_simd(word_t w, cntrl_t c);
    begin
      simd_word_t r;
      for (int i = 0; i < 4; i++)
        r.b[i] = {w[i], c[i]};
      return r;
    end
  endfunction // pack_simd

  // Function to unpack data word from SIMD word.
  function word_t unpack_simd(simd_word_t sw);
    begin
      word_t w;
      for (int i = 0; i < 4; i++)
        w[i] = sw.b[i].b;
      return w;
    end
  endfunction // unpack_simd

  // Function to compute control bits on A input.
  function cntrl_t op_to_cntrl_A(op_t op);
    begin
      cntrl_t r;
      case (op)
        OP_ADD32:    r  = 4'b1111;
        OP_SUB32:    r  = 4'b1111;
        OP_ADD16:    r  = 4'b1010;
        OP_SUB16:    r  = 4'b1111;
        OP_ADD8:     r  = 4'b0000;
        OP_SUB8:     r  = 4'b1111;
        OP_ADDSUB16: r  = 4'b1011;
        OP_SUBADD16: r  = 4'b1110;
        OP_ADDSUB8:  r  = 4'b0101;
        OP_SUBADD8:  r  = 4'b1010;
        default:     r  = 4'b0000;
      endcase // case (op)
      return r;
    end
  endfunction // op_to_cntrl_A

  // Function to compute control bits on B input.
  function cntrl_t op_to_cntrl_B(op_t op);
    begin
      cntrl_t r;
      case (op)
        OP_ADD32:    r  = 4'b0000;
        OP_SUB32:    r  = 4'b0001;
        OP_ADD16:    r  = 4'b0000;
        OP_SUB16:    r  = 4'b0101;
        OP_ADD8:     r  = 4'b0000;
        OP_SUB8:     r  = 4'b1111;
        OP_ADDSUB16: r  = 4'b0001;
        OP_SUBADD16: r  = 4'b0100;
        OP_ADDSUB8:  r  = 4'b0101;
        OP_SUBADD8:  r  = 4'b1010;
        default:     r  = 4'b0000;
      endcase // case (op)
      return r;
    end
  endfunction // op_to_cntrl_B

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : simd_PROC

      // Compute control bits.
      //
      cntrl_A    = op_to_cntrl_A(op);
      cntrl_B    = op_to_cntrl_B(op);

      // Compute the SIMD words for A, B. Conditionally invert B for SUB
      // opeations.
      //
      simd_A     = pack_simd(A, cntrl_A);
      simd_B     = pack_simd(cond_invert(op, B), cntrl_B);

      // Compute final result. NOTE: the OP_SEL{01} opcodes may be neglected.
      // They are included here because most microprocessor ALU require some
      // ability to select between individual oprands in the ALU. A conditional
      // move instruction, for example, is typically implemented as a
      // conditional arithmetic instruction where one input is 0.
      //
      case (op)
        OP_SEL0: simd_Y_w  = simd_A;
        OP_SEL1: simd_Y_w  = simd_B;
        default: simd_Y_w  = simd_A + simd_B;
      endcase // case (op_t)

      // Recover original packed format, drop control bits.
      //
      Y_r                = unpack_simd(simd_Y_r);
      valid_w            = pass;
      simd_Y_en          = valid_w;

    end // block: simd_PROC

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      valid_r <= 1'b0;
    else
      valid_r <= valid_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (simd_Y_en)
      simd_Y_r <= simd_Y_w;

endmodule // simd
