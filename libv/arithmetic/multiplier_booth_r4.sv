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

module multiplier_booth_r4 #(parameter int W = 16) (
  //
    input logic [W-1:0]                 a
  , input logic [W-1:0]                 b
  //
  , output logic [2*W-1:0]              y
);

  parameter int               RADIX  = 4;

  //   00001111222233334
  // -------------------
  // 1               xxx
  // 2             xxx
  // 3           xxx
  // 4         xxx
  // 5       xxx
  // 6     xxx
  // 7   xxx
  // 8 xxx
  // -------------------

  function int compute_r4_rounds(int W); begin
    int w_  = W + 1;
    int i   = 0;
    while (w_ > 0) begin
      w_ -= 2;
      i++;
    end
    return i;
  end endfunction

  localparam int ROUNDS  = compute_r4_rounds(W);

  // Word type
  typedef logic [W-1:0]            w_t;
  // Result type
  typedef logic [2*W-1:0]          r_t;
  // Partial product control word
  typedef logic [W:0]              p_t;
  // CSA type
  typedef struct packed { r_t [ROUNDS-1:0] r; } csa_t;

  // Helper function to sign extend w_t into r_t
  //
  function r_t sign_extend(w_t x); begin
    return { {$bits(r_t) - $bits(w_t){x[$bits(w_t)-1]}}, x };
  end endfunction

  //
  function r_t asl(r_t x, int i); begin
    return (x << i);
  end endfunction
    
  //
  r_t             inc_x;
  r_t             inc_y;
  r_t             a_bth [8];
  //
  csa_t           csa_x;
  r_t             csa_y1;
  r_t             csa_y2;

  // ------------------------------------------------------------------------ //
  //
  always_comb inc_x  = sign_extend(~a);
  
  // ------------------------------------------------------------------------ //
  // Compute Booth Radix-4 Factors
  always_comb
    begin : booth_r4_recoding_PROC

      // 000100|0
      //     xx x
      //   xxx
      // xxx
      a_bth [0]  = '0;
      a_bth [1]  = r_t'(a);
      a_bth [2]  = r_t'(a);
      a_bth [3]  = r_t'(a) << 1;
      a_bth [4]  = inc_y << 1;
      a_bth [5]  = inc_y;
      a_bth [6]  = inc_y;
      a_bth [7]  = '0;

    end // block: booth_r4_recoding_PROC

  // ------------------------------------------------------------------------ //
  // Derive partial products
  always_comb
    begin : partial_products_PROC

      //
      p_t p_cntrl    = {b, 1'b0};
      int j          = 0;

      //
      csa_x          = '0;
      for (int i = 0; i < ROUNDS; i++) begin
        // This arithmetic shift is a elaboration-time constant therefore
        // does not (should not) result in any logical overhead.
        //
        csa_x.r [i]  = asl(a_bth [p_cntrl [j +: 3]], 2 * i);
        j           += 2;
      end

    end // block: partial_products_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  csa #(.N(ROUNDS), .W($bits(r_t))) u_csa (.x(csa_x), .y1(csa_y1), .y2(csa_y2));

  // ------------------------------------------------------------------------ //
  //
  increment #(.W($bits(r_t))) u_increment (.x(inc_x), .y(inc_y));

  // ------------------------------------------------------------------------ //
  //
  fast_adder #(.W($bits(r_t))) u_cla (.a(csa_y1), .b(csa_y2), .cin(1'b0), .y(y), .cout());

endmodule // multilipler_booth_r4
