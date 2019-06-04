//========================================================================== //
// Copyright (c) 2016, Stephen Henry
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

module one_or_two #(parameter int W = 32) (

  //======================================================================== //
  //                                                                         //
  // Misc.                                                                   //
  //                                                                         //
  //======================================================================== //

  //
    input                               clk
  , input                               rst

 //========================================================================== //
 //                                                                           //
 // Input                                                                     //
 //                                                                           //
 //========================================================================== //

 , input         pass
 , input [W-1:0] x

 //========================================================================== //
 //                                                                           //
 // Control                                                                   //
 //                                                                           //
 //========================================================================== //

 , input logic inv

 //========================================================================== //
 //                                                                           //
 // Output                                                                    //
 //                                                                           //
 //========================================================================== //

 , output logic has_vld_r
 , output logic has_set_0_r
 , output logic has_set_1_r
 , output logic has_set_more_than_1_r
);

  // ======================================================================== //
  //                                                                          //
  // Wires                                                                    //
  //                                                                          //
  // ======================================================================== //

  logic [W-1:0]               x_cond;
  logic                       is_power_of_2;
  logic                       has_set_0_w;
  logic                       has_set_1_w;
  logic                       has_set_more_than_1_w;
  logic                       has_vld_w;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : one_or_two_PROC

      //
      x_cond                 = x ^ {W{inv}};

      //
      has_set_0_w            = (~|x_cond);

      // Critical path is through is_power_of_2
      //
      is_power_of_2          = ~|(x_cond & (x_cond - 1));

      //
      has_set_1_w            = (~has_set_0_w) & is_power_of_2;

      //
      has_set_more_than_1_w  = (W > 1) & (~has_set_0_w) & (~has_set_1_w);

      //
      has_vld_w              = pass;

    end // block: one_or_two_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      has_vld_r <= '0;
    else
      has_vld_r <= has_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (has_vld_w) begin
      has_set_0_r           <= has_set_0_w;
      has_set_1_r           <= has_set_1_w;
      has_set_more_than_1_r <= has_set_more_than_1_w;
    end

endmodule // one_or_two
