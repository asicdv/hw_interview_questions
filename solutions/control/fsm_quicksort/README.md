# Origin

FSM design

# Problem

Implement the quicksort algorithm using an FSM.

# Implementation

Quicksort is the canonical sort algorithm in computer science. Rarely,
if ever, is it implemented in hardware. Nevertheless, the quicksort
algorithm, poses a number of challenges when implemented in hardware:
how is recursion supported, how are accesses to and from memory
implemented.

The objective of this exercise is to demonstrate a communicating
FSM-style with some additional modification to support the recursive
nature of the quicksort algorithm.

# Disclaimer

In VLSI, sorting is typically performed using a sort-network and
rarely would one choose to sort in this manner (a rather inefficient
style). Instead, here I demonstrate some techniques employed in
non-trivial state machine design.
