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

`include "multi_ported_sram_pkg.svh"

module multi_ported_sram #(parameter int NUM_W = 1,
                         parameter int NUM_R = 1,
                         parameter int W = 32,
                         parameter int N = 8, 
                         parameter multi_ported_sram_pkg::type_t TYPE
                               = multi_ported_sram_pkg::LVT_FLOP) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Read Interface                                                          //
   //                                                                         //
   //======================================================================== //

   //
   , input        [NUM_R-1:0]                ren
   , input        [(NUM_R*$clog2(N))-1:0]    raddr
   //
   , output logic [(NUM_R*W)-1:0]            rdata

   //======================================================================== //
   //                                                                         //
   // Write Interface                                                         //
   //                                                                         //
   //======================================================================== //

   //
   , input        [NUM_W-1:0]                wen
   , input        [(NUM_W*$clog2(N))-1:0]    waddr
   , input        [(NUM_W*W)-1:0]            wdata

   //======================================================================== //
   //                                                                         //
   // Status/Control                                                          //
   //                                                                         //
   //======================================================================== //

   , input                                   init
   //
   , output logic                            busy_w
);
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  generate if (TYPE == multi_ported_sram_pkg::LVT_FLOP) begin

    multi_ported_sram_lvt_flop #(
       .NUM_W(NUM_W), .NUM_R(NUM_R), .W(W), .N(N)) u_mem (
      //
        .clk             (clk           )
      , .rst             (rst           )
      //
      , .ren             (ren           )
      , .raddr           (raddr         )
      , .rdata           (rdata         )
      //
      , .wen             (wen           )
      , .waddr           (waddr         )
      , .wdata           (wdata         )
      //
      , .init            (init          )
      , .busy_w          (busy_w        )
    );

  end endgenerate

  // ------------------------------------------------------------------------ //
  //
  generate if (TYPE == multi_ported_sram_pkg::XOR) begin

    multi_ported_sram_xor #(
       .NUM_W(NUM_W), .NUM_R(NUM_R), .W(W), .N(N)) u_mem (
      //
        .clk             (clk           )
      , .rst             (rst           )
      //
      , .ren             (ren           )
      , .raddr           (raddr         )
      , .rdata           (rdata         )
      //
      , .wen             (wen           )
      , .waddr           (waddr         )
      , .wdata           (wdata         )
      //
      , .init            (init          )
      , .busy_w          (busy_w        )
    );

  end endgenerate

endmodule
