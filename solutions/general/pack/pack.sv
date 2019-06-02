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

module pack #(parameter int N = 8, parameter int W = 32) (

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

   , input                                        in_pass
   , input [N-1:0][W-1:0]                         in_w
   , input [N-1:0]                                in_vld_w

   //======================================================================== //
   //                                                                         //
   // Out                                                                     //
   //                                                                         //
   //======================================================================== //

   , output logic                                 out_pass_r
   , output logic [N-1:0][W-1:0]                  out_r
   , output logic [N-1:0]                         out_vld_r
);

  //
  localparam int IDX_W  = $clog2(N);

  typedef logic [IDX_W:0] idx_t;

  //
  logic [N-1:0][W-1:0]                       out_r;
  logic [N-1:0][W-1:0]                       out_w;
  logic [N-1:0]                              out_en;

  //
  logic [N-1:0]                              out_vld_r;
  logic [N-1:0]                              out_vld_w;

  //
  logic [N-1:0][IDX_W:0]                     cnt;

  //
  logic                                      out_pass_w;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  function [IDX_W:0] popcnt(logic [N-1:0] n); begin
    popcnt      = '0;
    for (int i = 0; i < $bits(n); i++)
      popcnt += n[i] ? 'b1 : 'b0;
  end endfunction

  //
  function [N-1:0] sel_n(logic [N-1:0] n, int j); begin
    sel_n  = '0;
    for (int i = 0; i <= j; i++)
      sel_n [i]  = n [i];
  end endfunction
       
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cntrl_PROC

      // Compute the index of the i'th channel in the output
      // vector. This is carried out using a population count of the
      // preceeding valid slots (zero indexed).
      //
      for (int i = 0; i < N; i++)
        cnt [i]    = popcnt(sel_n(in_vld_w, i));

      // NxN crossbar to issue each valid input channel to its
      // corresponding output slot.
      //
      for (int i = 0; i < N; i++) begin

        //
        out_w [i]  = '0;
        for (int j = i; j < N; j++)
          out_w [i] |= (in_vld_w [j] & (cnt [j] == idx_t'(i + 1))) ? in_w [j] : 'b0;

        end // for (int i = 0; i < N; i++)

      //
      out_pass_w     = in_pass;

      // Compute a unary mask denoting the locations in the output
      // vector containing valid state. This is performed using a
      // population count of all valid entries and by forming the
      // corresponding unary-encoded representation.
      //
      out_vld_w      = ~('1 << popcnt(in_vld_w));

      //
      out_en         = out_vld_w;

    end // block: cntrl_PROC

  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      out_vld_r <= '0;
    else
      out_vld_r <= out_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < N; i++)
      if (out_en [i])
        out_r [i] <= out_w [i];

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      out_pass_r <= '0;
    else
      out_pass_r <= out_pass_w;
  
endmodule // pack

  
