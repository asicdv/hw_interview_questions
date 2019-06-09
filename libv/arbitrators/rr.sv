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

module rr #(parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

   //
     input                              clk
   , input                              rst

   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   , input        [W-1:0]               req
   , input                              ack
   , output logic [W-1:0]               gnt
);

  typedef logic [W-1:0]                 w_t;
  
  w_t                      idx_r;
  w_t                      idx_w;
  logic                    idx_en;

  logic                    gnt_set;

  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //

  //
  function w_t rll (w_t w); begin
    return { w[$bits(w_t)-2:0], w[$bits(w_t)-1] };
  end endfunction

  //
  function w_t mask_left_inclusive(w_t w); begin
    w_t r  = 0;
    for (int i = 0; i < $bits(w_t); i++) begin
      if  (w [i])
        r [i]  = 'b1;

      if (i > 0)
        r [i]   |= r [i - 1];
    end
    return r;
  end endfunction

  //
  function w_t ffs(w_t w); begin
    w_t r = 0;
    for (int i = $bits(w_t) - 1; i >= 0; i--)
      if (w [i])
        r  = w_t'(1 << i);
    return r;
  end endfunction

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : rr_PROC

      w_t mask  = mask_left_inclusive(idx_r);

      w_t lsel  = req &   mask;
      w_t rsel  = req & (~mask);

      //
      w_t gsel  = (|lsel) ? lsel : rsel;

      //
      gnt       = ffs(gsel);
      gnt_set   = (gnt != '0);
      
      // Really shouldn't tie ack high even when there is no REQ as this
      // can push the arbiter into an invalid state.
`define ACK_ON_NGRANT
      
`ifdef ACK_ON_NGRANT
      idx_w     = gnt_set ? rll(gnt) : idx_r;
`else
      idx_w     = rll(gnt);
`endif

    end // block: rr_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb idx_en  = ack;

  // ======================================================================== //
  //                                                                          //
  // Flops                                                                    //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      idx_r <= 'b1;
    else if (idx_en)
      idx_r <= idx_w;

endmodule
