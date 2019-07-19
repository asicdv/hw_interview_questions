// ==================================================================== //
// Copyright (c) 2019, Stephen Henry
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

`ifndef __LIBV_MACROS_PKG_VH__
`define __LIBV_MACROS_PKG_VH__

`define REG(__type, __name)\
  __type __name``_r;\
  __type __name``_w;\
  always_ff @(posedge clk) \
    __name``_r <= __name``_w

`define REG_EN(__type, __name)\
  __type __name``_r;\
  __type __name``_w;\
  logic  __name``_en;\
  always_ff @(posedge clk) \
    if (__name``_en)\
      __name``_r <= __name``_w

`define REG_EN_RST(__type, __name, __reset = 'b0)\
  __type __name``_r;\
  __type __name``_w;\
  logic  __name``_en;\
  always_ff @(posedge clk) \
    if (rst)\
     __name``_r <= __type'(__reset);\
    else\
      if (__name``_en)\
        __name``_r <= __name``_w

`endif
