// ==================================================================== //
// Copyright (c) 2017, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in
//   the documentation and/or other materials provided with the
//   distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
// OF THE POSSIBILITY OF SUCH DAMAGE.
// ==================================================================== //

module rob #(parameter int CMPL_W = 32,
             parameter int ALLOC_W = 32,
             parameter int N = 16) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Allocation                                                              //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   alloc_vld
   , input  [ALLOC_W - 1:0]                  alloc_data
   //
   , output logic                            alloc_rdy
   , output logic [$clog2(N) - 1:0]          alloc_id

   //======================================================================== //
   //                                                                         //
   // Completion                                                              //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   cmpl_vld
   , input [$clog2(N) - 1:0]                 cmpl_id
   , input [CMPL_W - 1:0]                    cmpl_data

   //======================================================================== //
   //                                                                         //
   // Retirement                                                              //
   //                                                                         //
   //======================================================================== //

   , input                                   retire_rdy
   //
   , output logic [CMPL_W - 1:0]             retire_cmpl_data
   , output logic [ALLOC_W - 1:0]            retire_alloc_data            
   , output logic                            retire_vld

   //======================================================================== //
   //                                                                         //
   // Status/Cntrl                                                            //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   clear
   //
   , output logic                            idle_r
   , output logic                            full_r
);
  //
  typedef logic [$clog2(N)-1:0]  id_t;
  typedef logic [N - 1:0]        id_d_t;
  typedef logic [ALLOC_W - 1:0]  alloc_word_t;
  typedef logic [CMPL_W - 1:0]   cmpl_word_t;

  //
  alloc_word_t  [N - 1:0]                    alloc_mem_r;
  logic                                      alloc_mem_en;
  //
  cmpl_word_t  [N - 1:0]                     cmpl_mem_r;
  logic                                      cmpl_mem_en;
  //
  logic                                      idle_w;
  logic                                      full_w;
  //
  logic                                      al_alloc_vld;
  id_t                                       al_alloc_id;
  logic                                      al_free_vld;
  id_t                                       al_free_id;
  logic                                      al_clear;
  logic                                      al_idle_r;
  logic                                      al_busy_r;
  id_d_t                                     al_state_w;
  id_d_t                                     al_state_r;
  //
  logic                                      cm_alloc_vld;
  id_t                                       cm_alloc_id;
  logic                                      cm_free_vld;
  id_t                                       cm_free_id;
  logic                                      cm_clear;
  logic                                      cm_idle_r;
  logic                                      cm_busy_r;
  id_d_t                                     cm_state_w;
  id_d_t                                     cm_state_r;
  //
  id_t                                       rd_ptr_r;
  id_t                                       rd_ptr_w;
  logic                                      rd_ptr_en;
  //
  id_t                                       wr_ptr_r;
  id_t                                       wr_ptr_w;
  logic                                      wr_ptr_en;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : rob_PROC

      //
      alloc_rdy          = (~al_busy_r);
      alloc_id           = wr_ptr_r;

      //
      retire_vld         = cm_state_r [rd_ptr_r];
      retire_alloc_data  = alloc_mem_r [rd_ptr_r];
      retire_cmpl_data   = cmpl_mem_r [rd_ptr_r];

      //
      al_alloc_vld       = alloc_vld & alloc_rdy;
      al_alloc_id        = alloc_id;
      al_free_vld        = retire_vld & retire_rdy;
      al_free_id         = rd_ptr_r;
      al_clear           = clear;

      //
      cm_alloc_vld       = cmpl_vld;
      cm_alloc_id        = cmpl_id;
      cm_free_vld        = retire_vld & retire_rdy;
      cm_free_id         = rd_ptr_r;
      cm_clear           = clear;

      //
      idle_w             = (al_state_w == '0);
      full_w             = (al_state_w == '1);

      //
      wr_ptr_w           = clear ? '0 : wr_ptr_r + 'b1;
      wr_ptr_en          = clear | (alloc_vld & alloc_rdy);

      //
      rd_ptr_w           = clear ? '0 : rd_ptr_r + 'b1;
      rd_ptr_en          = clear | (retire_vld & retire_rdy);

      //
      alloc_mem_en       = alloc_vld & alloc_rdy;
      cmpl_mem_en        = cmpl_vld;
      
    end // block: rob_PROC

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (alloc_mem_en)
      alloc_mem_r [alloc_id] <= alloc_data;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (cmpl_mem_en)
      cmpl_mem_r [cmpl_id] <= cmpl_data;
  
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
      wr_ptr_r <= '0;
    else if (wr_ptr_en)
      wr_ptr_r <= wr_ptr_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      idle_r <= '1;
    else
      idle_r <= idle_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      full_r <= '0;
    else
      full_r <= full_w;

  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  flm #(.N(N)) u_flm_allocated (
    //
      .clk               (clk           )
    , .rst               (rst           )
    //
    , .alloc_vld         (al_alloc_vld  )
    , .alloc_id          (al_alloc_id   )
    //
    , .free_vld          (al_free_vld   )
    , .free_id           (al_free_id    )
    //
    , .clear             (al_clear      )
    //
    , .idle_r            (al_idle_r     )
    , .busy_r            (al_busy_r     )
    , .state_w           (al_state_w    )
    , .state_r           (al_state_r    )
  );
                            
  // ------------------------------------------------------------------------ //
  //
  flm #(.N(N)) u_flm_completed (
    //
      .clk               (clk           )
    , .rst               (rst           )
    //
    , .alloc_vld         (cm_alloc_vld  )
    , .alloc_id          (cm_alloc_id   )
    //
    , .free_vld          (cm_free_vld   )
    , .free_id           (cm_free_id    )
    //
    , .clear             (cm_clear      )
    //
    , .idle_r            (cm_idle_r     )
    , .busy_r            (cm_busy_r     )
    , .state_w           (cm_state_w    )
    , .state_r           (cm_state_r    )
  );

endmodule // rob
