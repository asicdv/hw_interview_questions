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

module simd_shifter_tb (
  //
    input                                    clk
  , input                                    rst
  //
  , input [63:0]                             in
  , input simd_shifter_pkg::mode_t           mode
  , input simd_shifter_pkg::op_t             op
  , input simd_shifter_pkg::shift_t [7:0]    shift
  //
  , output logic                             fail
);

  //
  simd_shifter_pkg::word_t    simd_shifter__out;
  simd_shifter_pkg::word_t    simd_shifter_naive__out;

  // ----------------------------------------------------------------------- //
  //
  simd_shifter u_simd_shifter (
    //
      .in                (in                      )
    , .mode              (mode                    )
    , .op                (op                      )
    , .shift             (shift                   )
    //
    , .out               (simd_shifter__out       )
  );

  // ----------------------------------------------------------------------- //
  //
  simd_shifter_naive u_simd_shifter_naive (
    //
      .in                (in                      )
    , .mode              (mode                    )
    , .op                (op                      )
    , .shift             (shift                   )
    //
    , .out               (simd_shifter_naive__out )
  );
  
  // ----------------------------------------------------------------------- //
  //
  always_comb fail = (simd_shifter__out != simd_shifter_naive__out);

endmodule
