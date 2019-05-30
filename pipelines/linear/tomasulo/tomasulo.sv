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

`include "libtb2.vh"
`include "libv2_pkg.vh"

`include "tomasulo_pkg.vh"

module tomasulo (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // In                                                                      //
   //                                                                         //
   //======================================================================== //

   , input                                        in_pass
   , input tomasulo_pkg::reg_t                    in_inst_ra_1
   , input tomasulo_pkg::reg_t                    in_inst_ra_0
   , input tomasulo_pkg::reg_t                    in_inst_wa
   , input tomasulo_pkg::opcode_t                 in_inst_op
   , input tomasulo_pkg::imm_t                    in_imm
   //
   , output logic                                 in_accept

   //======================================================================== //
   //                                                                         //
   // Out                                                                     //
   //                                                                         //
   //======================================================================== //

   , output                                       out_vld_r
   , output tomasulo_pkg::reg_t                   out_wa_r
   , output tomasulo_pkg::word_t                  out_wdata_r
);
  import tomasulo_pkg::*;

  typedef struct packed {
    logic        o;
    logic [3:0]  p;
  } ptr_t;

  //
  fifo_inst_t                 fifo_in;
  //
  fifo_inst_t [15:0]          fifo_inst_r;
  //
  fifo_inst_t                 inst;
  logic                       inst_vld;
  logic                       inst_adv;
  //
  ptr_t                       wr_ptr_r;
  ptr_t                       wr_ptr_w;
  logic                       wr_ptr_en;
  //
  ptr_t                       rd_ptr_r;
  ptr_t                       rd_ptr_w;
  logic                       rd_ptr_en;
  //
  logic                       fifo_empty_w;
  logic                       fifo_empty_r;
  //
  logic                       fifo_full_w;
  logic                       fifo_full_r;
  //
  logic                       fifo_push;
  logic                       fifo_pop;
  //
  logic                       exe_arith_0_dis_vld_r;
  logic                       exe_arith_1_dis_vld_r;
  logic                       exe_logic_0_dis_vld_r;
  logic                       exe_logic_1_dis_vld_r;
  logic                       exe_mpy_dis_vld_r;
  logic [4:0]                 dis_vld_r;
  dispatch_t                  dis_r;
  //
  logic [4:0]                 iss_vld_r;
  logic [4:0]                 iss_busy_r;
  //
  issue_t                     exe_arith_0_iss;
  issue_t                     exe_arith_1_iss;
  issue_t                     exe_logic_0_iss;
  issue_t                     exe_logic_1_iss;
  issue_t                     exe_logic_1_mpy;
  //
  cdb_t                       exe_arith_0_cdb_r;
  cdb_t                       exe_arith_1_cdb_r;
  cdb_t                       exe_logic_0_cdb_r;
  cdb_t                       exe_logic_1_cdb_r;
  cdb_t                       exe_mpy_cdb_r;
  //
  cdb_t                       cdb_w;
  cdb_t                       cdb_r;
  //
  logic [4:0]                 rr_req;
  logic [4:0]                 rr_gnt;
  logic                       rr_ack;
  //
  sch_t                       sch_r;
  sch_t                       sch_w;
  
  // ======================================================================== //
  //                                                                          //
  // Combinational Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : front_end_PROC

      //
      fifo_in         = '0;
      fifo_in.ra [1]  = in_inst_ra_1;
      fifo_in.ra [0]  = in_inst_ra_0;
      fifo_in.wa      = in_inst_wa;
      fifo_in.op      = in_inst_op;
      fifo_in.imm     = in_imm;
      
      //
      in_accept       = (~fifo_full_r);
      
      //
      fifo_push       = (in_pass & in_accept);
      fifo_pop        = inst_adv;

      //
      wr_ptr_w        = fifo_push ? (wr_ptr_r + 'b1) : wr_ptr_r;
      wr_ptr_en       = fifo_push;

      //
      rd_ptr_w        = fifo_pop ? (rd_ptr_r + 'b1) : rd_ptr_r;
      rd_ptr_en       = fifo_pop;

      //
      fifo_empty_w    = (rd_ptr_w == wr_ptr_w);
      fifo_full_w     = (rd_ptr_w.o ^ wr_ptr_w.o) & (rd_ptr_w.p == wr_ptr_w.p);

      //
      inst            = fifo_inst_r [rd_ptr_r.p];
      inst_vld        = (~fifo_empty_r);

    end // block: front_end_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cdb_PROC

      //
      rr_ack      = (|rr_req);

      //
      casez ({|rr_gnt[1:0], |rr_gnt[3:2], rr_gnt[4]})
        3'b1??:  sch_w = (sch_r >> 1) | (1 << LATENCY_ARITH_N);
        3'b01?:  sch_w = (sch_r >> 1) | (1 << LATENCY_LOGIC_N);
        3'b001:  sch_w = (sch_r >> 1) | (1 << LATENCY_MPY_N);
        default: sch_w = (sch_r >> 1);
      endcase // casez ({|rr_req[1:0], |rr_req[3:2], rr_req[4]})

      //
      cdb_w       = '0;
      cdb_w      |= exe_arith_0_cdb_r;
      cdb_w      |= exe_arith_1_cdb_r;
      cdb_w      |= exe_logic_0_cdb_r;
      cdb_w      |= exe_logic_1_cdb_r;
      cdb_w      |= exe_mpy_cdb_r;

    end // block: cdb_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      sch_r <= '0;
    else
      sch_r <= sch_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cdb_r <= '0;
    else
      cdb_r <= cdb_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      wr_ptr_r <= '0;
    else if (wr_ptr_en)
      wr_ptr_r <= wr_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      rd_ptr_r <= '0;
    else if (rd_ptr_en)
      rd_ptr_r <= rd_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      fifo_empty_r <= 'b1;
    else
      fifo_empty_r <= fifo_empty_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      fifo_full_r <= 'b0;
    else
      fifo_full_r <= fifo_full_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (fifo_push)
      fifo_inst_r [wr_ptr_r.p] <= fifo_in;
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  tomasulo_dispatcher u_dispatcher (
    //
      .clk               (clk                     )
    , .rst               (rst                     )
    //
    , .cdb_r             (cdb_r                   )
    //
    , .out_vld_r         (out_vld_r               )
    , .out_wa_r          (out_wa_r                )
    , .out_wdata_r       (out_wdata_r             )
    //
    , .inst              (inst                    )
    , .inst_vld          (inst_vld                )
    , .inst_adv          (inst_adv                )
    //
    , .iss_vld_r         (iss_vld_r               )
    , .dis_r             (dis_r                   )
    , .dis_vld_r         (dis_vld_r               )
  );

  // ------------------------------------------------------------------------ //
  //
  rr #(.W(5)) u_rr (
    //
      .clk               (clk                  )
    , .rst               (rst                  )
    //
    , .req               (rr_req               )
    , .ack               (rr_ack               )
    //
    , .gnt               (rr_gnt               )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_rs #(.N(ARITH_RS_N), .LATENCY_N(LATENCY_ARITH_N)) u_tomasulo_rs_arith_0 (
    //
      .clk               (clk                  )
    , .rst               (rst                  )
    //
    , .sch_r             (sch_r                )
    //
    , .cdb_r             (cdb_r                )
    //
    , .cdb_gnt           (rr_gnt [0]           )
    , .cdb_req           (rr_req [0]           )
    //
    , .dis_vld_r         (dis_vld_r [0]        )
    , .dis_r             (dis_r                )
    //
    , .iss_vld_r         (iss_vld_r [0]        )
    , .iss_r             (exe_arith_0_iss      )
    , .iss_busy_r        (iss_busy_r [0]       )
  );
  
  // ------------------------------------------------------------------------ //
  //
  tomasulo_exe_arith #(.LATENCY_N(LATENCY_ARITH_N)) u_exe_arith_0 (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld           (iss_vld_r [0]      )
    , .iss               (exe_arith_0_iss    )
    , .iss_busy_r        (iss_busy_r [0]     )
    //
    , .cdb_r             (exe_arith_0_cdb_r  )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_rs #(.N(ARITH_RS_N), .LATENCY_N(LATENCY_ARITH_N)) u_tomasulo_rs_arith_1 (
    //
      .clk               (clk                  )
    , .rst               (rst                  )
    //
    , .sch_r             (sch_r                )
    //
    , .cdb_r             (cdb_r                )
    //
    , .cdb_gnt           (rr_gnt [1]           )
    , .cdb_req           (rr_req [1]           )
    //
    , .dis_vld_r         (dis_vld_r [1]        )
    , .dis_r             (dis_r                )
    //
    , .iss_vld_r         (iss_vld_r [1]        )
    , .iss_r             (exe_arith_1_iss      )
    , .iss_busy_r        (iss_busy_r [1]       )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_exe_arith #(.LATENCY_N(LATENCY_ARITH_N)) u_exe_arith_1 (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld           (iss_vld_r [1]      )
    , .iss               (exe_arith_1_iss    )
    , .iss_busy_r        (iss_busy_r [1]     )
    //
    , .cdb_r             (exe_arith_1_cdb_r  )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_rs #(.N(LOGIC_RS_N), .LATENCY_N(LATENCY_LOGIC_N)) u_tomasulo_rs_logic_0 (
    //
      .clk               (clk                  )
    , .rst               (rst                  )
    //
    , .sch_r             (sch_r                )
    //
    , .cdb_r             (cdb_r                )
    //
    , .cdb_gnt           (rr_gnt [2]           )
    , .cdb_req           (rr_req [2]           )
    //
    , .dis_vld_r         (dis_vld_r [2]        )
    , .dis_r             (dis_r                )
    //
    , .iss_vld_r         (iss_vld_r [2]        )
    , .iss_r             (exe_logic_0_iss      )
    , .iss_busy_r        (iss_busy_r [2]       )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_exe_logic #(.LATENCY_N(LATENCY_LOGIC_N)) u_exe_logic_0 (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld           (iss_vld_r [2]      )
    , .iss               (exe_logic_0_iss    )
    , .iss_busy_r        (iss_busy_r [2]     )
    //
    , .cdb_r             (exe_logic_0_cdb_r  )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_rs #(.N(LOGIC_RS_N), .LATENCY_N(LATENCY_LOGIC_N)) u_tomasulo_rs_logic_1 (
    //
      .clk               (clk                  )
    , .rst               (rst                  )
    //
    , .sch_r             (sch_r                )
    //
    , .cdb_r             (cdb_r                )
    //
    , .cdb_gnt           (rr_gnt [3]           )
    , .cdb_req           (rr_req [3]           )
    //
    , .dis_vld_r         (dis_vld_r [3]        )
    , .dis_r             (dis_r                )
    //
    , .iss_vld_r         (iss_vld_r [3]        )
    , .iss_r             (exe_logic_0_iss      )
    , .iss_busy_r        (iss_busy_r [3]       )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_exe_logic #(.LATENCY_N(LATENCY_LOGIC_N)) u_exe_logic_1 (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld           (iss_vld_r [3]      )
    , .iss               (exe_logic_0_iss    )
    , .iss_busy_r        (iss_busy_r [3]     )
    //
    , .cdb_r             (exe_logic_1_cdb_r  )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_rs #(.N(MPY_RS_N), .LATENCY_N(LATENCY_MPY_N)) u_tomasulo_rs_mpy (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .sch_r             (sch_r              )
    //
    , .cdb_r             (cdb_r              )
    //
    , .cdb_gnt           (rr_gnt [4]         )
    , .cdb_req           (rr_req [4]         )
    //
    , .dis_vld_r         (dis_vld_r [4]      )
    , .dis_r             (dis_r              )
    //
    , .iss_vld_r         (iss_vld_r [4]      )
    , .iss_r             (exe_logic_1_mpy    )
    , .iss_busy_r        (iss_busy_r [4]     )
  );

  // ------------------------------------------------------------------------ //
  //
  tomasulo_exe_mpy u_exe_mpy (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld           (iss_vld_r [4]      )
    , .iss               (exe_logic_1_mpy    )
    , .iss_busy_r        (iss_busy_r [4]     )
    //
    , .cdb_r             (exe_mpy_cdb_r      )
  );
  
endmodule // tomasulo
