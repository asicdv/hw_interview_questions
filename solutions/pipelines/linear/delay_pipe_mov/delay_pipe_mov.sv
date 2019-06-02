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

module delay_pipe_mov #(parameter int N = 5, parameter int W = 32) (

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

   //======================================================================== //
   //                                                                         //
   // Out                                                                     //
   //                                                                         //
   //======================================================================== //

   , output logic    [W-1:0]                      out_r
   , output                                       out_vld_r
);

  //
  logic [N-1:0]                         vld_r;
  logic [N-1:0]                         vld_w;

  //
  logic [N-1:0][W-1:0]                  dat_r;
  logic [N-1:0][W-1:0]                  dat_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  generate if (N > 0) begin

    always_comb
      begin : out_PROC

        //
        dat_w [0]  = in;
        vld_w [0]  = in_vld;

        for (int i = 1; i < N; i++) begin

          //
          dat_w [i]  = dat_r [i - 1];
          vld_w [i]  = vld_r [i - 1];

        end // for (int i = 1; i < N; i++)

        out_r        = dat_r [N - 1];
        out_vld_r    = vld_r [N - 1];

      end // block: out_PROC

  end endgenerate

  // ------------------------------------------------------------------------ //
  //
  generate if (N == 0) begin

    always_comb
      begin : out_PROC

        out_r      = in;
        out_vld_r  = in_vld;

      end // block: out_PROC
    
  end endgenerate
  
  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  generate if (N > 0) begin

    //
    always_ff @(posedge clk)
      if (rst)
        vld_r <= '0;
      else
        vld_r <= vld_w;

    //
    always_ff @(posedge clk)
      for (int i = 0; i < N; i++)
        if (vld_w [i])
          dat_r [i] <= dat_w [i];

  end endgenerate
    
endmodule
