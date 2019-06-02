# Detect Sequence

# Question

A single bit input is presented to a module. The input is synchronous
with the system clock. For a given static sequence of ones and zeros,
write logic to detect the sequence.

# Commentary

There are two standard solutions to this problem:

* Firstly, a Deterministic Finite Automaton (DFA), essentially a
  generalizstion of a standard Moore Machine, can be used to walk
  through all shifts of the input sequence and emit a strobe once the
  final bit of the sequence. This is approach is somewhat complicated
  for long sequences, as each permissible shift of the input sequence
  must be considered. This approach, however, can be easily
  generalized to a non-binary alphabet.

* Secondly, a shift register with comparision logic can be used. This
  is the easier approach, and perhaps one that is more resilient to
  bugs during coding. The area/performance trade-offs are probably not
  important, certainly not compared with the complexity of the strict
  DFA approach.
