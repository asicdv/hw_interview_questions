# Problem Statement

Design a left-/right logical-/arthimetic- shifter with support for
SIMD.

# Commentary

Barrel shifters may be inferred using the traditional SystemVerilog
operators. In the majority of cases, synthesis tools will do a good
job of infering the correct structure. In the case of SIMD, where the
shift function is dependent upon the word width, the naive designer is
forced to infer multiple shifters of varying widths and mux the
result. Presented here is a more sophisticated design where a single
barrel shifter structure is present, but the shift fields are
appropriately qualified on the shift type (logical, arithmetic) as
well as the SIMD word width. The solution present should be more
efficient than simply relying upon the synthesis tool to infer
multiple duplicates of the same structure. The design can be readily
extended to support rotation.
