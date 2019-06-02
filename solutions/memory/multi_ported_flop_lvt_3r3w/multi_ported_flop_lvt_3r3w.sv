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

module multi_ported_flop_lvt_3r3w (

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
   , input                                   ren0
   , input        [9:0]                      raddr0
   //
   , output logic [31:0]                     rdata0
   
   //
   , input                                   ren1
   , input     [9:0]                         raddr1
   //
   , output logic [31:0]                     rdata1
   
   //   
   , input                                   ren2
   , input     [9:0]                         raddr2
   //
   , output logic [31:0]                     rdata2

   //======================================================================== //
   //                                                                         //
   // Write Interface                                                         //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   wen0
   , input        [9:0]                      waddr0
   , input        [31:0]                     wdata0
   //
   , input                                   wen1
   , input        [9:0]                      waddr1
   , input        [31:0]                     wdata1
   //
   , input                                   wen2
   , input        [9:0]                      waddr2
   , input        [31:0]                     wdata2

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
  // Generic SRAM-/Flop-based multiported memory structure. RTL located
  // in: contrib/libv2/sv/memory/multi_ported_sram.sv
  //
  multi_ported_sram #(.NUM_W(3), .NUM_R(3), .W(32), .N(1024)) u_mem (
    //
      .clk               (clk                          )
    , .rst               (rst                          )
    //
    , .ren               ({ren2, ren1, ren0}           )
    , .raddr             ({raddr2, raddr1, raddr0}     )
    , .rdata             ({rdata2, rdata1, rdata0}     )
    //
    , .wen               ({wen2, wen1, wen0}           )
    , .waddr             ({waddr2, waddr1, waddr0}     )
    , .wdata             ({wdata2, wdata1, wdata0}     )
    //
    , .init              (init                         )
    , .busy_w            (busy_w                       )
  );

endmodule
