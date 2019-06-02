# Problem

You have seven, 1b numbers. If you have only full-adders, compute their sum in
the most efficient manner.

# Solution

This is a simple test of knowledge. A CSA can be used to effectively add each
number to produce the population count (the 3:2 compression function in a CSA
is, afterall, simply a Full Adder).

For the vector ABC_DEF_G, the CSA structure can be optimized by noting that
after compression of ABC and DEF we are left with two 2b numbers an a 1b number
(G). The final sum can be computed using G as the carry-in to full adder of the
LSB. This optimized structure requires only 4 full-adders instead of 5
full-adders using a naive CSA.
