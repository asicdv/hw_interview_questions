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

module rmw_long_latency (

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
   , input rmw_long_latency_pkg::id_t        iss_id_r
   , input rmw_long_latency_pkg::op_t        iss_op_r
   , input rmw_long_latency_pkg::word_t      iss_imm_r
   //
   , output logic                            iss_rdy_w

   //======================================================================== //
   //                                                                         //
   // Completion                                                              //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            cmpl_vld_r
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
   , input                                    tbl_rd_word_vld_r
   , input  rmw_long_latency_pkg::word_t      tbl_rd_word_r
   , input  rmw_long_latency_pkg::tag_t       tbl_rd_ctag_r
   //
   , output logic                             tbl_rd_r
   , output rmw_long_latency_pkg::id_t        tbl_rd_id_r
   , output rmw_long_latency_pkg::tag_t       tbl_rd_itag_r
);
  import rmw_long_latency_pkg::*;
  
  
  //
  issue_t                             iss_r;
  //
  logic                               exe_iss_vld_r;
  word_t                              exe_iss_imm_r;
  op_t                                exe_iss_op_r;
  word_t                              exe_iss_reg_r;
  momento_t                           exe_iss_momento_r;
  //
  logic                               exe_wrbk_w;
  word_t                              exe_wrbk_word_w;
  momento_t                           exe_wrbk_momento_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : iss_PROC

      iss_r      = '0;
      iss_r.id   = iss_id_r;
      iss_r.imm  = iss_imm_r;
      iss_r.op   = iss_op_r;

    end // block: iss_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : exe_PROC

      //
      exe_wrbk_w          = exe_iss_vld_r;
      exe_wrbk_momento_w  = exe_iss_momento_r;

      //
      case (exe_iss_op_r)
        OP_ADDI: exe_wrbk_word_w  = exe_iss_reg_r + exe_iss_imm_r;
        OP_SUBI: exe_wrbk_word_w  = exe_iss_reg_r - exe_iss_imm_r;
        OP_MOVI: exe_wrbk_word_w  = exe_iss_imm_r;

        //
        OP_NOP:  exe_wrbk_word_w  = exe_iss_reg_r;
        default: exe_wrbk_word_w  = exe_iss_reg_r;
      endcase // case (exe_iss_op_r)
      
    end // block: exe_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  rmw_long_latency_cache u_rmw_long_latency_cache (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .iss_vld_r         (iss_vld_r          )
    , .iss_r             (iss_r              )
    , .iss_rdy_w         (iss_rdy_w          )
    //
    , .cmpl_vld_r        (cmpl_vld_r         )
    , .cmpl_word_r       (cmpl_word_r        )
    //
    , .tbl_wr_r          (tbl_wr_r           )
    , .tbl_wr_id_r       (tbl_wr_id_r        )
    , .tbl_wr_word_r     (tbl_wr_word_r      )
    //
    , .tbl_rd_word_vld_r (tbl_rd_word_vld_r  )
    , .tbl_rd_word_r     (tbl_rd_word_r      )
    , .tbl_rd_ctag_r     (tbl_rd_ctag_r      )
    //
    , .tbl_rd_r          (tbl_rd_r           )
    , .tbl_rd_id_r       (tbl_rd_id_r        )
    , .tbl_rd_itag_r     (tbl_rd_itag_r      )
    //
    , .exe_iss_vld_r     (exe_iss_vld_r      )
    , .exe_iss_imm_r     (exe_iss_imm_r      )
    , .exe_iss_op_r      (exe_iss_op_r       )
    , .exe_iss_reg_r     (exe_iss_reg_r      )
    , .exe_iss_momento_r (exe_iss_momento_r  )
    //
    , .exe_wrbk_w        (exe_wrbk_w         )
    , .exe_wrbk_word_w   (exe_wrbk_word_w    )
    , .exe_wrbk_momento_w(exe_wrbk_momento_w )
  );

endmodule // rmw_long_latency
