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

module mlt5c #(parameter int W = 32) (

  //======================================================================== //
  //                                                                         //
  // Misc.                                                                   //
  //                                                                         //
  //======================================================================== //

  //
    input                               clk
  , input                               rst
  
  //======================================================================== //
  //                                                                         //
  // Misc.                                                                   //
  //                                                                         //
  //======================================================================== //

  //
  , input logic [W-1:0]                 a
  , input logic [W-1:0]                 b
  , input logic                         pass
  //
  , output logic [2*W-1:0]              y
  , output logic                        y_vld_r
  //
  , output logic                        busy_r
);

  //
  typedef logic [(W/2)-1:0]   h_t;
  typedef logic [W-1:0]       w_t;
  typedef logic [(W*2)-1:0]   w2_t;

  typedef struct packed {
    w2_t [2:0] w;
  } csa_t;

  typedef struct packed {
    h_t [1:0] v;
  } v_t;

  //
  typedef enum logic [3:0] {  S_C0      = 4'b0_00_0,
                              S_C1      = 4'b0_01_1,
                              S_C2      = 4'b0_10_1,
                              S_C3      = 4'b0_11_1,
                              S_RESULT  = 4'b1_00_1
                            } state_t;

  //
  localparam S_BUSY_B  = 0;
  localparam S_VLD_B  = 3;

  //
  state_t           state_r;
  state_t           state_w;
  logic             state_en;
  //
  w2_t              acc_0_r, acc_0_w;
  w2_t              acc_1_r, acc_1_w;
  logic             acc_en;
  logic             acc_clr;
  //
  csa_t             csa_x;
  w2_t              csa_y1, csa_y2;
  //
  h_t               mult_a;
  h_t               mult_b;
  w_t               mult_y;
  //
  v_t               v_a;
  v_t               v_b;
  //
  logic             y_vld_w;
  
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
      state_en  = (pass | state_r [S_BUSY_B]);

      //
      casez (state_r)
        S_C0:     state_w  = pass ? S_C1 : S_C0;
        S_C1:     state_w  = S_C2;
        S_C2:     state_w  = S_C3;
        S_C3:     state_w  = S_RESULT;
        S_RESULT: state_w  = S_C0;
        default:  state_w  = S_C0;
      endcase // casez (state_r)

    end // block: cntrl_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : mult_PROC

      //
      v_a       = a;
      v_b       = b;

      //
      case (state_r)
        default: begin
          //
          mult_a  = v_a.v [0];
          mult_b  = v_b.v [0];
        end
        S_C1: begin
          //
          mult_a  = v_a.v [1];
          mult_b  = v_b.v [0];
        end
        S_C2: begin
          //
          mult_a  = v_a.v [0];
          mult_b  = v_b.v [1];
        end
        S_C3: begin
          //
          mult_a  = v_a.v [1];
          mult_b  = v_b.v [1];
        end
      endcase

    end // block: mult_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : csa_PROC

      //
      csa_x        = '0;

      //
      case (state_r)
        S_C0:    csa_x.w [0]  = w2_t'(mult_y);
        S_C1:    csa_x.w [0]  = w2_t'(mult_y) << (W / 2);
        S_C2:    csa_x.w [0]  = w2_t'(mult_y) << (W / 2);
        S_C3:    csa_x.w [0]  = w2_t'(mult_y) << W;
        default: csa_x.w [0]  = '0;
      endcase // case (state_r)

      //
      case (state_r)
        S_C0: begin
          csa_x.w [1]  = '0;
          csa_x.w [2]  = '0;
        end
        default: begin
          csa_x.w [1]  = acc_0_r;
          csa_x.w [2]  = acc_1_r;
        end
      endcase // case (state_r)

      //
      case (state_r)
        S_RESULT: begin
          acc_0_w  = acc_0_r + acc_1_r;
        end
        default: begin
          acc_0_w  = csa_y1;
          acc_1_w  = csa_y2;
        end
      endcase // case (state_t)

      //
      acc_en  = (pass | state_r [S_BUSY_B]);

    end // block: csa_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : out_PROC

      //
      y  = acc_0_r;

      // RETAIN_VALID
      //
      // Retain the y_vld_r flag upon completion of a multiplcation
      // operation to be cleared upon arrival of new command, otherwise
      // flag is asserted for one cycle.

      //
      case (state_r)
        S_RESULT: y_vld_w  = 1'b1;
`ifdef RETAIN_VALID
        S_C0:     y_vld_w  = pass ? 1'b0 : y_vld_r;
`else 
        S_C0:     y_vld_w  = 1'b0;
`endif
        default:  y_vld_w  = y_vld_r;
      endcase // case (state_r)
      
      //
      busy_r  = state_r [S_BUSY_B];

    end // block: out_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) begin
      acc_0_r <= '0;
      acc_1_r <= '0;
    end else if (acc_en) begin
      acc_0_r <= acc_0_w;
      acc_1_r <= acc_1_w;
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      state_r <= S_C0;
    else if (state_en)
      state_r <= state_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      y_vld_r <= '0;
    else
      y_vld_r <= y_vld_w;
  
  // ===========================================<============================= //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

 // ------------------------------------------------------------------------ //
  //
  multiplier_booth_r4 #(.W(W/2)) u_mult (.a(mult_a), .b(mult_b), .y(mult_y));

  // ------------------------------------------------------------------------ //
  //
  csa #(.N(3), .W(2 * W)) u_csa (.x(csa_x), .y1(csa_y1), .y2(csa_y2));
  
endmodule // mlt5c
