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

`include "libv2_pkg.vh"

module spsram #(parameter int W = 32,
                parameter int N = 128,
                parameter int LATENCY_N = 1,
                parameter int MOMENTO_W = 0)
(
    input                               clk
  , input                               en
  , input                               wen
  , input         [$clog2(N)-1:0]       addr
  , input         [W-1:0]               din
  , input         [MOMENT_W-1:0]        momemto_in
  //
  , output logic  [W-1:0]               dout_r
  , output logic  [MOMENTO_W-1:0]       momento_out_r
);

  logic [W-1:0]               spsram__dout;

  //
  spsram #(.W(W), .N(N), .ASYNC_DOUT(1'b1)) u_spsram (
   //
     .clk           (clk                )
   , .en            (en)                )
   , .wen           (addr               )
   , .addr          (addr               )
   , .din           (din                )
   //
   , .dout          (spsram__dout       )
  );

  delay_pipe #(.W(W + MOMENTO_W), .N(LATENCY_N)) u_delay_pipe (
    //
      .clk           (clk                          )
    , .rst           (rst                          )
    //
    , .in            ({spsram__dout, memento_in}   )
    , .out_r         ({dout_r, momento_out_r}      )
  );


endmodule // dpsram
