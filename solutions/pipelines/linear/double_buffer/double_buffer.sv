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

module double_buffer #(parameter int N = 10, parameter int W = 32) (

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
  logic [N-1:0]                         A__in_vld;
  logic [N-1:0][W-1:0]                  A__in_w;

  //
  logic [N-1:0]                         B__out_vld_r;
  logic [N-1:0][W-1:0]                  B__out_r;

  //
  logic [N-1:0]                         B__stall;
  logic [N-1:0]                         A__stall_r;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : pipe_PROC

      //
      A__in_vld [0]  = in_vld;
      A__in_w [0]    = in;

      //
      in_accept      = (~A__stall_r [0]);

      for (int i = 1; i < N; i++) begin

        //
        A__in_vld [i]  = (~stall_req [i - 1]) & B__out_vld_r [i - 1];
        A__in_w [i]    = B__out_r [i - 1];

        //
        B__stall [i - 1]   = (stall_req [i - 1] | A__stall_r [i]);

      end // for (int i = 0; i < N; i++)

      //
      out_vld_r        = B__out_vld_r [N - 1];
      out_r            = B__out_r [N - 1];

    end // block: pipe_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  generate for (genvar g = 0; g < N; g++) begin

    dblbuf #(.W(W)) u_dblbuf (
      //
        .clk             (clk                )
      , .rst             (rst                )
      //
      , .A__in_vld       (A__in_vld [g]      )
      , .A__in_w         (A__in_w [g]        )
      //
      , .B__out_vld_r    (B__out_vld_r [g]   )
      , .B__out_r        (B__out_r [g]       )
      //
      , .B__stall        (B__stall [g]       )
      , .A__stall_r      (A__stall_r [g]     )
    );
    
  end endgenerate

endmodule // comb_stall
