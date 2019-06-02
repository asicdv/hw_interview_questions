# Problem

Construct a division unit that performs a constant divide by two for
any operand.

# Commentary

Division is a non-trivial algorithm in hardware. In the constant case,
it can be computed by performing a constant multiply (using a booth
multipler) against the operand the recipriocal of the 3 (in fixed
point representation). The result is then shifted according to the Q
of the divider representation.
