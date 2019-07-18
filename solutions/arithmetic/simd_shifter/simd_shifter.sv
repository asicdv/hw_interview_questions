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

`include "simd_shifter_pkg.vh"

module simd_shifter (
  //
    input simd_shifter_pkg::word_t           in
  , input simd_shifter_pkg::mode_t           mode
  , input simd_shifter_pkg::op_t             op
  , input simd_shifter_pkg::shift_t [3:0]    shift
  //
  , output simd_shifter_pkg::word_t          out
);
  import simd_shifter_pkg::*;

  word_t       word_rnd_6;
  word_t       word_rnd_5;
  word_t       word_rnd_4;
  word_t       word_rnd_3;
  word_t       word_rnd_2;
  word_t       word_rnd_1;
  word_t       word_rnd_0;

  function word_t shl (word_t w, int idx); begin
    return (idx >= 0) ? w [idx] : 'b0;
  end endfunction

  function word_t shr (word_t w, int idx); begin
    case (op)
      OP_SRL:  shr  = (idx > 63) ? 'b0 : w[idx];
      OP_SRA:  shr  = (idx > 63) ? in [63] : w[idx];
      default: shr  = w;
    endcase
  end endfunction

  // ----------------------------------------------------------------------- //
  //
  always_comb begin : shifter_PROC


    // Round 6
    word_rnd_6  = in;

    // Round 5
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [5], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_5 [i]  = shl(word_rnd_6, i - 32);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_5 [i]  = shr(word_rnd_6, i + 32);

        default:        word_rnd_5 [i]  = word_rnd_6 [i];
      endcase // case ({shift [0] [5], op})
    end

    // Round 4
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [4], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_4 [i]  = shl(word_rnd_5, i - 16);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_4 [i]  = shr(word_rnd_5, i + 16);

        default:        word_rnd_4 [i]  = word_rnd_5 [i];
      endcase // case ({shift [0] [4], op})
    end

    // Round 3
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [3], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_3 [i]  = shl(word_rnd_4, i - 8);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_3 [i]  = shr(word_rnd_4, i + 8);

        default:        word_rnd_3 [i]  = word_rnd_4 [i];
      endcase // case ({shift [0] [3], op})
    end

    // Round 2
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [2], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_2 [i]  = shl(word_rnd_3, i - 4);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_2 [i]  = shr(word_rnd_3, i + 4);

        default:        word_rnd_2 [i]  = word_rnd_3 [i];
      endcase // case ({shift [0] [2], op})
    end

    // Round 1
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [1], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_1 [i]  = shl(word_rnd_2, i - 2);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_1 [i]  = shr(word_rnd_2, i + 2);

        default:        word_rnd_1 [i]  = word_rnd_2 [i];
      endcase // case ({shift [0] [1], op})
    end

    // Round 0
    for (int i = 0; i < 64; i++) begin
      case ({shift [0] [0], op})
        {1'b1, OP_SLL},
        {1'b1, OP_SLA}: word_rnd_0 [i]  = shl(word_rnd_1, i - 1);

        {1'b1, OP_SRL},
        {1'b1, OP_SRA}: word_rnd_0 [i]  = shr(word_rnd_1, i + 1);

        default:        word_rnd_0 [i]  = word_rnd_1 [i];
      endcase // case ({shift [0] [0], op})
    end

    out  = word_rnd_0;

  end // block: shifter_PROC

endmodule // simd_shifter

