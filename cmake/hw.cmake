##========================================================================== //
## Copyright (c) 2016-2019, Stephen Henry
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

include(CMakeParseArguments)

macro (build_verilator)
  set(CMAKE_CXX_STANDARD 14)
  set(Verilator_SRCS
    ${Verilator_INCLUDE_DIR}/verilated.cpp
    ${Verilator_INCLUDE_DIR}/verilated_dpi.cpp
    ${Verilator_INCLUDE_DIR}/verilated_save.cpp
    ${Verilator_INCLUDE_DIR}/verilated_vcd_sc.cpp
    ${Verilator_INCLUDE_DIR}/verilated_vcd_c.cpp
    )
  set(Verilator_INCLUDE_DIR
    ${Verilator_INCLUDE_DIR}
    ${Verilator_INCLUDE_DIR}/vltstd
    )
  add_library(vcom SHARED ${Verilator_SRCS})
  target_include_directories(vcom PUBLIC
    ${Verilator_INCLUDE_DIR}
    ${SystemC_INCLUDE_DIR}
    )
  target_link_libraries(vcom ${SystemC_LIB})
  define_property(GLOBAL PROPERTY vinclude_path
    BRIEF_DOCS "Accumulated Verilator include path."
    FULL_DOCS "FOO"
    )
  define_property(GLOBAL PROPERTY verilator_opts
    BRIEF_DOCS "foo"
    FULL_DOCS "foo"
    )
endmacro ()

macro (add_verilator_include_path path)
  set_property(GLOBAL APPEND_STRING PROPERTY vinclude_path  ";-I${path}")
endmacro ()

macro (add_verilator_option opt)
  set_property(GLOBAL APPEND_STRING PROPERTY verilator_opts  ";${opt}")
endmacro ()

macro (verilate_sc target sv lib)
  get_property(vinclude_path GLOBAL PROPERTY vinclude_path)
  get_property(verilator_opts GLOBAL PROPERTY verilator_opts)

  ## Verilator requires that the SYSTEMC_INCLUDE and SYSTEMC_LIBDIR
  ## environment variables are set. As we build our own distributiono
  ## of SystemC it becomes necessary to run a seperate script to
  ## explicitly set the environment variable derived from the build
  ## system.
  ##
  add_custom_target(
    ${target}
    COMMAND ${CMAKE_COMMAND} -E env
       CXXFLAGS="-std=c++14"
       VERILATOR_EXE=${Verilator_EXE}
       SYSTEMC_INCLUDE=${SystemC_INCLUDE_DIR}
       SYSTEMC_LIBDIR=${SystemC_LIB}
       CMAKE_CURRENT_BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}
       verilator_opts="${verilator_opts}"
       vinclude_path="${vinclude_path}"
       top=${sv}
       ${CMAKE_SOURCE_DIR}/scripts/run_verilator.sh
    COMMENT "Verilating (SystemC): ${sv}"
    )
  ## Emitted verilated static library
  set(${lib} ${CMAKE_CURRENT_BINARY_DIR}/vobj/V${sv}__ALL.a)
endmacro ()

macro (emit_solution)
  cmake_parse_arguments(PARSED_ARGS
    ""
    "TARGET;VTOP"
    "TB;DEPS"
    ${ARGN}
    )
  
  message(STATUS "Emit solution: ${PARSED_ARGS_TARGET}")

  add_verilator_include_path(${CMAKE_CURRENT_SOURCE_DIR})
  add_verilator_include_path(${CMAKE_CURRENT_BINARY_DIR})
  if (${OPT_ENABLE_TRACE})
    add_verilator_option("--trace")
    add_verilator_option("--trace-structs")
  endif ()
  add_verilator_option("-Wno-lint")
  add_verilator_option("-Wno-style")

  get_filename_component(solution ${PARSED_ARGS_VTOP} NAME_WE)

  verilate_sc(verilate_${PARSED_ARGS_TARGET} ${solution} vobj)
  if (PARSED_ARGS_DEPS)
    add_dependencies(verilate_${PARSED_ARGS_TARGET} ${PARSED_ARGS_DEPS})
  endif ()
  add_executable(${PARSED_ARGS_TARGET} ${PARSED_ARGS_TB})
  add_dependencies(${PARSED_ARGS_TARGET} verilate_${PARSED_ARGS_TARGET})
  
  target_include_directories(
    ${PARSED_ARGS_TARGET} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
  target_link_libraries(${PARSED_ARGS_TARGET} systemc scv tb ${vobj} vcom)
  if (${OPT_TESTS_EN})
    target_link_libraries(${PARSED_ARGS_TARGET} gtest_main gtest pthread)
    add_test(NAME ${PARSED_ARGS_TARGET} COMMAND ${PARSED_ARGS_TARGET})
  endif ()
endmacro()
