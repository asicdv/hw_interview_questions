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

package dll_pkg;

  localparam int ID_N  = 4;
  localparam int PTR_N = 256;
  localparam int PTR_DIV2_N = PTR_N / 2;
  localparam int PTR_W  = $clog2(PTR_N);
  localparam int PTR_DIV2_W = $clog2(PTR_DIV2_N);

  typedef logic [$clog2(ID_N)-1:0] id_t;
  typedef logic [ID_N-1:0] id_d_t;
  typedef logic [PTR_W-1:0] ptr_t;
  typedef logic [PTR_N-1:0] ptr_d_t;
  typedef logic [PTR_DIV2_W-1:0] ptr_div2_t;
  typedef logic [PTR_DIV2_N-1:0] ptr_div2_d_t;
  typedef logic [ID_N-1:0] empty_t;
  typedef logic [31:0]     word_t;

  typedef enum logic [1:0] {  OP_POP_FRONT  = 2'b00,
                              OP_POP_BACK   = 2'b01,
                              OP_PUSH_FRONT   = 2'b10,
                              OP_PUSH_BACK    = 2'b11
                            } op_t;
  localparam int OP_PUSH_B  = 1;
  localparam int OP_BACK_B  = 0;

  typedef struct packed {
    logic        valid;
    ptr_t        head;
    ptr_t        tail;
  } queue_t;

  typedef struct packed {
    queue_t [ID_N-1:0] q;
  } queue_table_t;

  typedef struct packed {
    logic        valid;
    op_t         op;
    id_t         id;
    ptr_t        ptr;
    queue_t      q;
  } cmd_t;

endpackage // dll_pkg

`endif
