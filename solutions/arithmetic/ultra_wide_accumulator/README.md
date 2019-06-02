# Problem Statement

Implement an ultra-wide accumulator capable of consuming one 128b
value per cycle using only adders that can computer a 32b sum per
cycle.

# Solution

Addition can be trivially pipelined by partitioning the carry chain
across multiple cycles (in doing so, the pipelined segments of the
addition must be delayed). For some pipelined adder that computes its
value after some cycles, the accumulation can be performed by
retaining at the input two registers computed from the input each
cycle, the current values of the registers and a standard CSA
update. Accumulation can therefore be performed very quickly without
incurring the cost of the full carry chain.
