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

module simd_shifter_naive (
  //
    input simd_shifter_pkg::word_t           in
  , input simd_shifter_pkg::mode_t           mode
  , input simd_shifter_pkg::op_t             op
  , input simd_shifter_pkg::shift_t [7:0]    shift
  //
  , output simd_shifter_pkg::word_t          out
);

  // ----------------------------------------------------------------------- //
  //
  always_comb begin : shift_naive_PROC

    // Defaults
    out  = in;

    case (mode)

      simd_shifter_pkg::MODE_8B: begin
        case (op)
          simd_shifter_pkg::OP_SLL: out  = ($signed(in) << shift [0]);
          simd_shifter_pkg::OP_SRL: out  = ($signed(in) >> shift [0]);
          simd_shifter_pkg::OP_SLA: out  = ($signed(in) <<< shift [0]);
          simd_shifter_pkg::OP_SRA: out  = ($signed(in) >>> shift [0]);
        endcase
      end

      simd_shifter_pkg::MODE_4B: begin

        for (int i = 0; i < 2; i++) begin
          case (op)
            simd_shifter_pkg::OP_SLL:
              out.w32 [i]  = ($signed(in.w32 [i]) << shift [i]);
            simd_shifter_pkg::OP_SRL:
              out.w32 [i]  = ($signed(in.w32 [i]) >> shift [i]);
            simd_shifter_pkg::OP_SLA:
              out.w32 [i]  = ($signed(in.w32 [i]) <<< shift [i]);
            simd_shifter_pkg::OP_SRA:
              out.w32 [i]  = ($signed(in.w32 [i]) >>> shift [i]);
          endcase // case (op)
        end
      end

      simd_shifter_pkg::MODE_2B: begin

        for (int i = 0; i < 4; i++) begin
          case (op)
            simd_shifter_pkg::OP_SLL:
              out.w16[i]  = ($signed(in.w16[i]) << shift [i]);
            simd_shifter_pkg::OP_SRL:
              out.w16[i]  = ($signed(in.w16[i]) >> shift [i]);
            simd_shifter_pkg::OP_SLA:
              out.w16[i]  = ($signed(in.w16[i]) <<< shift [i]);
            simd_shifter_pkg::OP_SRA:
              out.w16[i]  = ($signed(in.w16[i]) >>> shift [i]);
          endcase
        end
      end

      simd_shifter_pkg::MODE_1B: begin
        for (int i = 0; i < 8; i++) begin
          case (op)
            simd_shifter_pkg::OP_SLL:
              out.w8[i]  = ($signed(in.w8[i]) << shift [i]);
            simd_shifter_pkg::OP_SRL:
              out.w8[i]  = ($signed(in.w8[i]) >> shift [i]);
            simd_shifter_pkg::OP_SLA:
              out.w8[i]  = ($signed(in.w8[i]) <<< shift [i]);
            simd_shifter_pkg::OP_SRA:
              out.w8[i]  = ($signed(in.w8[i]) >>> shift [i]);
          endcase
        end
      end

    endcase

  end // block: shift_naive_PROC

endmodule // simd_shifter_naive
