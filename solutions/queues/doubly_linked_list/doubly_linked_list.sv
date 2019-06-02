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

module doubly_linked_list
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
   , input   dll_pkg::word_t                 cmd_push_data
   //
   , output  dll_pkg::word_t                 cmd_pop_data
   , output  logic                           cmd_pop_data_vld_r
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

  `SPSRAM_SIGNALS(data_table_, $bits(word_t), $clog2(PTR_N));

  //
  ptr_t                       cmd_push_ptr_r;
  ptr_t                       cmd_pop_ptr_w;
  //
  logic                       cmd_pop_data_vld_w;

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : data_table_PROC

      data_table_en       = cmd_pass;
      data_table_wen      = (cmd_op == OP_PUSH_FRONT) || (cmd_op == OP_PUSH_BACK);

      // cmd_pop_ptr_w is some combinatorial function of cmd_id. This is a
      // fairly long path and may result in setup time violations for large data
      // tables. If this is the case, this may be trivally pipelined at the cost
      // of one additional cycle of latency.
      //
      data_table_addr     = cmd_op [OP_PUSH_B] ? cmd_push_ptr_r : cmd_pop_ptr_w;
      data_table_din      = cmd_push_data;
      cmd_pop_data        = data_table_dout;
      cmd_pop_data_vld_w  = data_table_en & (~data_table_wen);

    end // block: data_table_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmd_pop_data_vld_r <= '0;
    else
      cmd_pop_data_vld_r <= cmd_pop_data_vld_w;

  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  doubly_linked_list_cntrl u_cntrl (
    //
      .clk                    (clk                )
    , .rst                    (rst                )
    //
    , .cmd_pass               (cmd_pass           )
    , .cmd_op                 (cmd_op             )
    , .cmd_id                 (cmd_id             )
    //
    , .cmd_push_ptr_r         (cmd_push_ptr_r     )
    , .cmd_pop_ptr_w          (cmd_pop_ptr_w      )
    //
    , .clear                  (clear              )
    //
    , .full_r                 (full_r             )
    , .empty_r                (empty_r            )
    , .nempty_r               (nempty_r           )
    //
    , .busy_r                 (busy_r             )
  );

  // ------------------------------------------------------------------------ //
  //
  spsram #(.W($bits(word_t)), .N(PTR_N)) u_data_table (
    //
      .clk                    (clk                )
    //
    , .en                     (data_table_en      )
    , .wen                    (data_table_wen     )
    , .addr                   (data_table_addr    )
    , .din                    (data_table_din     )
    , .dout                   (data_table_dout    )
  );

endmodule
