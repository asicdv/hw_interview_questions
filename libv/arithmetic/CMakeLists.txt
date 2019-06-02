## ==================================================================== ##
## Copyright (c) 2017, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions
## are met:
##
## * Redistributions of source code must retain the above copyright
##   notice, this list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright
##   notice, this list of conditions and the following disclaimer in
##   the documentation and/or other materials provided with the
##   distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
## "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
## LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
## FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
## COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
## INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
## (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
## SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
## HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
## STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
## OF THE POSSIBILITY OF SUCH DAMAGE.
## ==================================================================== ##

add_verilator_include_path(${CMAKE_CURRENT_SOURCE_DIR})

if (PYTHONINTERP_FOUND)
  macro(generate_brent_kung W)
    add_custom_target(
      brent_kung_${W}
      BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/carry_chain_brent_kung_${W}.sv"
      COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/scripts/generate_brent_kung.py ${W}
      COMMENT "Generating Brent/Kung carry chain W = ${W}"
      DEPENDS "${CMAKE_SOURCE_DIR}/scripts/generate_brent_kung.py"  
      )
  endmacro()

  macro(generate_csa N W)
    add_custom_target(
      csa_N${N}_${W}b
      BYPRODUCTS "${CMAKE_CURRENT_BINARY_DIR}/csa_N${N}_${W}b.sv"
      COMMAND ${PYTHON_EXECUTABLE}
        ${CMAKE_SOURCE_DIR}/scripts/generate_csa.py ${N} ${W}
      COMMENT "Generating CSA N = ${N}, W = ${W}."
      DEPENDS "${CMAKE_SOURCE_DIR}/scripts/generate_csa.py"  
      )
  endmacro()

  if (OPT_REBUILD_LIBV)
    generate_brent_kung(128)
    generate_brent_kung(64)
    generate_brent_kung(32)
    generate_brent_kung(16)

    generate_csa(9 32)
    generate_csa(3 64)
  endif ()
  
endif ()

