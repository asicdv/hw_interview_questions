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

`include "dll_pkg.vh"
`include "spsram_pkg.vh"

module doubly_linked_list_cntrl
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
   , input   dll_pkg::op_t                   cmd_op
   , input   dll_pkg::id_t                   cmd_id
   //
   , output  dll_pkg::ptr_t                  cmd_push_ptr_r
   , output  dll_pkg::ptr_t                  cmd_pop_ptr_w
   //
   , input                                   clear
   //
   , output  logic                           full_r
   , output  logic                           empty_r
   , output  dll_pkg::empty_t                nempty_r
   //
   , output  logic                           busy_r
);

  import dll_pkg::*;

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
  //
  logic                                 ptr_table_n_en;
  logic                                 ptr_table_n_wen;
  ptr_t                                 ptr_table_n_addr;
  ptr_t                                 ptr_table_n_din;
  ptr_t                                 ptr_table_n_dout;
  //
  logic                                 ptr_table_p_en;
  logic                                 ptr_table_p_wen;
  ptr_t                                 ptr_table_p_addr;
  ptr_t                                 ptr_table_p_din;
  ptr_t                                 ptr_table_p_dout;

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : in_PROC

      //
      queue_t q       = queue_table_r.q [cmd_id];

      //
      ptr_table_n_en   = q.valid & cmd_pass;
      ptr_table_n_wen  = cmd_op [OP_PUSH_B];
      ptr_table_n_addr = '0;
      ptr_table_n_din  = '0;
      //
      ptr_table_p_en   = q.valid & cmd_pass;
      ptr_table_p_wen  = cmd_op [OP_PUSH_B];
      ptr_table_p_addr = '0;
      ptr_table_p_din  = '0;
      case (cmd_op)
        OP_POP_FRONT: begin
          ptr_table_p_addr = q.head;
          end
        OP_POP_BACK: begin
          ptr_table_n_addr = q.tail;
        end
        OP_PUSH_FRONT: begin
          //
          ptr_table_n_addr = q.head;
          ptr_table_n_din  = cmd_push_ptr_r;
          //
          ptr_table_p_addr = cmd_push_ptr_r;
          ptr_table_p_din  = q.head;
        end
        OP_PUSH_BACK: begin
          //
          ptr_table_n_addr = cmd_push_ptr_r;
          ptr_table_n_din  = q.tail;
          //
          ptr_table_p_addr = q.tail;
          ptr_table_p_din  = cmd_push_ptr_r;
        end
      endcase

      //
      cmd_en          = (cmd_pass | cmd_r.valid);

      //
      cmd_w           = '0;
      cmd_w.valid     = cmd_pass;
      cmd_w.op        = cmd_op;
      cmd_w.id        = cmd_id;
      cmd_w.ptr       = cmd_push_ptr_r;
      cmd_w.q         = q;

      //
      cmd_pop_ptr_w   = cmd_op [OP_BACK_B] ? q.tail : q.head;

      //
      ptr_valid_en    = (cmd_w.valid | clear);
      ptr_valid_w     = ptr_valid_r;
      casez ({clear, cmd_w.op})
        3'b1_??: begin
          ptr_valid_w  = '0;
        end
        3'b0_1?: begin
          ptr_valid_w [cmd_w.ptr] = '1;
        end
        default: begin
          ptr_t ptr = cmd_w.op [OP_BACK_B] ? cmd_w.q.tail : cmd_w.q.head;
          ptr_valid_w [ptr] = '0;
        end
      endcase

    end // block: in_PROC

  // ------------------------------------------------------------------------ //
  //
  function ptr_t ffs(ptr_d_t d);
    begin
      ptr_t r;
      for (int i = PTR_N - 1; i >= 0; i--)
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
      case (cmd_r.op[OP_PUSH_B])
        1'b1: begin
          upt.valid  = 1'b1;
          if (cmd_r.q.valid) begin
            upt.head = cmd_r.op [OP_BACK_B] ? cmd_r.q.head : cmd_r.ptr;
            upt.tail = cmd_r.op [OP_BACK_B] ? cmd_r.ptr : cmd_r.q.tail;
          end else begin
            upt.head = cmd_r.ptr;
            upt.tail = cmd_r.ptr;
          end
        end
        default: begin
          upt.valid  = (cmd_r.q.head != cmd_r.q.tail);
          if (upt.valid) begin
            upt.head = cmd_r.op [OP_BACK_B] ? cmd_r.q.head : ptr_table_p_dout;
            upt.tail = cmd_r.op [OP_BACK_B] ? ptr_table_n_dout : cmd_r.q.tail;
          end else begin
            upt.head = '0;
            upt.tail = '0;
          end
        end
      endcase
      queue_table_w  = upt;

    end // block: upt_PROC

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmd_push_ptr_r <= '0;
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
  spsram #(.W($bits(ptr_t)), .N(PTR_N)) u_ptr_table_n (
    //
      .clk                    (clk                 )
    //
    , .en                     (ptr_table_n_en      )
    , .wen                    (ptr_table_n_wen     )
    , .addr                   (ptr_table_n_addr    )
    , .din                    (ptr_table_n_din     )
    , .dout                   (ptr_table_n_dout    )
  );

  // ------------------------------------------------------------------------ //
  //
  spsram #(.W($bits(ptr_t)), .N(PTR_N)) u_ptr_table_p (
    //
      .clk                    (clk                 )
    //
    , .en                     (ptr_table_p_en      )
    , .wen                    (ptr_table_p_wen     )
    , .addr                   (ptr_table_p_addr    )
    , .din                    (ptr_table_p_din     )
    , .dout                   (ptr_table_p_dout    )
  );

endmodule // doubly_linked_list_cntrl
