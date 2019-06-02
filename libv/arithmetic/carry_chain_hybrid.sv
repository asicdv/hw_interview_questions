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

module carry_chain_hybrid #(parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Generate/Propagate                                                      //
   //                                                                         //
   //======================================================================== //

   //
     input logic [W-1:0]                          p
   , input logic [W-1:0]                          g

   //======================================================================== //
   //                                                                         //
   // Carry-In                                                                //
   //                                                                         //
   //======================================================================== //
     
   //  
   , output logic [W:0]                           c
);

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : bk_PROC

      libv_pkg::pg_t pg [W];

      // Initialize var
      for (int i = 0; i < W; i++)
        pg [i]    = { p[i], g[i] };

      // Brent-Kung St 1
      //
      for (int i = 1; i < W; i += 2)
        pg [i]    = libv_pkg::lkpg(pg [i], pg [i - 1]);
      
      // Kogge-Stone
      //
      for (int level = 1; level < 1 + $clog2(W / 2); level++) begin
        for (int hi = 1; hi < W; hi += 2) begin
          int lo     = hi - (1 << level);
          if (lo >= 0)
            pg [hi]    = libv_pkg::lkpg(pg [hi], pg [lo]);
        end
      end
                 
      // Brent-Kung St 2
      //
      for (int i = 2; i < W; i += 2)
        pg [i]  = libv_pkg::lkpg(pg [i], pg [i - 1]);
                 
      // Update output vector
      c  = '0;
      for (int i = 1; i < W + 1; i++) 
        c [i]  = pg [i - 1].g;

    end // block: bk_PROC

endmodule // carry_chain_hybrid
