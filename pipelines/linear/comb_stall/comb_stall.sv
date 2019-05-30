//========================================================================== //
// Copyright (c) 2018, Stephen Henry
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

`include "libtb2.vh"
`include "libv2_pkg.vh"

module comb_stall #(parameter int N = 10, parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // In                                                                      //
   //                                                                         //
   //======================================================================== //

   , input          [W-1:0]                       in
   , input                                        in_vld
   //
   , output logic                                 in_accept

   //======================================================================== //
   //                                                                         //
   // Out                                                                     //
   //                                                                         //
   //======================================================================== //

   , output logic    [W-1:0]                      out_r
   , output                                       out_vld_r

   //======================================================================== //
   //                                                                         //
   // Stall                                                                   //
   //                                                                         //
   //======================================================================== //

   , input  logic   [N-1:0]                       stall_req
);

  //
  logic [N-1:1]                         vld_r;
  logic [N-1:1]                         vld_w;

  //
  logic [N-1:1][W-1:0]                  ucode_r;
  logic [N-1:1][W-1:0]                  ucode_w;

  //
  logic [N-1:0]                         adv;
  logic [N-1:1]                         en;
  logic [N-1:0]                         stall;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : pipe_PROC

      // The final (output) stage cannot be stalled (for DV simplicity).
      //
      stall [N - 1]  = 1'b0;

      //
      for (int i = N - 2; i >= 0; i--)
        stall [i]  = ((i == 0) ? 1'b1 :vld_r [i]) & (stall_req [i] | stall [i + 1]);
      
      //
      in_accept    = (~stall [0]);

      //
      for (int i = 0; i < N; i++)
        adv [i]  = ((i == 0) ? in_vld : vld_r [i]) & (~stall [i]);
                  
      //
      for (int i = 1; i < N; i++)
        en [i]   = adv [i - 1];
      
      //
      for (int i = 1; i < N; i++)
        vld_w [i]  = stall [i] ? vld_r [i] : adv [i - 1];
                    
      //
      for (int i = 1; i < N; i++)
        ucode_w [i]  = (i == 1) ? in : ucode_r [i - 1];
                 
      //
      out_r          = ucode_r [N - 1];
      out_vld_r      = vld_r [N - 1];

    end // block: pipe_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      vld_r <= 'b0;
    else begin
      for (int i = 1; i < N; i++)
        vld_r [i] <= vld_w [i];
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 1; i < N; i++)
      if (en [i])
        ucode_r [i] <= ucode_w [i];

endmodule // comb_stall
