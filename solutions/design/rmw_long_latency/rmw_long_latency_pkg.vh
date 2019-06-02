`ifndef __RMW_LONG_LATENCY_PKG_VH__
`define __RMW_LONG_LATENCY_PKG_VH__

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

package rmw_long_latency_pkg;

  //
  typedef logic [15:0] id_t;
  typedef logic [31:0] word_t;

  //
  typedef enum logic [1:0] {  OP_NOP   = 2'b00,
                              OP_ADDI  = 2'b01,
                              OP_SUBI  = 2'b10,
                              OP_MOVI  = 2'b11
                            } op_t;

  //
  function logic op_requires_tbl_lkup(op_t op); begin
    op_requires_tbl_lkup  = (op != OP_MOVI);
  end endfunction

  //
  typedef struct packed {
    id_t id;
    word_t imm;
    op_t op;
  } issue_t;

  // The number of commands that may be enqueued. Typically around the
  // lookup latency.
  //
  localparam int N  = 64;

  typedef logic [N - 1:0] n_t;


  // The maximum number of concurrent in flight commands to TBL.
  //
/* -----\/----- EXCLUDED -----\/-----
  localparam int IN_FLIGHT_N  = 16;
  localparam int TAG_W  = $clog2(IN_FLIGHT_N);
  typedef logic [TAG_W-1:0] tag_t;

  function tag_t clz_tag(logic [IN_FLIGHT_N - 1:0] n); begin
    clz_tag = '0;
    for (int i = IN_FLIGHT_N - 1; i >= 0; i--)
      if (!n [i])
        clz_tag  = tag_t'(i);
  end endfunction
 -----/\----- EXCLUDED -----/\----- */
  
  //
  typedef logic [N-1:0] ptr_t;
  typedef logic [$clog2(N)-1:0] tag_t;
  
  function ptr_t advance_ptr(ptr_t p); begin
    if (p [$left(ptr_t)])
      advance_ptr  = 'b1;
    else
      advance_ptr  = (p << 1);
  end endfunction

  function tag_t enc_ptr(ptr_t p); begin
    enc_ptr  = '0;
    for (int i = $bits(ptr_t) - 1; i >= 0; i--)
      if (p [i])
        enc_ptr  = tag_t'(i);
  end endfunction
    

  //
  typedef enum logic [1:0] {  ST_RDY           = 2'b00,
                              ST_AWAIT_BYPASS  = 2'b01,
                              ST_AWAIT_TAG     = 2'b10,
                              ST_COMPLETE      = 2'b11
                              } state_t;

  //
  typedef struct packed {
    logic        vld;
    state_t      state;
    issue_t      issue;
    tag_t        tag;
    word_t       word;
    logic        killwrbk;
  } table_t;

  //
  typedef struct packed {
    id_t        id;
  } momento_t;
  

  function table_t table_mux_N(table_t [N-1:0] t, logic [N-1:0] sel); begin
    table_mux_N  = '0;
    for (int i = 0; i < N; i++)
      table_mux_N |= {$bits(table_t){sel[i]}} & t[i];
  end endfunction

  function n_t ror_n(n_t h, ptr_t p); begin
    n_t [1:0] a  = {h, h} >> enc_ptr(p);
    ror_n        = a[0];
  end endfunction

  function ptr_t pri_ptr(ptr_t a); begin
    pri_ptr  = '0;
    for (int i = $left(ptr_t); i >= $right(ptr_t); i--)
      if (a [i])
        pri_ptr  = ('b1 << i);
  end endfunction

endpackage // rmw_long_latency_pkg

`endif
