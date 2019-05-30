##========================================================================== //
## Copyright (c) 2016-2018, Stephen Henry
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

cmake_minimum_required(VERSION 3.5)
project(hw_interview_questions)

set(LIBTB2_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/contrib/libtb2/)
include(${LIBTB2_ROOT}/cmake/libtb2.cmake)

include_directories(${CMAKE_SOURCE_DIR}/contrib/libtb/src)

macro(emit_answer target answer_sv answer_cc)
  message(STATUS "Emit soluion: ${target}")
  libtb2_add_include_path(${CMAKE_CURRENT_SOURCE_DIR})
  libtb2_add_include_path(${CMAKE_CURRENT_BINARY_DIR})
  get_filename_component(answer ${answer_sv} NAME_WE)
  libtb2_verilate_sc(verilate_${target} ${answer} vlib)
  add_executable(${target} ${answer_cc})
  add_dependencies(${target} verilate_${target})
  target_include_directories(${target} PUBLIC
    ${LIBTB_ROOT}/src
    ${CMAKE_CURRENT_BINARY_DIR}
    )
  target_link_libraries(${target} tb2 systemc scv ${vlib} vcom gtest_main gtest pthread)
  add_test(NAME ${target} COMMAND ${target})
endmacro()

enable_testing()
add_subdirectory(third_party)
add_subdirectory(contrib)
add_subdirectory(design)
add_subdirectory(general)
add_subdirectory(arithmetic)
add_subdirectory(clocking)
add_subdirectory(problem_solving)
add_subdirectory(queues)
add_subdirectory(control)
add_subdirectory(memory)
add_subdirectory(pipelines)
