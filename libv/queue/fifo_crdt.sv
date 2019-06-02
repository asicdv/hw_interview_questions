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

`include "libtb2.vh"

module fifo_crdt #(
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

   //
   , input                                   push_vld
   , input [W-1:0]                           push_data
   //
   , input                                   push_crdt_deduct

   //======================================================================== //
   //                                                                         //
   // Pop Interface                                                           //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   pop
   //
   , output logic                            pop_vld_w
   , output logic [W-1:0]                    pop_data_w

   //======================================================================== //
   //                                                                         //
   // Control/Status Interface                                                //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   flush
   , input        [$clog2(N):0]              cancel
   //
   , output logic                            empty_r
   , output logic                            full_r
);
  // Validate parameterizations.
  `libtb2_static_assert(libtb2_pkg::is_power_of_2(N))

  // Validate input/output contract.
  `libtb2_assert({empty_r, full_r} != '1)
  `libtb2_assert(!(push_crdt_deduct && full_r))
  `libtb2_assert(!(pop && empty_r))

  //
  typedef logic [$clog2(N):0]                crdt_t;

  //
  typedef struct packed {
    logic                 ovr;
    logic [$clog2(N)-1:0] off;
  } ptr_t;

  //
  typedef logic [W-1:0]                      w_t;

  //
  typedef w_t                                mem_t [N-1:0];

  // ------------------------------------------------------------------------ //
  //
  logic                                      empty_w;
  logic                                      full_w;
  //
  crdt_t                                     crdt_r;
  crdt_t                                     crdt_w;
  logic                                      crdt_en;
  //
  ptr_t                                      wr_ptr_r;
  ptr_t                                      wr_ptr_w;
  logic                                      wr_ptr_en;
  //
  ptr_t                                      rd_ptr_r;
  ptr_t                                      rd_ptr_w;
  logic                                      rd_ptr_en;
  //
  mem_t                                      mem_r;
  logic                                      mem_en;
  //
  logic                                      crdt_cancel;
  
  // ======================================================================== //
  //                                                                          //
  // Wires                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : ptr_PROC

      //
      pop_vld_w    = (~empty_r);
      pop_data_w   = mem_r [rd_ptr_r.off];

      //
      wr_ptr_en    = push_vld;
      wr_ptr_w     = push_vld ? wr_ptr_r + 'b1 : wr_ptr_r;

      //
      rd_ptr_en    = pop;
      rd_ptr_w     = pop ? rd_ptr_r + 'b1 : rd_ptr_r;

      //
      mem_en       = push_vld;

      //
      crdt_cancel  = (cancel != '0);
      crdt_en      = (push_crdt_deduct | pop | crdt_cancel);
      crdt_w       =    crdt_r
                     - (push_crdt_deduct ? 'b1 : 'b0)
                     + (pop ? 'b1 : 'b0)
                     + (crdt_cancel ? cancel : 'b0)
                   ;

      //
      empty_w      = (rd_ptr_w == wr_ptr_w);
      full_w       = (crdt_w == '0);

    end // block: ptr_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      crdt_r <= crdt_t'(N);
    else if (crdt_en)
      crdt_r <= crdt_w;
  
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
    if (rst)
      wr_ptr_r <= '0;
    else if (wr_ptr_en)
      wr_ptr_r <= wr_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      rd_ptr_r <= '0;
    else if (rd_ptr_en)
      rd_ptr_r <= rd_ptr_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (mem_en)
      mem_r [wr_ptr_r.off] <= push_data;

endmodule // fifo_crdt_crdt


