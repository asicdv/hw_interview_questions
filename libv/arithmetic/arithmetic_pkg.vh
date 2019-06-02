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

// Fast adder carry lookahead topology.
//
typedef enum logic [2:0] {
                          INFERRED     = 3'b000,
                          KOGGE_STONE  = 3'b001,
                          BRENT_KUNG   = 3'b010,
                          HYBRID       = 3'b011
                          } adder_carry_t;

typedef struct packed {
  logic        p;
  logic        g;
} pg_t;

function pg_t lkpg (pg_t p2, pg_t p1); begin
  pg_t ret  = '0;
  ret.p     = (p1.p & p2.p);
  ret.g     = p2.g | (p1.g & p2.p);
  return ret;
end endfunction

// Incrementer type
//
typedef enum logic [1:0] {
                          INFERED  = 2'b00,
                          INVZERO  = 2'b01
                          } increment_t;

function logic [1:0] compress_3_to_2(logic [2:0] x); begin
  return { (&x[1:0]) | (x[2] & |x[1:0]), (^x) };
end endfunction

