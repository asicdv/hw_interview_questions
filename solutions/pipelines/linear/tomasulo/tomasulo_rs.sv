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

module tomasulo_rs #(parameter int N = 4, parameter int LATENCY_N = 2) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // Scheduler                                                               //
   //                                                                         //
   //======================================================================== //

   //
   , input tomasulo_pkg::sch_t                    sch_r

   //======================================================================== //
   //                                                                         //
   // CDB                                                                     //
   //                                                                         //
   //======================================================================== //

   , input tomasulo_pkg::cdb_t                    cdb_r

   //
   , input  logic                                 cdb_gnt
   //
   , output logic                                 cdb_req

   //======================================================================== //
   //                                                                         //
   // Dispatch                                                                //
   //                                                                         //
   //======================================================================== //

   , input                                        dis_vld_r
   , input tomasulo_pkg::dispatch_t               dis_r

   //======================================================================== //
   //                                                                         //
   // Issue                                                                   //
   //                                                                         //
   //======================================================================== //

   //
   , input                                        iss_busy_r
   //
   , output logic                                 iss_vld_r
   , output tomasulo_pkg::issue_t                 iss_r
);
  import tomasulo_pkg::*;

  //
  typedef logic [N - 1:0]               n_d_t;
  typedef logic [$clog2(N) - 1:0]       n_t;

  //
  typedef struct packed {
    opcode_t       opcode;
    oprand_t [1:0] oprand;
    tag_t          tag;
    reg_t          wa;
    imm_t          imm;
    robid_t        robid;
  } rs_entry_t;

  //
  rs_entry_t [N - 1:0]             rs_entry_r;
  rs_entry_t [N - 1:0]             rs_entry_upt;
  rs_entry_t [N - 1:0]             rs_entry_w;
  logic      [N - 1:0]             rs_entry_en;
  logic      [N - 1:0][1:0]        rs_latch_cdb;
  //
  n_d_t                            rs_vld_r;
  n_d_t                            rs_vld_upt;
  n_d_t                            rs_vld_w;
  logic                            rs_vld_en;
  //
  n_d_t                            rs_vld_set;
  n_d_t                            rs_vld_clr;
  //
  n_d_t                            rs_rdy_r;
  n_d_t                            rs_rdy_upt;
  n_d_t                            rs_rdy_w;
  logic                            rs_rdy_en;
  //
  n_d_t                            rs_rdy_set;
  n_d_t                            rs_rdy_clr;
  //
  //
  n_d_t                            rdy_sel;
  n_d_t                            vld_sel;
  n_d_t                            nvld_sel;
  //
  n_d_t                            cdb_req_n;
  //
  n_d_t                            iss_vld_d_w;
  issue_t [N-1:0]                  iss_d_w;
  //
  logic                            iss_vld_w;
  issue_t                          iss_w;

  // ======================================================================== //
  //                                                                          //
  // Combinational Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  function n_d_t ffs(n_d_t in); begin
    ffs  = '0;
    for (int i = $bits(n_d_t) - 1; i >= 0; i--)
      if (in [i])
        ffs  = ('b1 << i);
  end endfunction

  //
  function rs_entry_t to_rs_entry(dispatch_t d); begin
    to_rs_entry         = '0;
    to_rs_entry.opcode  = dis_r.opcode;
    to_rs_entry.oprand  = dis_r.oprand;
    to_rs_entry.tag     = dis_r.tag;
    to_rs_entry.robid   = dis_r.robid;
    to_rs_entry.imm     = dis_r.imm;
    to_rs_entry.wa      = dis_r.wa;
  end endfunction

  //
  function issue_t to_issue(rs_entry_t rs); begin
    to_issue            = '0;
    to_issue.rdata [0]  = rs.oprand [0].u.w;
    to_issue.rdata [1]  = rs.oprand [1].u.w;
    to_issue.op         = rs.opcode;
    to_issue.tag        = rs.tag;
    to_issue.imm        = rs.imm;
    to_issue.robid      = rs.robid;
    to_issue.wa         = rs.wa;
  end endfunction

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sel_PROC

      //
      rdy_sel   = iss_busy_r ? '0 : ffs( rs_rdy_r);

      //
      vld_sel   = ffs( rs_vld_r);

      //
      nvld_sel  = ffs(~rs_vld_r);

    end // block: sel_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : rs_dispatch_PROC

      //
      rs_vld_set   = '0;

      //
      cdb_req_n    = '0;

      //
      rs_rdy_set   = '0;

      //
      iss_vld_d_w  = '0;
      iss_d_w      = '0;

      //
      for (int i = 0; i < N; i++) begin

        //
        rs_entry_upt [i]  = rs_entry_r [i];
        rs_entry_en [i]   = 'b0;

        //
        casez ({//
                rs_vld_r [i], nvld_sel[i],

                //
                dis_vld_r, cdb_r.vld,

                //
                rs_rdy_r [i], rdy_sel [i]})

          // Current RS-entry is inactive and is selected for allocation to
          // incoming dispatched instruction.
          //
          6'b01_1?_??: begin
            rs_vld_set [i]    = 'b1;

            //
            rs_entry_en [i]   = 'b1;
            rs_entry_upt [i]  = to_rs_entry(dis_r);
          end

          // Current RS-entry is active, but not ready. Snoop inbound
          // traffic on CDB for writes to the same tag as those busy
          // oprands in the station.
          //
          6'b1?_?1_0?: begin
          end

          // The current station entry is valid, ready and is
          // scheduled for issue.
          //
          6'b1?_??_11: cdb_req_n [i]  = (~sch_r [LATENCY_N + 1]);

          default:;

        endcase // casez ({rs_vld_r [i]})

        // Bypass incoming CDB to current RS entry, whether already
        // present in the RS table or incoming from a dispatch.
        //
        for (int o = 0; o < 2; o++) begin

          //
          rs_latch_cdb [i][o]  = cdb_r.vld & rs_entry_upt [i].oprand [o].busy &
                                 (rs_entry_upt [i].oprand [o].u.t.tag == cdb_r.tag);

          if (rs_latch_cdb [i][o]) begin
            rs_entry_en [i]                   = 'b1;

            rs_entry_upt [i].oprand [o].busy  = 'b0;
            rs_entry_upt [i].oprand [o].u.w   = cdb_r.wdata;
          end
        end

        if (rs_entry_en [i])
          // If neither busy bits are set at the end of this round,
          // the station entry becomes ready for issue.
          //
          rs_rdy_set [i]   = ~(rs_entry_upt [i].oprand [0].busy |
                               rs_entry_upt [i].oprand [1].busy);

      end // for (int i = 0; i < N; i++)

    end // block: rs_dispatch_PROC

  // ------------------------------------------------------------------------ //
  //
  function n_d_t mask_msb (n_d_t in); begin
    mask_msb    = in;
    for (int i = 1; i < $bits(n_d_t); i++)
      mask_msb [i] |= mask_msb [i - 1];
  end endfunction

  always_comb
    begin : aging_PROC

      n_d_t do_shift  = '0;
//      n_d_t do_shift  = mask_msb(iss_vld_d_w);

      for (int i = 0; i < N; i++) begin
        //
        rs_vld_w [i]    = do_shift [i] ? rs_vld_upt [i + 1] : rs_vld_upt [i];
        rs_rdy_w [i]    = do_shift [i] ? rs_rdy_upt [i + 1] : rs_rdy_upt [i];
        rs_entry_w [i]  = do_shift [i] ? rs_entry_upt [i + 1] : rs_entry_upt [i];
      end

    end

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : rs_orr_PROC

      //
      cdb_req      = (|cdb_req_n);

      //
      iss_vld_d_w  = cdb_gnt ? cdb_req_n : '0;
      iss_vld_w    = (|iss_vld_d_w);

      //
      iss_d_w      = '0;
      for (int i = 0; i < N; i++) begin
        if (iss_vld_d_w [i])
          iss_d_w [i]  = to_issue(rs_entry_r [i]);
      end

      //
      iss_w  = '0;
      for (int i = 0; i < N; i++)
        iss_w    |= iss_d_w [i];

      //
      rs_vld_clr  = iss_vld_d_w;
      rs_rdy_clr  = iss_vld_d_w;

      //
      rs_vld_upt  = rs_vld_set | rs_vld_r & (~rs_vld_clr);
      rs_vld_en   = |{rs_vld_set, rs_vld_clr};

      //
      rs_rdy_upt  = rs_rdy_set | rs_rdy_r & (~rs_rdy_clr);
      rs_rdy_en   = |{rs_rdy_set, rs_rdy_clr};

    end // block: rs_orr_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < N; i++)
      if (rs_entry_en [i])
        rs_entry_r [i] <= rs_entry_w [i];

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      rs_vld_r <= '0;
    else if (rs_vld_en)
      rs_vld_r <= rs_vld_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      rs_rdy_r <= '0;
    else if (rs_rdy_en)
      rs_rdy_r <= rs_rdy_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      iss_vld_r <= 'b0;
    else
      iss_vld_r <= iss_vld_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (iss_vld_w)
      iss_r <= iss_w;

endmodule // tomasulo_rs
