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

module delay_pipe_static #(parameter int N = 5, parameter int W = 32) (

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

  typedef struct packed {
    logic                vld;
    logic [W-1:0]        dat;
  } pipe_t;

  //
  pipe_t  [N-1:0]        pipe_r;
  pipe_t  [N-1:0]        pipe_w;
  logic [N-1:0]          pipe_en;

  typedef logic [$clog2(N)-1:0] ptr_t;

  //
  ptr_t                  rd_ptr_r;
  ptr_t                  rd_ptr_w;

  //
  ptr_t                  wr_ptr_r;
  ptr_t                  wr_ptr_w;

  //
  logic [W-1:0]          out_w;
  logic                  out_vld_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  generate if (N > 0) begin

    function ptr_t inc_and_clip(ptr_t n); begin
      if (n == ptr_t'(N - 1))
        inc_and_clip  = 'b0;
      else
        inc_and_clip  = n + 'b1;
    end endfunction

    always_comb
      begin : out_PROC

        //
        rd_ptr_w            = inc_and_clip(rd_ptr_r);
        wr_ptr_w            = inc_and_clip(wr_ptr_r);

        //
        pipe_en             = 'b0;
        pipe_en [wr_ptr_r]  = in_vld;

        //
        out_vld_w           = pipe_r [rd_ptr_r].vld;
        out_w               = pipe_r [rd_ptr_r].dat;

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
        out_vld_r <= 'b0;
      else
        out_vld_r <= out_vld_w;

    //
    always_ff @(posedge clk)
      if (out_vld_w)
        out_r <= out_w;

    //
    always_ff @(posedge clk)
      if (rst) begin
        for (int i = 0; i < N; i++)
          pipe_r [i].vld <= 'b0;
      end else begin
        if (pipe_en [wr_ptr_r])
          pipe_r [wr_ptr_r] <= '{vld:in_vld, dat:in};
        else
          // If no incoming data, kill valid for the pipeline stage but
          // retain existing data to reduce power.
          pipe_r [wr_ptr_r].vld <= 'b0;
      end

    //
    always_ff @(posedge clk)
      if (rst)
        rd_ptr_r <= 'b0;
      else
        rd_ptr_r <= rd_ptr_w;
    
    //
    always_ff @(posedge clk)
      if (rst)
        wr_ptr_r <= ptr_t'(N - 1);
      else
        wr_ptr_r <= wr_ptr_w;

  end endgenerate
    
endmodule
