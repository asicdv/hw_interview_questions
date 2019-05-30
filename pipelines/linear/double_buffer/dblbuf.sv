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

module dblbuf #(parameter int W = 32) (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                        clk
   , input                                        rst


   //======================================================================== //
   //                                                                         //
   // Control                                                                 //
   //                                                                         //
   //======================================================================== //

   //
   , input                                        A__in_vld
   , input        [W-1:0]                         A__in_w

   , output logic                                 B__out_vld_r
   , output logic [W-1:0]                         B__out_r

   //
   , input                                        B__stall

   , output logic                                 A__stall_r

);

  //
  typedef logic [W-1:0]       buffer_t;

  //
  buffer_t [1:0]                             buffer_r;
  buffer_t [1:0]                             buffer_w;
  logic    [1:0]                             buffer_en;
  //
  logic [1:0]                                buffer_vld_r;
  logic [1:0]                                buffer_vld_w;
  
  // ======================================================================== //
  //                                                                          //
  // Combinatorial Logic                                                      //
  //                                                                          //
  // ======================================================================== //
  
  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : cntrl_PROC

      //
      buffer_en [0]  = (A__in_vld | buffer_vld_r [1]) & (~B__stall);
      buffer_en [1]  =  A__in_vld & buffer_vld_r [0]  &   B__stall;

      //
      buffer_w [0]   = buffer_vld_r [1] ? buffer_r [1] : A__in_w;
      buffer_w [1]   = A__in_w;
      
      //
      casez ({buffer_vld_r, A__in_vld, B__stall})
        4'b00_1?: buffer_vld_w [0]  = 'b1;
        4'b01_00: buffer_vld_w [0]  = 'b0;
        default:  buffer_vld_w [0]  = buffer_vld_r [0];
      endcase

      //
      casez ({buffer_vld_r, A__in_vld, B__stall})
        4'b01_11: buffer_vld_w [1]  = 'b1;
        4'b11_?0: buffer_vld_w [1]  = 'b0;
        default:  buffer_vld_w [1]  = buffer_vld_r [1];
      endcase
                          
      //
      A__stall_r    = buffer_vld_r [1];

      //
      B__out_vld_r  = buffer_vld_r [0];
      B__out_r      = buffer_r [0];
      
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
      buffer_vld_r <= 'b0;
    else
      buffer_vld_r <= buffer_vld_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < 2; i++)
      if (buffer_en [i])
        buffer_r [i] <= buffer_w [i];

endmodule // dblbuf

  
