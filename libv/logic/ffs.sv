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

module ffs #(parameter int W = 32,
	     parameter bit OPT_FIND_FIRST_ZERO = 0,
	     parameter int D = $clog2(W)) (

 //========================================================================== //
 //                                                                           //
 // Control                                                                   //
 //                                                                           //
 //========================================================================== //

   input [W-1:0]                        x

 , output logic [W-1:0]                 y
 , output logic [D-1:0]                 n
);

   // ======================================================================= //
   //                                                                         //
   // Combinatorial Logic                                                     //
   //                                                                         //
   // ======================================================================= //

   always_comb
     begin : ffs_PROC

        y = '0;
        n = '0;

        for (int i = W - 1; i >= 0; i--) begin

           if (x[i] == ~OPT_FIND_FIRST_ZERO) begin
              y = ('b1 << i);
              n = i[D-1:0];
           end

        end
     end // block: ffs_PROC

endmodule
