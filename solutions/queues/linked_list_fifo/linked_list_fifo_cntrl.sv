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

`include "llfifo_pkg.vh"
`include "spsram_pkg.vh"

module linked_list_fifo_cntrl
(
   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   cmd_pass
   , input                                   cmd_push
   , input   llfifo_pkg::id_t                cmd_id
   //
   , output  llfifo_pkg::ptr_t               cmd_push_ptr_r
   , output  llfifo_pkg::ptr_t               cmd_pop_ptr_w
   //
   , input                                   clear
   //
   , output  logic                           full_r
   , output  logic                           empty_r
   , output  llfifo_pkg::empty_t             nempty_r
   //
   , output  logic                           busy_r
);

  import llfifo_pkg::*;

  //
  logic                                 empty_w;
  logic                                 full_w;
  //
  queue_table_t                         queue_table_r;
  queue_t                               queue_table_w;
  id_d_t                                queue_table_en;
  //
  ptr_d_t                               ptr_valid_r;
  ptr_d_t                               ptr_valid_w;
  logic                                 ptr_valid_en;
  //
  ptr_t                                 cmd_push_ptr_w;
  logic                                 cmd_push_ptr_en;
  //
  cmd_t                                 cmd_r;
  cmd_t                                 cmd_w;
  logic                                 cmd_en;

  `SPSRAM_SIGNALS(ptr_table_, $bits(ptr_t), $clog2(PTR_N));

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : in_PROC

      //
      queue_t q       = queue_table_r.q [cmd_id];

      //
      cmd_pop_ptr_w   = q.tail;

      //
      ptr_table_en    = cmd_pass;
      ptr_table_wen   = cmd_push;
      ptr_table_addr  = cmd_push ? q.head : q.tail;
      ptr_table_din   = cmd_push_ptr_r;

      //
      cmd_en          = (cmd_pass | cmd_r.valid);

      //
      cmd_w           = '0;
      cmd_w.valid     = cmd_pass;
      cmd_w.push      = cmd_push;
      cmd_w.id        = cmd_id;
      cmd_w.ptr       = cmd_push_ptr_r;
      cmd_w.q         = q;

      //
      ptr_valid_en    = (cmd_w.valid | clear);
      ptr_valid_w     = ptr_valid_r;
      casez ({clear, cmd_w.push})
        2'b1?: begin
          ptr_valid_w  = '0;
        end
        2'b01: begin
          ptr_valid_w [cmd_w.ptr]= '1;
        end
        default: begin
          ptr_valid_w [cmd_w.q.tail]= '0;
        end
      endcase

    end // block: in_PROC

  // ------------------------------------------------------------------------ //
  //
  function ptr_t ffs(ptr_d_t d);
    begin
      ptr_t r;
      for (int i = $bits(ptr_d_t); i > 0; i--)
        if (d[i])
          r = ptr_t'(i);
      return r;
    end
  endfunction

  // ------------------------------------------------------------------------ //
  //
  always_comb begin : update_PROC

    //
    cmd_push_ptr_w   = ffs(~ptr_valid_w);
    cmd_push_ptr_en  = cmd_pass;

    //
    busy_r           = cmd_r.valid;

    //
    empty_w          = (ptr_valid_w == '0);
    full_w           = (ptr_valid_w == '1);

    //
    for (int i = 0; i < ID_N; i++)
      nempty_r [i] = queue_table_r.q [i].valid;

  end // block: update_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : upt_PROC

      queue_t upt;

      //
      queue_table_en             = '0;
      queue_table_en [cmd_r.id]  = cmd_r.valid;
      upt                        = cmd_r.q;
      case (cmd_r.push)
        1'b1: begin
          upt.cnt    = cmd_r.q.cnt + 'b1;
          upt.valid  = 1'b1;
          upt.head   = cmd_r.ptr;
          upt.tail   = cmd_r.q.valid ? cmd_r.q.tail : cmd_r.ptr;
        end
        default: begin
          upt.cnt    = cmd_r.q.cnt - 'b1;
          upt.valid  = (upt.cnt != '0);
          upt.head   = upt.valid ? cmd_r.q.head : '0;
          upt.tail   = upt.valid ? ptr_table_dout : '0;
        end
      endcase
      queue_table_w  = upt;

    end // block: upt_PROC

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmd_push_ptr_r <= 'b1;
    else if (cmd_push_ptr_en)
      cmd_push_ptr_r <= cmd_push_ptr_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      ptr_valid_r <= '0;
    else if (ptr_valid_en)
      ptr_valid_r <= ptr_valid_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk) begin : empty_full_reg_PROC
      if (rst) begin
        empty_r <= 1'b1;
        full_r  <= 1'b0;
      end else begin
        empty_r <= empty_w;
        full_r  <= full_w;
      end
  end // block: empty_full_reg_PROC

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk) begin : queue_table_reg_PROC
    if (rst) begin
      queue_table_r <= '0;
    end else begin
      for (int i = 0; i < ID_N; i++)
        if (queue_table_en [i] || clear)
          queue_table_r.q [i] <= clear ? '0 : queue_table_w;
    end
  end // block: queue_table_reg_PROC

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmd_r <= '0;
    else if (cmd_en)
      cmd_r <= cmd_w;

  // ------------------------------------------------------------------------ //
  //
  spsram #(.W($bits(ptr_t)), .N(PTR_N)) u_ptr_table (
    //
      .clk                    (clk                )
    //
    , .en                     (ptr_table_en       )
    , .wen                    (ptr_table_wen      )
    , .addr                   (ptr_table_addr     )
    , .din                    (ptr_table_din      )
    , .dout                   (ptr_table_dout     )
  );


endmodule // linked_list_fifo_cntrl
