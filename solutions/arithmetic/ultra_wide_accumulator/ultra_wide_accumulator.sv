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

module ultra_wide_accumulator (

   //======================================================================== //
   //                                                                         //
   // Misc.                                                                   //
   //                                                                         //
   //======================================================================== //

     input                                   clk
   , input                                   rst

   //======================================================================== //
   //                                                                         //
   // In                                                                      //
   //                                                                         //
   //======================================================================== //

   //
   , input                                   pass
   , input                                   clear
   , input     [127:0]                       x

   //======================================================================== //
   //                                                                         //
   // Out                                                                     //
   //                                                                         //
   //======================================================================== //

   //
   , output logic [127:0]                    y_r
   , output logic                            y_vld_r
);

  typedef logic [31:0]                       w_t;

  typedef struct packed {
    w_t [3:0] w;
  } l_t;

  typedef struct packed {
    logic        v;
    logic [3:0]  c;
    w_t   [3:0]  d;
    l_t          a;
    l_t          b;
  } ucode_t;

  //
  l_t               csa_0_w;
  l_t               csa_1_w;
  l_t               csa_2_w;
  //
  l_t               acc_0_r, acc_0_w;
  l_t               acc_1_r, acc_1_w;
  logic             acc_en;
  //
  ucode_t [3:0]     ucode_w;
  ucode_t [3:0]     ucode_r;
  logic [3:0]       valid_w;
  logic [3:0]       valid_r;
  

  function logic [1:0] compress_3_to_2 (input [2:0] x); begin
    return { &x[1:0] | (x[2] & (|x[1:0])), (^x) };
  end endfunction

  function l_t [1:0] compress_3_to_2_v (l_t a, l_t b, l_t c); begin
    l_t ret1, ret0;
    for (int i = 0; i < 128; i++)
      { ret1 [i], ret0 [i] }  = compress_3_to_2({a[i], b[i], c[i]});
    return { ret1 << 1, ret0 };
  end endfunction

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
      csa_0_w               = x;
      csa_1_w               = clear ? '0 : acc_0_r;
      csa_2_w               = clear ? '0 : acc_1_r;

      //
      { acc_0_w, acc_1_w }  = compress_3_to_2_v(csa_0_w, csa_1_w, csa_2_w);

      //
      acc_en                = (clear | pass);

    end // block: cntrl_PROC

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : pipe_PROC

      //
      valid_w                           = { valid_r [2:0], pass };

      // Adder 0
      ucode_w [0]                       = '0;
      ucode_w [0].a                     = acc_0_w;
      ucode_w [0].b                     = acc_1_w;
      { ucode_w [0].c [0], ucode_w [0].d [0]}  =
          (ucode_w [0].a.w [0] + ucode_w [0].b.w [0]);
      
      // Adder 1
      ucode_w [1]                       = ucode_r [0];
      { ucode_w [1].c [1], ucode_w [1].d [1] }  =
          (ucode_r [0].a.w [1] + ucode_r [0].b.w [1] + w_t'(ucode_r [0].c [0]));
      
      // Adder 2
      ucode_w [2]                       = ucode_r [1];
      { ucode_w [2].c [2], ucode_w [2].d [2] }  =
          (ucode_r [1].a.w [2] + ucode_r [1].b.w [2] + w_t'(ucode_r [1].c [1]));
      
      // Adder 3
      ucode_w [3]                       = ucode_r [2];
      { ucode_w [3].c [3], ucode_w [3].d [3] }  =
          (ucode_r [2].a.w [3] + ucode_r [2].b.w [3] + w_t'(ucode_r [2].c [2]));

      y_r                               = { ucode_r [3].d [3],
                                            ucode_r [3].d [2],
                                            ucode_r [3].d [1],
                                            ucode_r [3].d [0]
                                          }
                                        ;

      //
      y_vld_r                           = valid_r [3];

    end // block: pipe_PROC

  // ======================================================================== //
  //                                                                          //
  // Sequential Logic                                                         //
  //                                                                          //
  // ======================================================================== //

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst) begin
      acc_0_r <= '0;
      acc_1_r <= '0;
    end else if (acc_en) begin
      acc_0_r <= acc_0_w;
      acc_1_r <= acc_1_w;
    end

  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    if (rst)
      valid_r <= '0;
    else
      valid_r <= valid_w;
  
  // ------------------------------------------------------------------------ //
  //
  always_ff @(posedge clk)
    for (int i = 0; i < 4; i++) begin
      if (valid_w [i])
        ucode_r [i] <= ucode_w [i];
    end

endmodule // ultra_wide_accumulator
