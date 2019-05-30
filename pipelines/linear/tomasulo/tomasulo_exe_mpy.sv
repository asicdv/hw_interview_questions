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

module tomasulo_exe_mpy (

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
  typedef struct packed {
    reg_t          wa;
    tag_t          tag;
    robid_t        robid;
  } delay_pipe_t;
  localparam int DELAY_PIPE_W  = $bits(delay_pipe_t);
  //
  cdb_t                                 cdb_w;
  logic                                 cdb_en;
  //
  logic [31:0]                          mpy__a;
  logic [31:0]                          mpy__b;
  logic                                 mpy__pass;
  //
  logic [1:0][31:0]                     mpy__y;
  logic                                 mpy__y_vld_r;
  //
  logic                                 mpy__busy_r;

  //
  delay_pipe_t                          delay_pipe_in;
  delay_pipe_t                          delay_pipe_out_r;

  //
  cdb_t                                 cdb;

  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cntrl_PROC

      // Ideally the RS_EXE should not issue back-to-back until the
      // completion from the preceeding instruction has been seen. To
      // do this correctly would necessitate a slight refactoring of
      // the RS logic. As the EXE is unique in so far that it is
      // non-pipelined, a quick and simple (but dirty) solution is to
      // simply apply back pressure on the valid issue - even though
      // it is not registered.
      //
      iss_busy_r     = (iss_vld | mpy__busy_r);

      //
      mpy__pass      = iss_vld;
      mpy__a         = iss.rdata [0];
      mpy__b         = iss.rdata [1];

      //
      cdb            = '0;
      cdb.vld        = mpy__y_vld_r;
      cdb.wdata      = mpy__y [0];
      cdb.tag        = delay_pipe_out_r.tag;
      cdb.wa         = delay_pipe_out_r.wa;
      cdb.robid      = delay_pipe_out_r.robid;

      //
      cdb_en         = (cdb.vld | cdb_r.vld);
      cdb_w          = cdb.vld ? cdb : '0;

      //
      delay_pipe_in  = '{wa:iss.wa, tag:iss.tag, robid:iss.robid};

    end // block: cntrl_PROC

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
  delay_pipe #(.W(DELAY_PIPE_W), .N(5)) u_delay_pipe (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .in                (delay_pipe_in      )
    , .out_r             (delay_pipe_out_r   )
  );
  
  // ------------------------------------------------------------------------ //
  //
  mlt5c #(.W(WORD_W)) u_mpy (
    //
      .clk               (clk                )
    , .rst               (rst                )
    //
    , .a                 (mpy__a             )
    , .b                 (mpy__b             )
    , .pass              (mpy__pass          )
    //
    , .y                 (mpy__y             )
    , .y_vld_r           (mpy__y_vld_r       )
    //
    , .busy_r            (mpy__busy_r        )
  );

endmodule // tomasulo_exe_logic
