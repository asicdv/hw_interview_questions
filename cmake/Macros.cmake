##========================================================================== //
## Copyright (c) 2016, Stephen Henry
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met:
##
## * Redistributions of source code must retain the above copyright notice, this
##   list of conditions and the following disclaimer.
##
## * Redistributions in binary form must reproduce the above copyright notice,
##   this list of conditions and the following disclaimer in the documentation
##   and/or other materials provided with the distribution.
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
## POSSIBILITY OF SUCH DAMAGE.
##========================================================================== //

MACRO(EMIT_ANSWER ANSWER)
  SET(VERILATED_OBJ "${CMAKE_CURRENT_BINARY_DIR}/obj")
  SET(VERILATED_LIB "${VERILATED_OBJ}/V${ANSWER}__ALL.a")
  SET(VERILATOR_INCLUDE
    "-I${Libv_VINCLUDE_DIRS}"
    )
  ADD_CUSTOM_TARGET(
    verilate
    COMMAND verilator --sc ${VERILATOR_INCLUDE}
        --trace --trace-structs --trace-params
        --Mdir ${VERILATED_OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/${ANSWER}.sv
    COMMAND make -C ${VERILATED_OBJ} -f "V${ANSWER}.mk"
    COMMENT "Verilating top-level: ${ANSWER}"
    )
  ADD_EXECUTABLE(${ANSWER} ${ANSWER}.cpp)
  ADD_DEPENDENCIES(${ANSWER} verilate)
  TARGET_INCLUDE_DIRECTORIES(${ANSWER} PUBLIC
    ${Verilator_INCLUDE_DIR}
    ${SystemC_INCLUDE_DIR}
    ${Libtb_INCLUDE_DIRS}
    ${VERILATED_OBJ}
    )
  TARGET_LINK_LIBRARIES(${ANSWER}
    ${SystemC_LIBRARY}
    ${VERILATED_LIB}
    verilated
    pthread
    tb
    )
ENDMACRO()