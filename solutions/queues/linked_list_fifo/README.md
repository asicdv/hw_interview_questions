# Problem

Design and implement a unit can maintains N-ordered queues as a singly
linked list.

# Commentary

Singly linked lists are commonly used in networking applications where
multiple queues must be maintained for differing and independent
contexts and the load across individual queues is not balanced for all
traffic flows. In this case, it would otherwise be necessary to size
each individual queue to the worst case, whether used or not. A
secondary option is to dynamically allocate lines to queues using a
linked list approach. The nice thing here is that a single buffer can
be very efficient allocated on-demand. A disadvantage of this scheme
however is that no longer is it possible to push/pop the queue on
back-to-back cycles. Similarly, because internal state is maintained
using a single ported SRAM it is not possible to execute push/pop
commands on the same cycle.

The performance degredation associated with such queues is mitigated
by allocating buffer space at the Flit-level opposed to the
Phit-level. This corresponds well with flow control elsewhere and is a
common technique used with this approach.
