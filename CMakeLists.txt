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

set(CMAKE_CXX_STANDARD 14)

option(OPT_TESTS_EN "Enable unit tests." ON)
option(OPT_REBUILD_LIBV "Invoke the build scripts for LIBV." OFF)
option(OPT_ENABLE_TRACE "Enable waveform tracing." ON)
option(OPT_ENABLE_SCV "Enable SystemC Verification Library." OFF)

add_subdirectory(third_party)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
include(${CMAKE_SOURCE_DIR}/cmake/hw.cmake)

if (OPT_TESTS_EN)
  enable_testing()
endif ()

find_package(Vivado)
find_package(PythonInterp)
find_package(SystemC)
find_package(Verilator)
build_verilator()

add_subdirectory(libtb)
add_subdirectory(libv)
include_directories(${CMAKE_SOURCE_DIR})

add_subdirectory(solutions)
add_subdirectory(tests)
