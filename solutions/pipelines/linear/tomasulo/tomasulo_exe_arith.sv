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

module tomasulo_exe_arith #(parameter int LATENCY_N = 2) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // Issue                                                                   //
   //                                                                         //
   //======================================================================== //

   , input                                        iss_vld
   , input  tomasulo_pkg::issue_t                 iss
   //
   , output logic                                 iss_busy_r

   //======================================================================== //
   //                                                                         //
   // Completion                                                              //
   //                                                                         //
   //======================================================================== //

   , output tomasulo_pkg::cdb_t                   cdb_r
);
  import tomasulo_pkg::*;

  //
  cdb_t                                 cdb_w;
  logic                                 cdb_en;
  //
  cdb_t                                 delay_pipe_in;
  cdb_t                                 delay_pipe_out_r;

  //
  cdb_t                                 cdb;
  
  //
  function word_t exe(opcode_t op, word_t [1:0] r); begin
    exe  = '0;
    case (op)
      OP_ADD: exe   = r[0] + r[1];
      OP_SUB: exe   = r[0] - r[1];
      default: exe  = r[0];
    endcase // case (op)
  end endfunction

  // ======================================================================== //
  //                                                                          //
  // Combinational Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : exe_PROC

      //
      iss_busy_r  = '0;

      //
      cdb.vld     = iss_vld;
      cdb.tag     = iss_vld ? iss.tag : '0;
      cdb.wdata   = iss_vld ? exe(iss.op, iss.rdata) : '0;
      cdb.robid   = iss.robid;
      cdb.wa      = iss.wa;

      if (LATENCY_N > 1) begin

        //
        delay_pipe_in  = cdb;

        //
        cdb_en         = (delay_pipe_out_r.vld | cdb_r.vld);
        cdb_w          =  delay_pipe_out_r.vld ? delay_pipe_out_r: '0;

      end else begin

        //
        cdb_en  = (iss_vld | cdb_r.vld);
        cdb_w   = cdb.vld ? cdb : '0;

      end

    end // block: exe_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      cdb_r <= '0;
    else if (cdb_en)
      cdb_r <= cdb_w;

  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  generate if (LATENCY_N > 1) begin

    delay_pipe #(.W(CDB_W), .N(LATENCY_N - 1)) u_delay_pipe (
      //
        .clk                  (clk                     )
      , .rst                  (rst                     )
      //
      , .in                   (delay_pipe_in           )
      , .out_r                (delay_pipe_out_r        )
    );

  end endgenerate
  
endmodule // tomasulo_exe_arith
