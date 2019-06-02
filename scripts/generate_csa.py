## Copyright (c) 2017, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in
##   the documentation and/or other materials provided with the
##   distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
## FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
## COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
## INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
## STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
## OF THE POSSIBILITY OF SUCH DAMAGE.
## ==================================================================== ##

import sys

# Simple script to auto-generate and emit a NxWb CSA tree.

N=int(sys.argv[1])
W=int(sys.argv[2])

VERILOG_PREAMBLE="""
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

`include "libv_pkg.vh"

// **** THIS FILE IS AUTO-GENERATED. DO NOT MODIFY ****

module csa_N{0}_{1}b (
  //
    input  logic [{0}*{1}-1:0]          x
  //
  , output logic [{1}-1:0]              y1
  , output logic [{1}-1:0]              y2
);

  //
  typedef logic [{1}-1:0]               w_t;

  // ------------------------------------------------------------------------ //
  //
  function w_t [1:0] v_compress_3_to_2(w_t a, w_t b, w_t c); begin
    w_t r  = '0;
    w_t s  = '0;
    for (int i = 0; i < $bits(w_t); i++)
      {{s [i], r [i]}}  = libv_pkg::compress_3_to_2({{a[i], b[i], c[i]}});
    return {{s << 1, r}};
  end endfunction

""".format(N, W)

VERILOG_POSTAMBLE="""
endmodule // csa_N{0}_{1}b
""".format(N, W)

LL="""
  // ------------------------------------------------------------------------ //
  //
"""

FN="csa_N{0}_{1}b.sv".format(N, W)
out=open(FN, 'w')
LN=lambda: out.write("\n")

def emit_preamble():
    out.write("  //\n")
    for i in range(N):
        out.write("  w_t    w_rnd0_{0};\n".format(i))
    LN()
    out.write("  always_comb\n")
    out.write("    begin : preamble_PROC\n")
    LN()
    for i in range(N):
        lo = i * W
        hi = (i + 1) * W - 1
        out.write("    w_rnd0_{0} = x [{1}:{2}];\n".format(i, hi, lo))
    LN()
    out.write("    end // preamble_PROC \n")

def emit_round(left, rnd):
    i = 0
    j = 0
    remain = left

    proc = ""
    proc += "  always_comb\n"
    proc += "    begin : rnd{0}_PROC\n".format(rnd)
    while left > 2:
        proc += "      //\n"
        proc += "      {{ w_rnd{0}_{1}, w_rnd{0}_{2} }} =".format(rnd, i, i + 1)
        proc += " v_compress_3_to_2(w_rnd{0}_{1}, w_rnd{0}_{2}, w_rnd{0}_{3});\n".format(
            rnd - 1, j, j + 1, j + 2)
        i += 2
        j += 3
        left -= 3
        remain -= 1
    for _ in range(left):
        proc += "      w_rnd{0}_{1} = w_rnd{2}_{3};\n".format(rnd, i, rnd - 1, j)
        i += 1
        j += 1

    out.write(LL)
    for i in range(remain):
        out.write("  w_t    w_rnd{0}_{1};\n".format(rnd, i))
    LN()
    out.write(proc)
    LN()
    out.write("    end // rnd{0}_PROC\n".format(rnd))
    return remain

def emit_out(final_round):
    comb = "\n"
    comb += LL
    comb += "always_comb\n"
    comb += "  begin : out_PROC\n"
    comb += "\n"
    comb += "    y1 = w_rnd{0}_0;\n".format(final_round)
    comb += "    y2 = w_rnd{0}_1;\n".format(final_round)
    comb += "\n"
    comb += "  end // out_PROC\n"
    out.write(comb)

def emit_verilog():
    out.write(VERILOG_PREAMBLE)
    emit_preamble()
    (left, rnd) = (N, 0)
    while left > 2:
        rnd += 1
        left = emit_round(left, rnd)
    emit_out(rnd)
    out.write(VERILOG_POSTAMBLE)

emit_verilog()
sys.exit(0)
