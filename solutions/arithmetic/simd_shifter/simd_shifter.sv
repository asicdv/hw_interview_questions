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
  , input simd_shifter_pkg::shift_t [7:0]    shift
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

  // ----------------------------------------------------------------------- //
  //
  function word_t shl (word_t w, int b, int idx); begin
    case (mode)
      MODE_1B: begin
        case (b)
          7:       shl = (idx >= 56) ? w [idx] : 'b0;
          6:       shl = (idx >= 48) ? w [idx] : 'b0;
          5:       shl = (idx >= 40) ? w [idx] : 'b0;
          4:       shl = (idx >= 32) ? w [idx] : 'b0;
          3:       shl = (idx >= 24) ? w [idx] : 'b0;
          2:       shl = (idx >= 16) ? w [idx] : 'b0;
          1:       shl = (idx >=  8) ? w [idx] : 'b0;
          0:       shl = (idx >=  0) ? w [idx] : 'b0;
          default: shl = (idx >=  0) ? w [idx] : 'b0;
        endcase
      end
      MODE_2B: begin
        case (b)
          6,7:     shl = (idx >= 48) ? w [idx] : 'b0;
          4,5:     shl = (idx >= 32) ? w [idx] : 'b0;
          2,3:     shl = (idx >= 16) ? w [idx] : 'b0;
          0,1:     shl = (idx >=  0) ? w [idx] : 'b0;
          default: shl = (idx >=  0) ? w [idx] : 'b0;
        endcase
      end
      MODE_4B: begin
        case (b)
          7,6,5,4: shl  = (idx >= 32) ? w [idx] : 'b0;
          3,2,1,0: shl  = (idx >=  0) ? w [idx] : 'b0;
          default: shl  = (idx >=  0) ? w [idx] : 'b0;
        endcase
      end
      MODE_8B: begin
        case (b)
          default: shl = (idx >=  0) ? w [idx] : 'b0;
        endcase
      end
    endcase // case (mode)
  end endfunction

  // ----------------------------------------------------------------------- //
  //
  function word_t shr (word_t w, int b, int idx); begin
    logic is_arith  = (op == OP_SRA);

    case (mode)
      MODE_1B: begin
        case (b)
          7:       shr = (idx >= 64) ? (is_arith ? in [63] : 'b0) : w [idx];
          6:       shr = (idx >= 56) ? (is_arith ? in [55] : 'b0) : w [idx];
          5:       shr = (idx >= 48) ? (is_arith ? in [47] : 'b0) : w [idx];
          4:       shr = (idx >= 40) ? (is_arith ? in [39] : 'b0) : w [idx];
          3:       shr = (idx >= 32) ? (is_arith ? in [31] : 'b0) : w [idx];
          2:       shr = (idx >= 24) ? (is_arith ? in [23] : 'b0) : w [idx];
          1:       shr = (idx >= 16) ? (is_arith ? in [15] : 'b0) : w [idx];
          0:       shr = (idx >=  8) ? (is_arith ? in [ 7] : 'b0) : w [idx];
          default: shr = (idx >=  8) ? (is_arith ? in [ 7] : 'b0) : w [idx];
        endcase
      end
      MODE_2B: begin
        case (b)
          7,6:     shr = (idx >= 64) ? (is_arith ? in [63] : 'b0) : w [idx];
          5,4:     shr = (idx >= 48) ? (is_arith ? in [47] : 'b0) : w [idx];
          3,2:     shr = (idx >= 32) ? (is_arith ? in [31] : 'b0) : w [idx];
          1,0:     shr = (idx >= 16) ? (is_arith ? in [15] : 'b0) : w [idx];
          default: shr = (idx >= 16) ? (is_arith ? in [15] : 'b0) : w [idx];
        endcase
      end
      MODE_4B: begin
        case (b)
          7,6,5,4: shr = (idx >= 64) ? (is_arith ? in [63] : 'b0) : w [idx];
          3,2,1,0: shr = (idx >= 32) ? (is_arith ? in [31] : 'b0) : w [idx];
          default: shr = (idx >= 32) ? (is_arith ? in [31] : 'b0) : w [idx];
        endcase
      end
      MODE_8B: begin
        case (b)
          default: shr = (idx >= 64) ? (is_arith ? in [63] : 'b0) : w [idx];
        endcase // case (b)
      end
    endcase // case (mode)
  end endfunction

  shift_t [7:0] shift_b;
  
  // ----------------------------------------------------------------------- //
  //
  always_comb begin : broadcast_PROC

    //
    case (mode)
      MODE_2B: begin
        shift_b [7]  = shift [3];
        shift_b [6]  = shift [3];
        shift_b [5]  = shift [2];
        shift_b [4]  = shift [2];
        shift_b [3]  = shift [1];
        shift_b [2]  = shift [1];
        shift_b [1]  = shift [0];
        shift_b [0]  = shift [0];
      end
      MODE_4B: begin
        shift_b [7]  = shift [1];
        shift_b [6]  = shift [1];
        shift_b [5]  = shift [1];
        shift_b [4]  = shift [1];
        shift_b [3]  = shift [0];
        shift_b [2]  = shift [0];
        shift_b [1]  = shift [0];
        shift_b [0]  = shift [0];
      end
      MODE_8B: begin
        shift_b [7]  = shift [0];
        shift_b [6]  = shift [0];
        shift_b [5]  = shift [0];
        shift_b [4]  = shift [0];
        shift_b [3]  = shift [0];
        shift_b [2]  = shift [0];
        shift_b [1]  = shift [0];
        shift_b [0]  = shift [0];
      end
      default: begin
        shift_b [7]  = shift [7];
        shift_b [6]  = shift [6];
        shift_b [5]  = shift [5];
        shift_b [4]  = shift [4];
        shift_b [3]  = shift [3];
        shift_b [2]  = shift [2];
        shift_b [1]  = shift [1];
        shift_b [0]  = shift [0];
      end
    endcase // case (mode)

  end // block: broadcast_PROC
  
  // ----------------------------------------------------------------------- //
  //
  always_comb begin : shifter_PROC

    // Round 6
    word_rnd_6  = in;

    // Round 5
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [5], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_5 [i]  = shl(word_rnd_6, b, i - 32);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_5 [i]  = shr(word_rnd_6, b, i + 32);

          default:        word_rnd_5 [i]  = word_rnd_6 [i];
        endcase // case ({shift_b [b] [5], op})
      end // for (int i = (8 * b); i < 8 * (b + 1); i++)
    end

    // Round 4
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [4], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_4 [i]  = shl(word_rnd_5, b, i - 16);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_4 [i]  = shr(word_rnd_5, b, i + 16);

          default:        word_rnd_4 [i]  = word_rnd_5 [i];
        endcase // case ({shift_b [b] [4], op})
      end
    end

    // Round 3
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [3], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_3 [i]  = shl(word_rnd_4, b, i - 8);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_3 [i]  = shr(word_rnd_4, b, i + 8);

          default:        word_rnd_3 [i]  = word_rnd_4 [i];
        endcase // case ({shift_b [b] [3], op})
      end
    end

    // Round 2
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [2], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_2 [i]  = shl(word_rnd_3, b, i - 4);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_2 [i]  = shr(word_rnd_3, b, i + 4);

          default:        word_rnd_2 [i]  = word_rnd_3 [i];
        endcase // case ({shift_b [b] [2], op})
      end
    end

    // Round 1
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [1], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_1 [i]  = shl(word_rnd_2, b, i - 2);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_1 [i]  = shr(word_rnd_2, b, i + 2);

          default:        word_rnd_1 [i]  = word_rnd_2 [i];
        endcase // case ({shift_b [b] [1], op})
      end
    end

    // Round 0
    for (int b = 0; b < 8; b++) begin
      for (int i = (8 * b); i < 8 * (b + 1); i++) begin
        case ({shift_b [b] [0], op})
          {1'b1, OP_SLL},
          {1'b1, OP_SLA}: word_rnd_0 [i]  = shl(word_rnd_1, b, i - 1);

          {1'b1, OP_SRL},
          {1'b1, OP_SRA}: word_rnd_0 [i]  = shr(word_rnd_1, b, i + 1);

          default:        word_rnd_0 [i]  = word_rnd_1 [i];
        endcase // case ({shift_b [b] [0], op})
      end
    end

    out  = word_rnd_0;

  end // block: shifter_PROC

endmodule // simd_shifter

