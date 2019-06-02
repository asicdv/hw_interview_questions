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

`include "multi_ported_sram_pkg.svh"

module multi_ported_sram_lvt_flop #(parameter int NUM_W = 1,
                         parameter int NUM_R = 1,
                         parameter int W = 32,
                         parameter int N = 8) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // Read Interface                                                          //
   //                                                                         //
   //======================================================================== //

   //
   , input        [NUM_R-1:0]                ren
   , input        [(NUM_R*$clog2(N))-1:0]    raddr
   //
   , output logic [(NUM_R*W)-1:0]            rdata

   //======================================================================== //
   //                                                                         //
   // Write Interface                                                         //
   //                                                                         //
   //======================================================================== //

   //
   , input        [NUM_W-1:0]                wen
   , input        [(NUM_W*$clog2(N))-1:0]    waddr
   , input        [(NUM_W*W)-1:0]            wdata

   //======================================================================== //
   //                                                                         //
   // Status/Control                                                          //
   //                                                                         //
   //======================================================================== //

   , input                                   init
   //
   , output logic                            busy_w
);
  localparam int ADDR_W  = $clog2(N);

  typedef logic [W-1:0]                 w_t;
  typedef logic [ADDR_W-1:0]            addr_t;
  typedef logic [$clog2(NUM_W)-1:0]     num_w_t;
  
  logic [NUM_R-1:0]                     sram_1rNw__ren;
  addr_t [NUM_R-1:0]                    sram_1rNw__raddr;
  w_t [NUM_R-1:0][NUM_W-1:0]            sram_1rNw__rdata;
  //
  logic [NUM_W-1:0]                     sram_1rNw__wen;
  addr_t [NUM_W-1:0]                    sram_1rNw__waddr;
  w_t [NUM_W-1:0]                       sram_1rNw__wdata;
  //
  logic [NUM_R-1:0]                     lvt_rf__ren;
  addr_t [NUM_R-1:0]                    lvt_rf__ra;
  num_w_t [NUM_R-1:0]                   lvt_rf__rdata_r;
  //
  logic  [NUM_W-1:0]                    lvt_rf__wen;
  addr_t [NUM_W-1:0]                    lvt_rf__wa;
  num_w_t [NUM_W-1:0]                   lvt_rf__wdata;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : sram_1rNw_PROC

      //
      sram_1rNw__ren    = ren;
      sram_1rNw__raddr  = raddr;

      //
      sram_1rNw__wen    = wen;
      sram_1rNw__waddr  = waddr;
      sram_1rNw__wdata  = wdata;
    
    end // block: sram_1rNw_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : lvt_rf_PROC

      //
      lvt_rf__wen    = wen;

      //
      lvt_rf__wa = '0;
      for (int i = 0; i < NUM_W; i++)
        lvt_rf__wa [i]  = waddr [i * ADDR_W +: ADDR_W];

      //
      lvt_rf__wdata     = '0;
      for (int i = 0; i < NUM_W; i++)
        lvt_rf__wdata [i]  = num_w_t'(i);

      //
      lvt_rf__ren          = ren;

      //
      lvt_rf__ra           = '0;
      for (int i = 0; i < NUM_R; i++)
        lvt_rf__ra [i]  = raddr [i * ADDR_W +: ADDR_W];

    end // block: lvt_rf_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : out_PROC

      rdata       = '0;

      for (int i = 0; i < NUM_R; i++) begin

        w_t d       = '0;

        for (int j = 0; j < NUM_W; j++)
          if (lvt_rf__rdata_r [j] == num_w_t'(j))
            d |= sram_1rNw__rdata [i][j];
        
        rdata [i * W +: W]  = d;
      end

    end // block: out_PROC
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  rf #(.W($clog2(NUM_W)), .N(N), .FLOP_OUT('b1), .WR_N(NUM_W), .RD_N(NUM_R))
     u_lvt_rf (
     //
       .clk                   (clk                )
     , .rst                   (rst                )
     //
     , .ra                    (lvt_rf__ra         )
     , .ren                   (lvt_rf__ren        )
     , .rdata                 (lvt_rf__rdata_r    )
     //
     , .wa                    (lvt_rf__wa         )
     , .wen                   (lvt_rf__wen        )
     , .wdata                 (lvt_rf__wdata      )
  );
  
  // ------------------------------------------------------------------------ //
  //
  generate for (genvar g = 0; g < NUM_W; g++) begin

    //
    sram_1rNw #(.W(W), .N(N), .NUM_R(NUM_R)) u_sram_1rNw (
      //
        .clk             (clk                      )
      , .rst             (rst                      )
      //
      , .ren             (sram_1rNw__ren           )
      , .raddr           (sram_1rNw__raddr         )
      , .rdata           (sram_1rNw__rdata [g]     )
      //
      , .wen             (sram_1rNw__wen [g]       )
      , .waddr           (sram_1rNw__waddr [g]     )
      , .wdata           (sram_1rNw__wdata [g]     )
    );
    
  end endgenerate

endmodule
