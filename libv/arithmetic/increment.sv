// ==================================================================== //
// Copyright (c) 2017, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in
//   the documentation and/or other materials provided with the
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
// ==================================================================== //

`include "libv_pkg.vh"

module increment #(
  //
    parameter int W = 32
  //
  , parameter libv_pkg::increment_t T = libv_pkg::INFERED
) (
    input logic [W-1:0]                 x
  //
  , output logic [W-1:0]                y
);

  typedef logic [W-1:0] w_t;
  typedef logic [$clog2(W)-1:0] l_t;

  //
  l_t clz_y;
  w_t mask_y;

  // ------------------------------------------------------------------------ //
  //
  generate if (T == libv_pkg::INFERED)
    
    always_comb
      begin : do_increment_PROC
        y  = x + 'b1;
      end // block: do_increment_PROC

  else if (T == libv_pkg::INVZERO) begin

    // Compute indices of first '0' in input vector.
    //
    clz #(.W(W)) u_clz (.x(~x), .y(clz_y));

    // Derive mask from index.
    //
    mask #(.W(W)) u_mask (.x(clz_y), .y(mask_y));

    // Conditional invert to compute final increment.
    //
    always_comb y  = x ^ mask_y;

  end endgenerate

endmodule // increment
