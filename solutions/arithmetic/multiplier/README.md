# Problem Statement

Explain the design and implementation of a hardware multiplier.

# Commentary

For, y = A * B, where A-bits is the width of vector 'A'.

Multiplication is simply the repeated addition of shifted variants of
the multiplicand for each non-zero bit within the multiplier. The
implementation of this in gates, appears something not dissimillar to
a CSA chain with inputs of A shifted one place for each input masked
against the bit at the corresponding location in B.

The inputs to the CSA chain can be reduced by employing Booth recoding
of the 'A' term. In this scheme, some additional logic is employed to
compute the term equivalent to multiple bits in 'B'.

Multiplication can be pipelined over multiple cycles by computing
individual terms in the operation and accumulating the result
(shifting accordingly). The accumulation step is retained in a
redundundant format and typically the final result is computed in the
final cycle.

This example demonstrates a 5 cycle 64b multiplier. 16b x 16b = 32b
elements are computed in four cycles and accumulated. The final 64b
value computed from the partially computed products.
