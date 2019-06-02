# Problem Statement

Implement a Re-Order Buffer (ROB)

# Commentary

A Re-order Buffer (ROB) is a commonly seen block used to recover order
in a stream of data when completions can arrive out of order. The
principle of operation is fairly simple: items/data is allocated in
sequential order and provided with a unique tag (an identifier). Upon
completion, the original tag is returned and any ancillary state
retained. Tags are dellocated in sequential order to recover the
original ordering.

Although the use of a ROB may be unavoidable, its introduction
introduces latency into a system because ordering is typically
employed in non-blocking memory subsystems to improve performance. The
ROB must be sized such that it may maintain at least the maximum
number of inflight transactions plus some overhead to account for the
'out-of-orderedness'.
