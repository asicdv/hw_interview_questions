`ifndef LLFIFO_PKG_VH
`define LLFIFO_PKG_VH

//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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

package llfifo_pkg;

  localparam int ID_N  = 4;
  localparam int PTR_N = 255;
  localparam int PTR_W  = $clog2(PTR_N);

  typedef logic [$clog2(ID_N)-1:0] id_t;
  typedef logic [ID_N-1:0] id_d_t;
  typedef logic [PTR_W-1:0] ptr_t;
  typedef logic [PTR_N-1:1] ptr_d_t;
  typedef logic [ID_N-1:0] empty_t;
  typedef logic [31:0]     word_t;
  typedef logic [$clog2(PTR_N)-1:0] cnt_t;

  typedef struct packed {
    ptr_t [PTR_W-1:1] p;
  } ptr_table_t;

  typedef struct packed {
    logic        valid;
    ptr_t        head;
    ptr_t        tail;
    cnt_t        cnt;
  } queue_t;

  typedef struct packed {
    queue_t [PTR_N-1:0] q;
  } queue_table_t;

  typedef struct packed {
    logic        valid;
    logic        push;
    id_t         id;
    ptr_t        ptr;
    queue_t      q;
  } cmd_t;

endpackage // llfifo_pkg

`endif
