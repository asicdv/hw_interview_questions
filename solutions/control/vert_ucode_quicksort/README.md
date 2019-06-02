# Origin

Micro-processors/-sequencers

# Problem Statement

Sort an input array using the quicksort algorithm implemented as a
vertically encoded micro-sequencer.

# Implementation

Any sufficiently complex controller, particularly one that is
programmable, begins to appear not dissimilar to a traditional
microprocessor datapath. Micro-code represents the signals used within
a programmable data path to enable to various gates to perform some
operation (mux enable, carry-in, etc.) Considering that such
micro-code is highly redundant, we are able to encode it into a more
efficient format to be subsequently decoded. This, in essence, is no
different from the operation of a CPU.

The complexity in the implementation of quicksort in a microsequencer,
stems from the need to construct the instruction set upon which the
algorithm is executed. Similary, we must construct the structures
required for pre-emption and retention of the programs architectural
state.

# Disclaimer

In VLSI, sorting is (more often than not) performed using a "sorting
network" consisting of a series of parallel compare and swap style
blocks.

The objective of this design, is not necessarily to demonstrate how
sorting is done, but instead used to illustrate the implementation of
a sufficiently complex algorithm in hardware using a micro-sequenced
style.
