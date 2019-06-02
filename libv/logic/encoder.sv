//========================================================================== //
// Copyright (c) 2016, Stephen Henry
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

module encoder #(parameter int W = 32,
                 parameter bit OPT_PRIORITY = 1,
                 parameter int E = $clog2(W)) (

 //========================================================================== //
 //                                                                           //
 // Control                                                                   //
 //                                                                           //
 //========================================================================== //

   input [W-1:0]                        x

 , output logic [E-1:0]                 n
);

   if (OPT_PRIORITY) begin

      always_comb
        begin : priority_PROC
           n = '0;

           // Priorized encoder circuit.
           //
           for (int i = W - 1; i >= 0; i--) begin
              if (x[i])
                n = i[E-1:0];
           end
        end

   end

   if (!OPT_PRIORITY) begin

      always_comb
        begin : no_priority_PROC
           n = '0;
           // Require one bit to be set within input vector otherwise
           // computed result is invalid; computation however it
           // faster than priority version.
           //
           for (int i = 0; i < W; i++)
             n |= ({E{x[i]}} & i[E-1:0]);
        end

   end

endmodule // encoder
