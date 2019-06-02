# Problem

What is SIMD? What does it do?

# Commentary

Single Instruction, Multiple Data (SIMD) is a common architectural
technique to enable data-parallelism. Specifically, when operating on
data types smaller than the word width of the machine, it becomes
possible to compute multiple operations using smaller data types in
parallel using the same hardware in the same time. A single 32b adder
may, for example, be used to compute 4 8b additions in parallel. I
will point you to the relevant literature for further information.

During implementation, there is a specific trick used to enable
efficient logic. For a given adder, one requires the ability to split
the same adder into smaller, independent adders. One does not wish to
incur the area penalty of simply instantiating multiple smaller adders
and multiplexing between them based upon the result. Instead, a single
adder is extended and special control bits injected between the
smallest data type operation to be supported (in this case
bytes). Thus, a single 32b adder becomes 32b+4b where 4 additional
control bits inserted between each byte.

Individual bytes within the word can be joined by setting the
individual control bits. Control bits of 00b will, for example split
both bytes (where for ABb, A and B corresponding to both inputs to the
adder), where 11b will specifically inject a Carry-In to the next
byte. 10b or 01b will join bytes and enable the carry to propagate
between adjacent bytes. This can be easily derived from the known
equations for Carry Propagation and Generation in fast, lookahead
adders.

This technique may appear overly complex, however, it enables two
things: the ability to partition a single adder programmatically based
upon some opcode, and, the ability to maintain the fast lookahead
carry path through a single adder (where otherwise the adder carry
would be serialized over the width of each individual partitions, in
this case a byte).
