# Problem

For a given input vector (of arbitrary length), write logic to emit
the indices of each zero in the vector (beginning at the LSB).

# Commentary

This is performed in a similar manner to the 'slow' case, except that
zero detection is performed in seperate power-of-2 bins. In this
manner, the priority logic can be performed in parallel and the bin id
computed as the AND reduction of the bin (when masked). This approach,
which is more complicated than the 'slow' version, scales to higher N.

An alternative approach to the scalability problem is to perform the
priority operation using a prefix network approach. This approach,
which is similar to that used in fast adders results in a network with
is O(lgN) vs. O(N).
