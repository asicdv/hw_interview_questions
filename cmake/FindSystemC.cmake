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

find_path(SystemC_INCLUDE_DIR systemc
  PATH_SUFFIXES include
  HINTS /opt/systemc/include
  HINTS /usr/local/systemc/include
  HINTS $ENV{HOME}/pkgs/systemc/include
  HINTS /home/stephenry/r/include
  DOC "Searching for SystemC includes."
  )

find_library(SystemC_LIB systemc
  HINTS /opt/systemc/lib
  HINTS /opt/systemc/lib-macosx64
  HINTS /usr/local/systemc/lib
  HINTS /usr/local/systemc/lib-linux64
  HINTS $ENV{HOME}/pkgs/systemc/lib-macosx64
  HINTS /home/stephenry/r/lib-linux64
  DOC "Searching for SystemC libraries."
  )

find_library(Scv_LIB scv
  HINTS /opt/systemc/lib
  HINTS /opt/systemc/lib-macosx64
  HINTS /home/stephenry/r/lib-linux64
  DOC "Searching for SystemC verification libraries."
  )

if (SystemC_LIB)
  execute_process(
    COMMAND "cat" ${SystemC_INCLUDE_DIR}/sysc/kernel/sc_ver.h
    OUTPUT_VARIABLE sc_ver)
  string(REGEX REPLACE
    ".*#define SC_VERSION_MAJOR      ([0-9]).*" "\\1" SYSTEMC_MAJOR_VERSION ${sc_ver})
  string(REGEX REPLACE
    ".*#define SC_VERSION_MINOR      ([0-9]).*" "\\1" SYSTEMC_MINOR_VERSION ${sc_ver})
  string(REGEX REPLACE
    ".*#define SC_VERSION_PATCH      ([0-9]).*" "\\1" SYSTEMC_PATCH_VERSION ${sc_ver})
  set(SYSTEMC_VERSION
    ${SYSTEMC_MAJOR_VERSION}.${SYSTEMC_MINOR_VERSION}.${SYSTEMC_PATCH_VERSION})

  add_library(systemc SHARED IMPORTED)
  set_property(TARGET systemc PROPERTY IMPORTED_LOCATION ${SystemC_LIB})

  include_directories(${SystemC_INCLUDE_DIR})

  add_library(scv SHARED IMPORTED)
  set_property(TARGET scv PROPERTY IMPORTED_LOCATION ${Scv_LIB})

  message (STATUS "Found SystemC version ${SYSTEMC_VERSION}")
  message (STATUS "SystemC include path: ${SystemC_INCLUDE_DIR}")
  message (STATUS "SystemC library path: ${SystemC_LIB}")
  if (Scv_LIB)
    message (STATUS "SCV library path: ${Scv_LIB}")    
  endif ()
else ()
  message (STATUS "SystemC not found.")
endif ()
