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

module replay #(parameter int N = 10, parameter int W = 32) (

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
   // Cntrl                                                                   //
   //                                                                         //
   //======================================================================== //

   , input  logic                                 replay_s4_req
   , input  logic                                 replay_s8_req
   , input  logic   [N-1:0]                       stall_req
);

  typedef struct packed {
    logic        o;
    logic [2:0]  p;
  } ptr_t;

  typedef struct packed {
    logic [W-1:0] w;
    ptr_t         ptr;
  } ucode_t;

  //
  logic [N-1:1]                         vld_r;
  logic [N-1:1]                         vld_w;

  //
  ucode_t [N-1:1]                       ucode_r;
  ucode_t [N-1:1]                       ucode_w;

  //
  logic [N-1:0]                         adv;
  logic [N-1:1]                         en;
  logic [N-1:0]                         stall;
  logic [N-1:0]                         kill;

  //
  logic                                 commit_s8;
  logic                                 replay_s4_w;
  logic                                 replay_s8_w;
  logic                                 replay_s4_r;
  logic                                 replay_s8_r;
  logic                                 replay_s4;
  logic                                 replay_s8;

  //
  ptr_t                                 rd_ptr_arch_r;
  ptr_t                                 rd_ptr_arch_w;
  logic                                 rd_ptr_arch_en;
  //
  ptr_t                                 rd_ptr_spec_r;
  ptr_t                                 rd_ptr_spec_w;
  logic                                 rd_ptr_spec_en;
  //
  ptr_t                                 wr_ptr_r;
  ptr_t                                 wr_ptr_w;
  logic                                 wr_ptr_en;
  //
  logic                                 empty_w;
  logic                                 empty_r;
  logic                                 full_w;
  logic                                 full_r;
  //
  logic [7:0][W-1:0]                    fifo_r;
  logic                                 fifo_en;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : front_end_PROC

      //
      rd_ptr_arch_en = commit_s8;
      rd_ptr_arch_w  = commit_s8 ? (rd_ptr_arch_r + 'b1) : rd_ptr_arch_r;

      //
      rd_ptr_spec_en = (replay_s4 | replay_s8 | adv [0]);

      //
      casez ({replay_s8, replay_s4, adv [0]})
        3'b1??:  rd_ptr_spec_w  = ucode_r [8].ptr;
        3'b01?:  rd_ptr_spec_w  = ucode_r [4].ptr;
        3'b001:  rd_ptr_spec_w  = rd_ptr_spec_r + 'b1;
        default: rd_ptr_spec_w  = rd_ptr_spec_r;
      endcase // casez ({replay_s8_r, adv [0]})
      
      //
      fifo_en   = in_vld & in_accept;

      //
      wr_ptr_en = fifo_en;
      wr_ptr_w  = (fifo_en) ? wr_ptr_r + 'b1 : wr_ptr_r;

      //
      empty_w  = (rd_ptr_spec_w == wr_ptr_w);
      full_w   = (rd_ptr_arch_w.o  ^ wr_ptr_w.o) &
                 (rd_ptr_arch_w.p == wr_ptr_w.p);

    end // block: front_end_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : pipe_PROC

      // Replays are injected synthetically by the TB. For correct
      // operation the restart address must be valid, therefore the
      // replay request is held until the associated stage becomes
      // valid, when it is then applied.
      //
      casez ({replay_s4_req, replay_s4_r, vld_r [4]})
        3'b1_0_?: replay_s4_w  = 1'b1;
        3'b?_1_1: replay_s4_w  = 1'b0;
        default:  replay_s4_w  = replay_s4_r;
      endcase // casez ({replay_s4_req, replay_s4_r, vld_r [4]})

      //
      replay_s4  = replay_s4_r & vld_r [4];

      //
      casez ({replay_s8_req, replay_s8_r, vld_r [8]})
        3'b1_0_?: replay_s8_w  = 1'b1;
        3'b?_1_1: replay_s8_w  = 1'b0;
        default:  replay_s8_w  = replay_s8_r;
      endcase // casez ({replay_s8_req, replay_s8_r, vld_r [8]})

      //
      replay_s8  = replay_s8_r & vld_r [8];

      // The final (output) stage cannot be stalled (for DV simplicity).
      //
      stall [N - 1]  = 1'b0;

      // Kill stages on upstream replay.
      //
      kill           = '0;
      for (int i = 0; i < N; i++) begin
        kill [i]  |= (i <= 8) ? replay_s8 : '0;
        kill [i]  |= (i <= 4) ? replay_s4 : '0;
      end

      //
      for (int i = N - 2; i >= 0; i--)
        stall [i]  = ((i == 0) ? 1'b1 : vld_r [i]) & (stall_req [i] | stall [i + 1]);
      
      //
      in_accept    = (~full_r);

      //
      for (int i = 0; i < N; i++)
        adv [i]  = ((i == 0) ? (~empty_r) : vld_r [i]) & (~stall [i]) & (~kill [i]);
                  
      //
      for (int i = 1; i < N; i++)
        en [i]    = adv [i - 1];
      
      //
      for (int i = 1; i < N; i++) begin
        
        casez ({kill [i], stall [i]})
          2'b1?:   vld_w [i]  = 'b0;
          2'b00:   vld_w [i]  = adv [i - 1];
          default: vld_w [i]  = vld_r [i];
        endcase // casez ({kill [i], stall [i]})

        end // for (int i = 1; i < N; i++)
                    
      //
      for (int i = 1; i < N; i++) begin
        if (i == 1)
          ucode_w [i]  = '{w:fifo_r [rd_ptr_spec_r.p], ptr:rd_ptr_spec_r};
        else
          ucode_w [i]  = ucode_r [i - 1];
      end

      // Commit point at stage 8. 
      //
      commit_s8  = (~replay_s8_r) & (adv [8]);
                 
      //
      out_r      = ucode_r [N - 1].w;
      out_vld_r  = vld_r [N - 1];

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
    else
      vld_r <= vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 1; i < N; i++)
      if (en [i])
        ucode_r [i] <= ucode_w [i];

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      replay_s4_r <= '0;
    else
      replay_s4_r <= replay_s4_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      replay_s8_r <= '0;
    else
      replay_s8_r <= replay_s8_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) 
      rd_ptr_arch_r <= '0;
    else if (rd_ptr_arch_en)
      rd_ptr_arch_r <= rd_ptr_arch_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) 
      rd_ptr_spec_r <= '0;
    else if (rd_ptr_spec_en)
      rd_ptr_spec_r <= rd_ptr_spec_w;
  
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
    if (fifo_en)
      fifo_r [wr_ptr_r.p] <= in;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      full_r <= 'b0;
    else
      full_r <= full_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      empty_r <= 'b1;
    else
      empty_r <= empty_w;
  
endmodule // replay
