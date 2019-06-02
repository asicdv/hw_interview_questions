//========================================================================== //
// Copyright (c) 2016, Stephen Henry
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

module rf #(

   //
     parameter int W = 32
     
   //
   , parameter int N = 8

   //
   , parameter bit FLOP_OUT = '0

   //
   , parameter int WR_N = 1

   //
   , parameter int RD_N = 1
) (
   //
     input                              clk
   , input                              rst // TBD: Future compatibility.
   //
   , input   [RD_N-1:0][$clog2(N)-1:0]  ra
   , input   [RD_N-1:0]                 ren
   //
   , output logic [RD_N-1:0][W-1:0]     rdata
   //
   , input   [WR_N-1:0][$clog2(N)-1:0]  wa
   , input   [WR_N-1:0]                 wen
   , input   [WR_N-1:0][W-1:0]          wdata
);

   logic [W-1:0] mem_r [N-1:0];

   // ------------------------------------------------------------------------ //
   //
   generate if (!FLOP_OUT)

     always_comb
       for (int i = 0; i < RD_N; i++)
         rdata [i] = mem_r [ra[i]];

   endgenerate

   // ------------------------------------------------------------------------ //
   //
   generate if (FLOP_OUT)
     
     always_ff @(posedge clk)
       for (int i = 0; i < RD_N; i++) begin
         if (ren [i])
           rdata [i] <= mem_r [ra[i]];
       end

   endgenerate

   // ------------------------------------------------------------------------ //
   //
   always_ff @(posedge clk)
     for (int i = 0; i < WR_N; i++) begin
       if (wen [i])
         mem_r [wa [i]] <= wdata [i];
     end

endmodule // rf
