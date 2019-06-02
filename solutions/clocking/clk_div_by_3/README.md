# Problem

Generate a circuit that divides a clock by some odd number (in this
case 3). In particular, the duty-cycle of the clock must be 50%.

# Notes

The only way this can be done with a duty cycle of 50% is by using a
doubly edge triggered flip flop. These are uncommon, (unsupported by
FPGA flows), and should be used with caution. Leave this to the PD
people to figure out.
