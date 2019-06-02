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

`include "spsram_pkg.vh"

module stack #(parameter int N = 16, parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //------------------------------------------------------------------------ //
   //
   , input                                   cmd_vld
   , input                                   cmd_push
   , input        [W-1:0]                    cmd_push_dat
   , input                                   cmd_clr
   //
   , output logic [W-1:0]                    cmd_pop_dat_r

   //======================================================================== //
   //                                                                         //
   // Status                                                                  //
   //                                                                         //
   //======================================================================== //

   //------------------------------------------------------------------------ //
   //
   , output logic                            empty_r
   , output logic                            full_r
);

  typedef logic [W-1:0] w_t;
  typedef logic [$clog2(N)-1:0] n_t;

  //
  `SPSRAM_SIGNALS(spram__, W, $clog2(N));
  //
  logic                           empty_w;
  logic                           full_w;
  logic                           empty_mem_r;
  logic                           empty_mem_w;  
  //
  n_t                             rd_ptr_r;
  n_t                             rd_ptr_w;
  logic                           rd_ptr_en;
  //
  n_t                             wr_ptr_r;
  n_t                             wr_ptr_w;
  logic                           wr_ptr_en;
  //
  w_t                             cmd_pop_dat_r;
  w_t                             cmd_pop_dat_w;
  logic                           cmd_pop_dat_en;
  logic                           cmd_pop_dat_vld_r;
  logic                           cmd_pop_dat_vld_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : status_PROC

      //
      cmd_pop_dat_en  = cmd_vld;
      cmd_pop_dat_w   = cmd_push ? cmd_push_dat : spram__dout;

      //
      casez ({cmd_pop_dat_vld_r, cmd_vld, cmd_push, cmd_clr})
        4'b?_1?1: wr_ptr_w  = 'b0;
        4'b1_110: wr_ptr_w  = wr_ptr_r + 'b1;
        4'b1_100: wr_ptr_w  = (wr_ptr_r != '0) ? wr_ptr_r - 'b1 : wr_ptr_r;
        default:  wr_ptr_w  = wr_ptr_r;
      endcase // casez ({cmd_pop_dat_vld_r, cmd_vld, cmd_push, cmd_cld})

      //
      wr_ptr_en  = cmd_vld;
      
      //
      casez ({cmd_pop_dat_vld_r, cmd_vld, cmd_push, cmd_clr})
        4'b?_1?1: rd_ptr_w  = 'b0;
        4'b1_110: rd_ptr_w  = empty_mem_r ? rd_ptr_r : (rd_ptr_r + 'b1);
        4'b1_100: rd_ptr_w  = (rd_ptr_r != '0) ? (rd_ptr_r - 'b1) : rd_ptr_r;
        default:  rd_ptr_w  = rd_ptr_r;
      endcase // casez ({cmd_vld, cmd_push, cmd_pop_dat_vld_r})

      //
      rd_ptr_en    = cmd_vld;

      //
      empty_mem_w  = (wr_ptr_w == '0);

      //
      casez ({cmd_vld, cmd_push, cmd_clr})
        3'b1_10: cmd_pop_dat_vld_w  = 'b1;
        3'b1_00: cmd_pop_dat_vld_w  = ~empty_mem_r;
        3'b1_?1: cmd_pop_dat_vld_w  = 'b0;
        default: cmd_pop_dat_vld_w  = cmd_pop_dat_vld_r;
      endcase

      //
      empty_w      = ~cmd_pop_dat_vld_w;
      full_w       = (rd_ptr_w == n_t'(N-1));

      //
      spram__en    = cmd_vld & cmd_pop_dat_vld_r;
      spram__wen   = cmd_vld & cmd_pop_dat_vld_r & cmd_push;
      spram__addr  = cmd_push ? wr_ptr_r : rd_ptr_r;
      spram__din   = cmd_pop_dat_r;

    end // block: status_PROC

  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (cmd_pop_dat_en)
      cmd_pop_dat_r <= cmd_pop_dat_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmd_pop_dat_vld_r <= 'b0;
    else
      cmd_pop_dat_vld_r <= cmd_pop_dat_vld_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      rd_ptr_r <= 'b0;
    else if (rd_ptr_en)
      rd_ptr_r <= rd_ptr_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      wr_ptr_r <= 'b0;
    else if (wr_ptr_en)
      wr_ptr_r <= wr_ptr_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) begin
      empty_r     <= 'b1;
      empty_mem_r <= 'b1;
      full_r      <= 'b0;
    end else begin
      empty_r     <= empty_w;
      empty_mem_r <= empty_mem_w;
      full_r      <= full_w;
    end

  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  spsram #(.W(W), .N(N), .ASYNC_DOUT('b1)) u_spram (
    //
      .clk          (clk                )
    //
    , .en           (spram__en          )
    , .wen          (spram__wen         )
    , .addr         (spram__addr        )
    , .din          (spram__din         )
    //
    , .dout         (spram__dout        )
  );

endmodule
