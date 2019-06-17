# Hardware Interview Questions

## Introduction

The original object of this project was to collate solutions to the
standard hardware/computer engineering interview questions one may be
asked for a logic design position. Over the years, the collection of
solutions has grown from a relatively small collection of puzzles, to
a number of design examples that demonstrate commonly occuring idioms
that one may encounter in a wider setting not necessary during an
interview. In particular, the project has become a holding location
for small hardware designs that are not necessary large enough to
warrant an independent piece of work but are nonetheless interesting
for what they demonstrate.

All solutions in this work are implemented in standard System Verilog
and are compiled into SystemC models using Verilator. A C++ based
verification environment is present along with each solution to
demonstrate its correctness. All testbenches are integrated into the
Google Test unit test library and a full regression suite is available
through CTEST.

__DISCLAIMER:__ *This work is not necessary fit for any particular
purpose and is really only useful from a pedagogical perspective. In
addition, I have really just used it for the purpose of joting down
ideas that explore particular scenarios in logic design.*

## System Requirements
* cmake >= 3.2
* systemc >= 2.3.1 (built for C++14)
* verilator >= 3.9

## Build Steps (SIM)
~~~~
git clone https://github.com/stephenry/hw_interview_questions
cd hw_interview_questions
git submodule update --init --recursive
mkdir build
cd build
cmake ../
make -j
~~~~

## Regression
~~~~
ctest .
~~~~

## Verification Environment

A full regression is initiated through the CTEST front-end. When run
all registered tests shall be executed and shall raise a PASS or FAIL
flag. The implementation of regression suites with SystemC is a bit
challenging because of the restrictions imposed by its simulation
kernel.

```
Test project /home/stephenry/github.com/hw_interview_questions/b
      Start  1: tb_multiplier
 1/46 Test  #1: tb_multiplier ....................   Passed    1.76 sec
      Start  2: tb_div_by_3
 2/46 Test  #2: tb_div_by_3 ......................   Passed    0.52 sec
      Start  3: tb_fused_multiply_add
 3/46 Test  #3: tb_fused_multiply_add ............   Passed    2.23 sec
      Start  4: tb_increment
 4/46 Test  #4: tb_increment .....................   Passed    0.06 sec
      Start  5: tb_multiply_by_21
 5/46 Test  #5: tb_multiply_by_21 ................   Passed    0.05 sec
      Start  6: tb_simd
 6/46 Test  #6: tb_simd ..........................   Passed    0.15 sec
      Start  7: tb_ultra_wide_accumulator
 7/46 Test  #7: tb_ultra_wide_accumulator ........   Passed    1.50 sec
      Start  8: tb_count_ones
 8/46 Test  #8: tb_count_ones ....................   Passed    0.05 sec
      Start  9: tb_count_zeros_32
 9/46 Test  #9: tb_count_zeros_32 ................   Passed    0.11 sec
      Start 10: tb_detect_sequence
10/46 Test #10: tb_detect_sequence ...............   Passed    1.36 sec
      Start 11: tb_fibonacci
11/46 Test #11: tb_fibonacci .....................   Passed    0.02 sec
      Start 12: tb_missing_duplicated_word
12/46 Test #12: tb_missing_duplicated_word .......   Passed    0.28 sec
      Start 13: tb_multi_counter
13/46 Test #13: tb_multi_counter .................   Passed    0.08 sec

```

## Ancillary utilities

* __libv__

A collection of reusable logic primitives that are used throughout the
solutions (memories, encoders, queues). All of these components are
fairly self-explanatory and robust (although still not targeted
towards production use).

* __libtb__

A collection of reusable C++14 routines (integrated with SystemC)
which form the basis of the testbenches/verification environments of
each of the solutions presented. Overall this is fairly small and
non-intended for general use (as it is tangential to the overall
objective of the work).

## Notable Solutions

Although the majority of the design solutions are fairly
self-explanatory, there are a few that are notable for as a point of
interest.

* ___solution/control/vert_ucode_quicksort___

Some years ago, after having been laid-off I had shared a whisky with
a collection of ex-colleagues to lament having "not pursued a career
in software instead of hardware". During this meeting, a friend of a
friend (a software engineer), complained over having been asked about
the Quicksort algorithm during his failed attempt at a Google
Interview. Annoyed by his inability to describe a fundamental and
important algorithm, I decided that it be interesting to implement the
algorithm in hardware as a veritically micro-coded sequencer. Of
course, even though sorting is a rather atypical function performed by
hardware, (when performed it is typically achieved using a sorting
network), I decided that this would be a fairly tricky thing to
code up and get working.

* ___solution/pipeline/precompute___

I wanted to demonstrate some of the more sophisticated techniques
employed in the implementation of hardware pipelines (replays, stalls,
speculation). In this design example, a fictious 10-staged pipeline is
implemented with a single ALU, stalls/interlocks, and replays. The
novel aspect of this design is that the forwarding logic for the
pipeline is precomputed as instructions are issued into the
pipeline. This is a non-trivial task as the dependency vectors must be
propagated through the pipeline as some function of the stalls taking
place. Despite the complexity in RTL, the result is a smaller and more
optimized design as comparator logic need only be present at the issue
point in the pipeline and not at every stage prior to the ALU. In
addition, because dependencies are precomputed, forwarding takes
places from flopped state where otherwise it would have taken place
after the comparison and priority operations had completed.

* ___solution/design/sorted_lists___

Some year ago, I was asked a design question for a position in Wall
Street in a high-frequency trading position. Although I turned the
position down, this is the solution I came up with. It involved
maintaining a collection of contexts within a table, where each
context had a list of key, value pairs that were sorted on the
key. Contexts could be updated concurrently with queries to their
state. The design also had run at 150MHz, which is fairly aggressive
for an FPGA (the solution requires that the datapaths within the logic
have only partial forwarding to achieve timing).

* ___solution/arithmetic/simd___

SIMD is a common technique seen in most CPU. Implemented naively
however one would assume that it consisted of simply duplicating
multiple smaller ALU and driving each depending on the desired
operating mode. This is not the case. Instead, in SIMD, a single ALU
is partitioned such that the carrys are conditionally killed between
lanes. Although this is slight more complicated, and somewhat
non-obvious, it results in a faster and smaller design, as a single
CLA can be used whereas in the multiple ALU case, the carry-chain
would be broken each time a new ALU is used.
