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

module dblbuf #(parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst


   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   //
   , input                                        A__in_vld_r
   , input        [W-1:0]                         A__in_w

   , output logic                                 A__in_accept_r

   //
   , input logic                                  B__out_accept_r

   , output logic                                 B__out_vld_r
   , output logic [W-1:0]                         B__out_r

   //
   , input                                        stall_req
);

  //
  typedef logic [W-1:0]       buffer_t;

  //
  buffer_t [1:0]                             buffer_r;
  buffer_t [1:0]                             buffer_w;
  logic    [1:0]                             buffer_en;
  //
  logic [1:0]                                buffer_vld_r;
  logic [1:0]                                buffer_vld_w;
  //
  logic                                      A__in_accept_w;
  logic                                      A__adv;

  typedef struct packed {
    logic        in_accept;
    logic [1:0]  st;
  } state_t;

  localparam state_t IDLE   = 3'b0_00;
  localparam state_t SINGLE = 3'b0_01;
  localparam state_t DOUBLE  = 3'b1_10;

  state_t                                    state_r;
  state_t                                    state_w;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
 
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cntrl_PROC

      //
      buffer_w      = buffer_r;
      buffer_vld_w  = buffer_vld_r;
      buffer_en     = '0;

      //
      state_w       = state_r;

      //
      A__adv        = (A__in_vld_r & A__in_accept_r);

      case (state_r)
        IDLE: begin
          if (A__adv) begin
            buffer_w [0]  = A__in_w;
            buffer_vld_w  = 2'b01;
            buffer_en     = 2'b01;
            
            state_w       = SINGLE;
          end
        end
        SINGLE: begin
          casez ({A__adv, B__out_accept_r})
            2'b01: begin
              buffer_vld_w  = 2'b00;

              state_w       = IDLE;
            end
            2'b10: begin
              buffer_w [1]   = A__in_w;
              buffer_vld_w   = 2'b11;
              buffer_en [1]  = 1'b1;

              state_w        = DOUBLE;
            end
            2'b11: begin
              buffer_w [0]   = A__in_w;
              buffer_vld_w   = 2'b01;
              buffer_en [0]  = 'b1;

              state_w        = SINGLE;
            end
            default:;
          endcase
        end
        DOUBLE: begin
          if (B__out_accept_r) begin
            buffer_w [0]   = buffer_r [1];
            buffer_vld_w   = 2'b01;
            buffer_en [0]  = 'b1;

            state_w        = SINGLE;
          end
        end
        default:;
      endcase // case (state_r)

      //
      A__in_accept_w  = state_w.in_accept & (~stall_req);

      B__out_vld_r    = buffer_vld_r [0];
      B__out_r        = buffer_r [0];

    end // block: cntrl_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      state_r <= IDLE;
    else
      state_r <= state_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      A__in_accept_r <= '0;
    else
      A__in_accept_r <= A__in_accept_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      buffer_vld_r <= 'b0;
    else
      buffer_vld_r <= buffer_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < 2; i++)
      if (buffer_en [i])
        buffer_r [i] <= buffer_w [i];

endmodule // dblbuf
