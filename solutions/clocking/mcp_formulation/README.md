# Problem

Communicate an arbitrary vector across clock domains using a
multicycle path.

# Commentary

Synchronization is performed across the clock bounary using a standard
synchronizer. The synchronized pulse is used to enable a register at
the receiving side. Some additional logic is required to perform a
handshake back to the sending domain.

This is a very slow means by which state can be communicated across
clock boundaries and consequently an asynchronous queue is typically
used. Care must be taken to annotate the path between the sending and
receiving flops as multi-path to avoid timing violations.
