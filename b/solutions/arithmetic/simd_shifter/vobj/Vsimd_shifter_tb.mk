# Verilated -*- Makefile -*-
# DESCRIPTION: Verilator output: Makefile for building Verilated archive or executable
#
# Execute this makefile from the object directory:
#    make -f Vsimd_shifter_tb.mk

default: Vsimd_shifter_tb__ALL.a

### Constants...
# Perl executable (from $PERL)
PERL = perl
# Path to Verilator kit (from $VERILATOR_ROOT)
VERILATOR_ROOT = /tools/verilator/latest
# SystemC include directory with systemc.h (from $SYSTEMC_INCLUDE)
SYSTEMC_INCLUDE ?= /tools/systemc/latest/include
# SystemC library directory with libsystemc.a (from $SYSTEMC_LIBDIR)
SYSTEMC_LIBDIR ?= /tools/systemc/latest/lib-linux64/libsystemc.so

### Switches...
# SystemC output mode?  0/1 (from --sc)
VM_SC = 1
# Legacy or SystemC output mode?  0/1 (from --sc)
VM_SP_OR_SC = $(VM_SC)
# Deprecated
VM_PCLI = 0
# Deprecated: SystemC architecture to find link library path (from $SYSTEMC_ARCH)
VM_SC_TARGET_ARCH = linux

### Vars...
# Design prefix (from --prefix)
VM_PREFIX = Vsimd_shifter_tb
# Module prefix (from --prefix)
VM_MODPREFIX = Vsimd_shifter_tb
# User CFLAGS (from -CFLAGS on Verilator command line)
VM_USER_CFLAGS = \

# User LDLIBS (from -LDFLAGS on Verilator command line)
VM_USER_LDLIBS = \

# User .cpp files (from .cpp's on Verilator command line)
VM_USER_CLASSES = \

# User .cpp directories (from .cpp's on Verilator command line)
VM_USER_DIR = \


### Default rules...
# Include list of all generated classes
include Vsimd_shifter_tb_classes.mk
# Include global rules
include $(VERILATOR_ROOT)/include/verilated.mk

# Verilated -*- Makefile -*-
