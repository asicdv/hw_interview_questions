## ==================================================================== ##
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
import collections
import math

# Script to programmatically genereate a Brent Kung carry tree from an
# initial width. Refer to Fig. 6.9 from Computer Arithmetic by Parhami
# for further information.
#
# Script generates two recursive trees: an up tree for odd indices and
# an down tree for the even indices.

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

`include "libv2_pkg.vh"

// **** THIS FILE IS AUTO-GENERATED. DO NOT MODIFY ****

module carry_chain_brent_kung_{0} (

   //======================================================================== //
   //                                                                         //
   // Generate/Propagate                                                      //
   //                                                                         //
   //======================================================================== //

   //
     input logic [{0}-1:0]                         p
   , input logic [{0}-1:0]                         g

   //======================================================================== //
   //                                                                         //
   // Carry-In                                                                //
   //                                                                         //
   //======================================================================== //
     
   //  
   , output logic [{0}:0]                          c
);

  typedef logic [{0}-1:0]                w_t;

  // ------------------------------------------------------------------------ //
  //
  always_comb
    begin : bk_PROC

      libv2_pkg::pg_t pg [{0}];

      // Initialize var
      for (int i = 0; i < {0}; i++)
        pg [i]  = {{ p[i], g[i] }};

"""

VERILOG_POSTAMBLE="""
      // Update output vector
      c  = '0;
      for (int i = 1; i < {0} + 1; i++) 
        c [i]  = pg [i - 1].g;

    end // block: bk_PROC

endmodule // carry_chain_brent_kung_{0}
"""

if len(sys.argv[1:]) == 0:
    print("Invalid argument count")
    sys.exit(1)

W=int(sys.argv[1])
FN="carry_chain_brent_kung_{0}.sv".format(W)
out=open(FN, 'w')

NODES=collections.defaultdict(list)

LEVEL_UP_MAX = int(math.log(W,2))

def to_verilog(j, i):
    return "pg [{0}] = libv2_pkg::lkpg(pg [{0}], pg [{1}]);\n".format(j, i)

def tree_up(idx, level, span):
    """Generate upwards carry tree of odd vector indices
    """
    if level == 0:
        return

    NODES[level].append(to_verilog(idx - 1, idx - span - 1))
    tree_up(idx, level - 1, span / 2)
    tree_up(idx - span, level - 1, span / 2)


def tree_down(idx, level, span):
    if span == 0:
        return

    NODES[level].append(to_verilog(idx + span - 1, idx - 1))
    tree_down(idx + span, level + 1, span / 2)
    tree_down(idx, level + 1, span /2)

def generate_down_tree():
    """Generate downwards carry tree of even vector indices
    """
    START=W/2
    while True:
        span = START/2
        if span == 0:
            break
        tree_down(START, LEVEL_UP_MAX, span)
        START=START/2

def emit_verilog():
    out.write(VERILOG_PREAMBLE.format(W))
    for level in NODES:
        out.write("// Level: %d\n" %  level)
        for rng in NODES[level]:
            out.write(rng)
        out.write("//\n")
    out.write(VERILOG_POSTAMBLE.format(W))

tree_up(W, LEVEL_UP_MAX, W / 2)
generate_down_tree()
emit_verilog()
sys.exit(0)
