//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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

module delay_pipe #(parameter int W = 1, parameter N = 1)
(
   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst

   //======================================================================== //
   //                                                                         //
   // In/Out                                                                  //
   //                                                                         //
   //======================================================================== //

   , input          [W-1:0]                       in
   //
   , output logic   [W-1:0]                       out_r
);


  typedef logic [W-1:0] word_t;
  typedef struct packed {
    word_t [N-1:0] w;
  } shift_t;
  shift_t shift_r, shift_w;

  // ------------------------------------------------------------------------ //
  //
  generate if (N == 0)

    //
    always_comb out_r  = in;

  endgenerate


  // ------------------------------------------------------------------------ //
  //
  generate if (N > 0)

    always_comb
      begin : shift_PROC
        shift_w           = '0;
        shift_w.w[0]      = in;
        for (int i = 1; i < N; i++)
          shift_w.w[i]  = shift_r.w[i - 1];
        //
        out_r           = shift_r.w[N - 1];
      end

    always_ff @(posedge clk)
      shift_r <= rst ? '0 : shift_w;

  endgenerate

endmodule // delay_pip
