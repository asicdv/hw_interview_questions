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

module multi_ported_sram_xor #(parameter int NUM_W = 1,
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
  //
  localparam int NUM_X  = NUM_R + (NUM_W - 1);
  localparam int NUM_Y  = NUM_W;

  localparam int ADDR_W  = $clog2(N);
  typedef logic [W-1:0]                 w_t;
  typedef logic [ADDR_W-1:0]            addr_t;
  typedef logic [$clog2(NUM_W)-1:0]     num_w_t;

  //
  logic  [NUM_X-1:0][NUM_Y-1:0]         dpsram__en0;
  addr_t [NUM_X-1:0][NUM_Y-1:0]         dpsram__addr0;
  w_t               [NUM_Y-1:0]         dpsram__din0;

  //
  logic [NUM_X-1:0][NUM_Y-1:0]          dpsram__en1;
  addr_t [NUM_X-1:0][NUM_Y-1:0]         dpsram__addr1;
  w_t   [NUM_X-1:0][NUM_Y-1:0]          dpsram__dout1;

  //
  logic [NUM_W-1:0]                     wdata_reg_en;
  w_t   [NUM_W-1:0]                     wdata_reg_r;
  w_t   [NUM_W-1:0]                     wdata_reg_w;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : read_PROC

      for (int rd_port = 0; rd_port < NUM_R; rd_port++) begin

        rdata [rd_port]  = '0;
        
        for (int y = 0; y < NUM_W; y++) begin

          //
          dpsram__en1 [rd_port][y + NUM_W - 1]    = ren [y];
          dpsram__addr1 [rd_port][y + NUM_W - 1]  = raddr [y * ADDR_W +: ADDR_W];
            
          //
          rdata [rd_port * W +: W]               ^= dpsram__dout1 [NUM_W + rd_port][y];

        end

      end

    end

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : write_PROC

      //
      for (int y = 0; y < NUM_Y; y++) begin
        wdata_reg_en [y]  = wen [y];
        wdata_reg_w [y]   = wdata [y * W +: W];
        
        //
        dpsram__din0 [y]  = wdata_reg_r [y];
      end

      //
      for (int y = 0; y < NUM_W; y++) begin

        int y_xor  = 0;
        
        for (int x = 0; x < NUM_W - 1; x++) begin

          if (y == y_xor)
            y_xor++;

          //
          dpsram__din0 [y]         ^= dpsram__dout1 [x][y_xor];

          //
          dpsram__en1 [x][y_xor]    = wen [y];
          dpsram__addr1 [x][y_xor]  = waddr [y * ADDR_W +: ADDR_W];

          y_xor++;
        end

      end // for (int y = 0; y < NUM_W; y++)

      //
      for (int y = 0; y < NUM_W; y++) begin

        for (int x = NUM_W; x < NUM_R + NUM_W - 1; x++) begin
          dpsram__en0 [x][y]    = wen [y];
          dpsram__addr0 [x][y]  = waddr [y * ADDR_W +: ADDR_W];
        end

      end // for (int y = 0; y < NUM_W; y++)

    end
  
  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < NUM_W; i++)
      if (wdata_reg_en [i])
        wdata_reg_r [i]    <= wdata_reg_w [i];
  
  // ======================================================================== //
  //                                                                          //
  // Instances                                                                //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  generate for (genvar x = 0; x < NUM_R + (NUM_W - 1); x++) begin

    for (genvar y = 0; y < NUM_W; y++) begin

      //
      dpsram #(.W(W), .N(N)) u_dpsram (
        //
          .clk0            (clk                   )
        , .en0             (dpsram__en0 [x][y]    )
        , .wen0            ('b1                   )
        , .addr0           (dpsram__addr0 [x][y]  )
        , .din0            (dpsram__din0 [y]      )
        //
        , .dout0           ()
        //
        , .clk1            (clk                   )
        , .en1             (dpsram__en1 [x][y]    )
        , .wen1            ('b0                   )
        , .addr1           (dpsram__addr1 [x][y]  )
        , .din1            ()
        //
        , .dout1           (dpsram__dout1 [x][y]  )
      );
    
    end

  end endgenerate

endmodule
