# Problem

For a given input vector (of arbitrary length), write logic to emit
the indices of each zero in the vector (beginning at the LSB).

# Commentary

Zero detection is performed using a standard priority network. Each
time an index is emitted, a mask is constructed to negate the location
from consideration in the next round. This continues until the OR of
the original vector and the mask is all ones.

The performance of the detection logic is limited by the
prioritization logic and as N increases, so does the cycle time.
