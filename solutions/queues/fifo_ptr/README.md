# Problem Statement

Construct a power-of-2 Queue (FIFO).

# Commentary

Two pointers are maintained: a read pointer, a write pointer. Each
pointer is one bit wider than the LOG2CEIL of the FIFO capacity. This
additional bit is used to disambigutate between the EMPTY and FULL
conditions. During these conditions, the modulo of the pointers
relative to the size is zero. The full condition is set whenever the
pointer differ only in the MSB. The empty condition is set whenever
the pointers are equal.

The use of additional state (in this case an additional bit in the
MSB) is a very common idiom in hardware design. This technique is also
used to detect overflow (when two signed numbers are summed to produce
a negative number).
