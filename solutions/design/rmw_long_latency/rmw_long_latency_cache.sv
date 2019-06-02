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

`include "rmw_long_latency_pkg.vh"

module rmw_long_latency_cache (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

   //
     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Issue                                                                   //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   iss_vld_r
   , input rmw_long_latency_pkg::issue_t     iss_r
   //
   , output logic                            iss_rdy_w

   //======================================================================== //
   //                                                                         //
   // Completion                                                              //
   //                                                                         //
   //======================================================================== //

   //
   , output                                  cmpl_vld_r
   , output rmw_long_latency_pkg::word_t     cmpl_word_r

   //======================================================================== //
   //                                                                         //
   // Lookup                                                                  //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                             tbl_wr_r
   , output rmw_long_latency_pkg::id_t        tbl_wr_id_r
   , output rmw_long_latency_pkg::word_t      tbl_wr_word_r
   //
   , input  logic                             tbl_rd_word_vld_r
   , input  rmw_long_latency_pkg::word_t      tbl_rd_word_r
   , input  rmw_long_latency_pkg::tag_t       tbl_rd_ctag_r
   //
   , output logic                             tbl_rd_r
   , output rmw_long_latency_pkg::id_t        tbl_rd_id_r
   , output rmw_long_latency_pkg::tag_t       tbl_rd_itag_r

   //======================================================================== //
   //                                                                         //
   // Exe                                                                     //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            exe_iss_vld_r
   , output rmw_long_latency_pkg::word_t     exe_iss_imm_r
   , output rmw_long_latency_pkg::op_t       exe_iss_op_r
   , output rmw_long_latency_pkg::word_t     exe_iss_reg_r
   , output rmw_long_latency_pkg::momento_t  exe_iss_momento_r

   //
   , input logic                             exe_wrbk_w
   , input rmw_long_latency_pkg::word_t      exe_wrbk_word_w
   , input rmw_long_latency_pkg::momento_t   exe_wrbk_momento_w
);
  import rmw_long_latency_pkg::*;

  //
  table_t [N - 1:0]                     table_r;
  table_t [N - 1:0]                     table_w;
  logic [N - 1:0]                       table_en;
  //
  table_t                               alloc_table;
  table_t                               issue_table;
  //
  logic [N-1:0]                         tbl_rd_capture;
  logic [N-1:0]                         tbl_id_hit_d;
  logic                                 tbl_id_hit;
  tag_t                                 tbl_id_hit_tag;
  //
  ptr_t                                 alloc_ptr_r;
  ptr_t                                 alloc_ptr_w;
  logic                                 alloc_ptr_en;
  //
  ptr_t                                 issue_ptr_r;
  ptr_t                                 issue_ptr_w;
  logic                                 issue_ptr_en;
  //
  ptr_t                                 alloc_adv_d;
  ptr_t                                 issue_adv_d;
  //
  logic                                 alloc_adv;
  logic                                 issue_adv;
  logic                                 exe_wrbk_bypass;
  //
  logic                                 exe_wrbk_r;
  word_t                                exe_wrbk_word_r;
  momento_t                             exe_wrbk_momento_r;
  //
  //
  logic                                 exe_iss_vld_w;
  word_t                                exe_iss_imm_w;
  op_t                                  exe_iss_op_w;
  word_t                                exe_iss_reg_w;
  momento_t                             exe_iss_momento_w;
  //
  logic                                 tbl_wr_w;
  id_t                                  tbl_wr_id_w;
  word_t                                tbl_wr_word_w;
  //
  logic                                 tbl_rd_w;
  id_t                                  tbl_rd_id_w;
  tag_t                                 tbl_rd_itag_w;
  //
  logic                                 cmpl_vld_w;
  word_t                                cmpl_word_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : table_ptr_PROC

      //
      alloc_table   = table_mux_N(table_r, alloc_ptr_r);
      issue_table   = table_mux_N(table_r, issue_ptr_r);

    end // block: table_ptr_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : table_PROC

      //
      for (int i = 0; i < N; i++)
        tbl_id_hit_d [i]  = iss_vld_r & table_r [i].vld & (table_r [i].issue.id == iss_r.id);

      //
      tbl_id_hit          = (tbl_id_hit_d != '0);
      tbl_id_hit_tag      = enc_ptr(pri_ptr(ror_n(tbl_id_hit_d, alloc_ptr_r)));

      for (int i = 0; i < N; i++) begin

        //
        table_w [i]  = table_r [i];
        
        //
        unique0 case (1'b1)
          // Allocation
          alloc_ptr_r [i]: begin
            table_w [i].vld    = 'b1;
            casez ({op_requires_tbl_lkup(iss_r.op), tbl_id_hit})
              2'b10:   table_w [i].state  = ST_AWAIT_TAG;
              2'b11:   table_w [i].state  = ST_AWAIT_BYPASS;
              default: table_w [i].state  = ST_RDY;
            endcase // casez ({op_requires_tbl(iss_r.op)})
            table_w [i].issue  = iss_r;
            table_w [i].tag    = tbl_id_hit ? tbl_id_hit_tag : tag_t'(i);
          end
          issue_ptr_r [i]: begin
            table_w [i].vld  = 'b0;
          end
          default: begin
            table_w [i]  = table_r [i];
          end
        endcase // unique0 case (1'b1)

        //
        tbl_rd_capture [i]  = tbl_rd_word_vld_r & (table_r [i].tag == tbl_rd_ctag_r);

        //
        if (tbl_rd_capture [i]) begin
          table_w [i].state  = ST_RDY;
          table_w [i].word   = tbl_rd_word_r;
        end

        //
        table_w [i].killwrbk  = tbl_id_hit_d [i];

        //
        table_en [i]          = '0;
        table_en [i]         |= alloc_adv_d [i];
        table_en [i]         |= issue_adv_d [i];
        table_en [i]         |= tbl_rd_capture [i];
        table_en [i]         |= tbl_id_hit_d [i];

        end // for (int i     = 0; i < N; i++)

    end // block: table_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cntrl_PROC

      //
      iss_rdy_w        = 1'b1;
      exe_wrbk_bypass  = exe_wrbk_w & (issue_table.issue.id == exe_wrbk_momento_w.id);

      //
      alloc_adv        = (iss_vld_r & iss_rdy_w);

      //
      casez ({issue_table.vld, issue_table.state, exe_wrbk_bypass})
        {1'b1, ST_RDY, 1'b?}:          issue_adv  = 'b1;
        {1'b1, ST_AWAIT_BYPASS, 1'b1}: issue_adv  = 'b1;
        default:                       issue_adv  = 'b0;
      endcase // casez ({issue_table.vld, issue_table.state})
      
      //
      alloc_adv_d  = alloc_adv ? alloc_ptr_r : '0;
      issue_adv_d  = issue_adv ? issue_ptr_r : '0;

      //
      exe_iss_vld_w      = issue_adv;
      exe_iss_imm_w      = issue_table.issue.imm;
      exe_iss_op_w       = issue_table.issue.op;
      exe_iss_momento_w  = issue_table.issue.id;
      exe_iss_reg_w      = exe_wrbk_bypass ? exe_wrbk_word_w : issue_table.word;

    end // block: cntrl_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : tbl_PROC

      //
      tbl_wr_w       = exe_wrbk_r;
      tbl_wr_id_w    = exe_wrbk_momento_r.id;
      tbl_wr_word_w  = exe_wrbk_word_r;

      //
      tbl_rd_w       = alloc_adv & op_requires_tbl_lkup(iss_r.op) & (~tbl_id_hit);
      tbl_rd_id_w    = iss_r.id;
      tbl_rd_itag_w  = enc_ptr(alloc_ptr_r);

    end // block: tbl_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : ptr_PROC
      
      //
      alloc_ptr_w    = alloc_adv ? advance_ptr(alloc_ptr_r) : alloc_ptr_r;
      alloc_ptr_en   = alloc_adv;

      //
      issue_ptr_w    = issue_adv ? advance_ptr(issue_ptr_r) : issue_ptr_r;
      issue_ptr_en   = issue_adv;

    end // block: ptr_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cmpl_PROC

      //
      cmpl_vld_w      = exe_wrbk_r;
      cmpl_word_w     = exe_wrbk_word_r;

    end // block: cmpl_PROC
    
  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      exe_wrbk_r <= '0;
    else
      exe_wrbk_r <= exe_wrbk_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (exe_wrbk_w) begin
      exe_wrbk_word_r    <= exe_wrbk_word_w;
      exe_wrbk_momento_r <= exe_wrbk_momento_w;
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      alloc_ptr_r <= 'b1;
    else if (alloc_ptr_en)
      alloc_ptr_r <= alloc_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      issue_ptr_r <= 'b1;
    else if (issue_ptr_en)
      issue_ptr_r <= issue_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk) begin
    for (int i = 0; i < N; i++)
      if (rst)
        table_r [i].vld <= 'b0;
      else if (table_en [i])
        table_r [i] <= table_w [i];
  end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      exe_iss_vld_r <= 'b0;
    else
      exe_iss_vld_r <= exe_iss_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (exe_iss_vld_w) begin
      exe_iss_imm_r <= exe_iss_imm_w;
      exe_iss_op_r  <= exe_iss_op_w;
      exe_iss_reg_r <= exe_iss_reg_w;
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      tbl_wr_r <= 'b0;
    else
      tbl_wr_r <= tbl_wr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (tbl_wr_w) begin
      tbl_wr_id_r   <= tbl_wr_id_w;
      tbl_wr_word_r <= tbl_wr_word_w;
    end

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      tbl_rd_r <= 'b0;
    else
      tbl_rd_r <= tbl_rd_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (tbl_rd_w) begin
      tbl_rd_id_r   <= tbl_rd_id_w;
      tbl_rd_itag_r <= tbl_rd_itag_w;
    end
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cmpl_vld_r <= '0;
    else
      cmpl_vld_r <= cmpl_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (cmpl_vld_w)
      cmpl_word_r <= cmpl_word_w;
  
  // ======================================================================== //
  //                                                                          //
  // Instantiations                                                           //
  //                                                                          //
  // ======================================================================== //
  
endmodule
