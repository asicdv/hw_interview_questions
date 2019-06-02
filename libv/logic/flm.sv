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

module flm #(parameter int N = 16) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   alloc_vld
   , input logic [$clog2(N)-1:0]             alloc_id
   //
   , input                                   free_vld
   , input logic [$clog2(N)-1:0]             free_id
   //
   , input                                   clear

   //======================================================================== //
   //                                                                         //
   // State                                                                   //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            idle_r
   , output logic                            busy_r
   //
   , output logic [N-1:0]                    state_w
   , output logic [N-1:0]                    state_r
);
  // Validate parameterizations
  //
  `libtb2_static_assert(N > 0);

  //
  typedef logic [N-1:0]                 n_t;
  typedef logic [$clog2(N)-1:0]         id_t;
  typedef logic [$clog2(N):0]           id1_t;

  //
  logic                                 state_en;
  //
  logic                                 idle_w;
  logic                                 busy_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  // Validate no attempt to allocate when busy.
  //
//  `libtb2_assert(alloc_vld & (~busy_r));

  // Validate no attempt to free with idle.
  //
//  `libtb2_assert(free_vld & (~idle_r));
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : state_PROC

      //
      state_w = state_r;
      if (alloc_vld)
        state_w [alloc_id] = 'b1;
      if (free_vld)
        state_w [free_id] = 'b0;
      if (clear)
        state_w = '0;

    end // block: state_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : rob_PROC

      //
      state_en = (alloc_vld | free_vld | clear);

      //
      idle_w = (state_w == '0);
      busy_w = (state_w == '1);
                
    end // block: rob_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      state_r <= '0;
    else if (state_en)
      state_r <= state_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      idle_r <= 'b1;
    else
      idle_r <= idle_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      busy_r <= 'b0;
    else
      busy_r <= busy_w;

endmodule
