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

module fifo_basic #(
     parameter integer W = 32
   , parameter integer N = 16
) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Push Interface                                                          //
   //                                                                         //
   //======================================================================== //

   , input                                   push
   , input [W-1:0]                           push_data

   //======================================================================== //
   //                                                                         //
   // Pop Interface                                                           //
   //                                                                         //
   //======================================================================== //

   , input                                   pop

   , output logic [W-1:0]                    pop_data

   //======================================================================== //
   //                                                                         //
   // Control/Status Interface                                                //
   //                                                                         //
   //======================================================================== //

   , input                                   flush
   //
   , output logic                            empty_r
   , output logic                            full_r
);

  // ======================================================================== //
  //                                                                          //
  // Wires                                                                    //
  //                                                                          //
  // ======================================================================== //

  //
  logic                                 empty_w;
  logic                                 full_w;

  //
  typedef struct packed {
    logic                 w;
    logic [$clog2(N)-1:0] a;
  } addr_t;

  addr_t                                rd_ptr_w, rd_ptr_r;
  logic                                 rd_ptr_en;
  addr_t                                wr_ptr_w, wr_ptr_r;
  logic                                 wr_ptr_en;
  //
  logic                                 push_adv;
  logic                                 pop_adv;

  typedef logic [W-1:0]   w_t;

  typedef struct packed {
    w_t [N-1:0] m;
  } mem_t;

  mem_t                                 mem_r;

  logic                                 pop_en, push_en;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin :  fifo_cntrl_PROC

      //
      push_adv  = push & (~full_r);
      pop_adv   = pop & (~empty_r);
      

      // Read pointer update
      //
      casez ({flush, pop_adv})
        2'b1_?:  rd_ptr_w  = '0;
        2'b0_1:  rd_ptr_w  = rd_ptr_r + 'b1;
        default: rd_ptr_w  = rd_ptr_r;
      endcase

      // Pop enable
      //
      rd_ptr_en  = (flush | pop_adv);

      // Write pointer update
      //
      casez ({flush, push_adv})
        2'b1_?:  wr_ptr_w  = '0;
        2'b0_1:  wr_ptr_w  = wr_ptr_r + 'b1;
        default: wr_ptr_w  = wr_ptr_r;
      endcase

      // Push enable
      //
      wr_ptr_en  = (flush | push_adv);

      // Empty when pointers match
      //
      empty_w    = (rd_ptr_w == wr_ptr_w);

      // Full when pointers alias but do not match.
      //
      full_w     = (rd_ptr_w.w ^ wr_ptr_w.w) & (rd_ptr_w.a == wr_ptr_w.a);

      // Compute comb. output for current read pointer.
      //
      pop_data   = mem_r.m [rd_ptr_r.a];

    end // block: fifo_cntrl_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      empty_r <= 1'b1;
    else
      empty_r <= empty_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      full_r <= 1'b0;
    else
      full_r <= full_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) begin
      rd_ptr_r <= 'b0;
      wr_ptr_r <= 'b0;
    end else begin
      if (rd_ptr_en)
        rd_ptr_r <= rd_ptr_w;
      if (wr_ptr_en)
        wr_ptr_r <= wr_ptr_w;
    end

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (push_adv)
      mem_r.m [wr_ptr_r.a] <= push_data;

endmodule // fifo_basic
