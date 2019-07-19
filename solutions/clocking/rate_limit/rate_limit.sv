//========================================================================== //
// Copyright (c) 2019, Stephen Henry
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

`include "libv_pkg.vh"

module rate_limit #(parameter int W = 32) (
   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

   //
     input                                   tx_clk
   , input                                   tx_rst

   //
   , input                                   rx_clk
   , input                                   rx_rst

   //======================================================================== //
   //                                                                         //
   // Push Interface                                                          //
   //                                                                         //
   //======================================================================== //

   , input                                   tx_valid
   , input [W-1:0]                           tx_data

   //======================================================================== //
   //                                                                         //
   // Pop Interface                                                           //
   //                                                                         //
   //======================================================================== //

   , output logic                            rx_valid
   , output logic [W-1:0]                    rx_data

   //======================================================================== //
   //                                                                         //
   // Control/Status Interface                                                //
   //                                                                         //
   //======================================================================== //

   //
   , output logic                            tx_full_r
);
  localparam int N  = 16;


  // Sender clock: 1.5 GHz; Receiver clock: 1.3 GHz
  //
  // --> 1.3 / 1.5 = 13 / 15 ~ 13 credits every 15 cycles.
  //
  localparam int CNT_TIMEOUT  = 15;
  localparam int CNT_INC  = 13;

  typedef logic [4:0]                        cnt_t;
  typedef logic [4:0]                        credit_t;
  typedef logic [5:0]                        credit1_t;
  typedef logic [W-1:0]                      word_t;

  //
  cnt_t                       cnt_r;
  cnt_t                       cnt_w;
  logic                       cnt_en;
  //
  logic                       cnt_timeout;
  //
  credit_t                    credit_r;
  credit_t                    credit1_w;
  logic                       credit_sat;
  logic                       credit_max;
  logic                       credit_zero;
  credit_t                    credit_w;
  logic                       credit_en;
  //
  logic                       credit_vld_w;
  logic                       credit_vld_r;
  //
  logic                       fifo_async__pop;
  word_t                      fifo_async__pop_data;
  logic                       fifo_async__pop_vld_r;
  word_t                      fifo_async__push_data;
  logic                       fifo_async__push;
  logic                       fifo_async__empty_r;
  logic                       fifo_async__full_r;
  //
  logic                       fifo_ad__push;
  word_t                      fifo_ad__push_data;
  logic                       fifo_ad__pop;
  word_t                      fifo_ad__pop_data;
  //
  logic                       fifo_ad__empty_r;
  logic                       fifo_ad__full_r;
  //
  logic                       did_dequeue;
  
  // ------------------------------------------------------------------------ //
  //
  logic                       dequeue;

  always_comb begin : leaky_bucket_PROC

    //
    dequeue      = credit_vld_r & (~fifo_ad__empty_r);

    cnt_timeout  = (cnt_r == CNT_TIMEOUT);
    cnt_w        = cnt_timeout ? 'b0 : (cnt_r + 'b1);

    credit1_w    = credit_r + CNT_INC;
    credit_sat   = (credit1_w > credit1_t'(N));
    credit_zero  = (credit_r == '0);

    casez ({cnt_timeout, credit_sat, dequeue, credit_zero})
      4'b11_??: credit_w = credit_t'(N);
      4'b10_??: credit_w = credit_t'(credit1_w);
      4'b0?_10: credit_w = credit_r - 'b1;
      default:  credit_w = credit_r;
    endcase

    credit_en     = (cnt_timeout | dequeue);
    credit_vld_w  = (credit_w != '0);

  end // block: leaky_bucket_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb begin : fifo_cntrl_PROC

    //
    fifo_ad__push          = tx_valid;
    fifo_ad__push_data     = tx_data;

    //
    fifo_ad__pop           = dequeue;

    //
    fifo_async__push       = dequeue;
    fifo_async__push_data  = fifo_ad__pop_data;

    //
    fifo_async__pop        = (~fifo_async__empty_r);
    
    //
    rx_valid               = fifo_async__pop_vld_r;
    rx_data                = fifo_async__pop_data;

    //
    tx_full_r              = fifo_ad__full_r;

  end // block: fifo_cntrl_PROC
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge tx_clk)
    if (tx_rst)
      cnt_r <= '0;
    else
      cnt_r <= cnt_w;

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge tx_clk)
    if (tx_rst)
      credit_r <= credit_t'(N);
    else if (credit_en)
      credit_r <= credit_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge tx_clk)
    credit_vld_r <= credit_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  fifo_basic #(.W(W), .N(4)) u_fifo_ad (
    //
      .clk                    (tx_clk                  )
    , .rst                    (tx_rst                  )
    //
    , .push                   (fifo_ad__push           )
    , .push_data              (fifo_ad__push_data      )
    //
    , .pop                    (fifo_ad__pop            )
    , .pop_data               (fifo_ad__pop_data       )
    //
    , .flush                  ('b0                     )
    , .empty_r                (fifo_ad__empty_r        )
    , .full_r                 (fifo_ad__full_r         )
  );


  // ------------------------------------------------------------------------ //
  //
  fifo_async #(.W(W), .N(N)) u_fifo_async (
    //
      .wclk                   (tx_clk                  )
    , .wrst                   (tx_rst                  )
    //
    , .rclk                   (rx_clk                  )
    , .rrst                   (rx_rst                  )
    //
    , .push                   (fifo_async__push        )
    , .push_data              (fifo_async__push_data   )
    //
    , .pop                    (fifo_async__pop         )
    , .pop_data               (fifo_async__pop_data    )
    , .pop_data_vld_r         (fifo_async__pop_vld_r   )
    //
    , .empty_r                (fifo_async__empty_r     )
    , .full_r                 (fifo_async__full_r      )
  );
  

endmodule // rate_limit
