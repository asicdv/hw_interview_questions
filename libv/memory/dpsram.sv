//========================================================================== //
// Copyright (c) 2016-2017, Stephen Henry
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

module dpsram #(parameter int W = 32, parameter int N = 128)
(
    input                               clk0
  , input                               en0
  , input                               wen0
  , input         [$clog2(N)-1:0]       addr0
  , input         [W-1:0]               din0
  //
  , output logic  [W-1:0]               dout0
  //
  , input                               clk1
  , input                               en1
  , input                               wen1
  , input         [$clog2(N)-1:0]       addr1
  , input         [W-1:0]               din1
  //
  , output logic  [W-1:0]               dout1
);

  parameter logic P_IGNORE_COLLISIONS  = 1'b0;

  function logic address_collision; begin
    logic ret  = '0;
    if (en0 && en1) begin
      ret  = (addr0 == addr1) & (|{wen0, wen1});
    end
    return P_IGNORE_COLLISIONS ? 1'b0 : ret;
  end endfunction // address_collision
  
  `libtb2_assert_clk(!address_collision(), posedge clk0 or posedge clk1);

  typedef logic [W-1:0]       w_t;
  w_t                         mem_r [N-1:0];

  logic                       do_write0;
  logic                       do_read0;

  logic                       do_write1;
  logic                       do_read1;

  always_comb do_write0  = en0 & wen0;
  always_comb do_read0   = en0 & (~wen0);

  always_comb do_write1  = en1 & wen1;
  always_comb do_read1   = en1 & (~wen1);

  always_ff @(posedge clk0)
    if (do_write0)
      mem_r [addr0] <= din0;

  always_ff @(posedge clk1)
    if (do_write1)
      mem_r [addr1] <= din1;

  always_ff @(posedge clk0)
    if (do_read0)
      dout0 <= mem_r [addr0];

  always_ff @(posedge clk1)
    if (do_read1)
      dout1 <= mem_r [addr1];

endmodule // dpsram
