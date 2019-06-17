c# Hardware Interview Questions

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

## Notable Solutions

Although the majority of the design solutions are fairly
self-explanatory, there are a few that are notable for as a point of
interest.

* ___solution/control/vert_ucode_quicksort__

* ___solution/pipeline/precompute___

* ___solution/design/sorted_lists___
