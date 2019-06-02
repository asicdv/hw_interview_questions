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

module sram_1rNw #(parameter int NUM_R = 1,
                   parameter int W = 32,
                   parameter int N = 8,
                   parameter bit ASYNC_DOUT = 'b0) (

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
   , input                                   wen
   , input        [$clog2(N)-1:0]            waddr
   , input        [W-1:0]                    wdata
);

  localparam int ADDR_W  = $clog2(N);
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  generate for (genvar g = 0; g < NUM_R; g++) begin

    //
    dpsram #(.W(W), .N(N)) u_dpsram_bnk (
      //
        .clk0                 (clk                )
      //
      , .en0                  (wen                )
      , .wen0                 (1'b1               )
      , .addr0                (waddr              )
      , .din0                 (wdata              )
      //
      , .dout0                ()
      //
      , .clk1                 (clk                )
      //
      , .en1                  (ren [g]            )
      , .wen1                 ('b0                )
      , .addr1                (raddr [(g*ADDR_W)+:ADDR_W]   )
      , .din1                 ('b0                )
      //
      , .dout1                (rdata [(g*W)+:W]   )
    );

  end endgenerate
  
endmodule
