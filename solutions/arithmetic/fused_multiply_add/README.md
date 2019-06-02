# Problem

Implemented a Fused Multiply Accumulator

# Commentary

FMA operations are used during polynominal evaluation (filtering). FMA
can be computed efficiently by considering that multiplication (using
the Booth algorithm) makes use of a Carry Save Adder. The addition and
accumulation operation can be performed by injecting the appropriate
operands into the CSA chain.
