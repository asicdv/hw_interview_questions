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

// References:
//
//  Parhami, B., "Computer Arthimetic: Algorithms and Hardware Designs"
//
//  [1] Fig. 6.10
//
//  [2] Fig. 6.9 
//
//  [3] Fig. 6.11
//


`include "libv_pkg.vh"

// TODO: change name to cla.sv
module fast_adder #(parameter int W = 32,
                    parameter libv_pkg::adder_carry_t CARRY_TYPE = libv_pkg::INFERRED) (

   //======================================================================== //
   //                                                                         //
   // Adder                                                                   //
   //                                                                         //
   //======================================================================== //

   //
     input logic [W-1:0]                     a
   , input logic [W-1:0]                     b
   , input                                   cin
   //
   , output logic [W-1:0]                    y
   , output logic                            cout
);

  typedef logic [W-1:0]            w_t;
  typedef logic [W:0]              w1_t;

  //
  w_t p, g;
  w1_t c;

  // ------------------------------------------------------------------------ //
  //
  generate if (CARRY_TYPE == libv_pkg::INFERRED) begin

    // Simple inferred adder
    always_comb {cout, y}  = a + b + w_t'(cin);

  end else begin

    always_comb
      begin : bk_PROC

        // Propagate
        p     = (a ^ b);

        // Generate
        g     = (a & b);

        // Sum
        y     = (a ^ b ^ w_t'(c));
        cout  = c [W];
        

      end // block: bk_PROC
    
  end endgenerate

  // ------------------------------------------------------------------------ //
  //
  generate if (CARRY_TYPE == libv_pkg::KOGGE_STONE) begin

    // Kogge-Stone Carry Chain [1]
    //
    carry_chain_kogge_stone #(.W(W)) u_cc (.p(p), .g(g), .c(c));

    
  end endgenerate

  // ------------------------------------------------------------------------ //
  //
  generate if (CARRY_TYPE == libv_pkg::BRENT_KUNG) begin

    // Brent-Kung Carry Chain [2]
    //
    carry_chain_brent_kung #(.W(W)) u_cc (.p(p), .g(g), .c(c));
    
  end endgenerate

  // ------------------------------------------------------------------------ //
  //
  generate if (CARRY_TYPE == libv_pkg::HYBRID) begin

    // Hybrid Breng-Kung/Kogge-Stone Chain [3]
    //
    carry_chain_hybrid #(.W(W)) u_cc (.p(p), .g(g), .c(c));
    
  end endgenerate


endmodule // fast_adder

