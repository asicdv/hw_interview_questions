// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header

#ifndef _Vsimd_shifter_tb__Syms_H_
#define _Vsimd_shifter_tb__Syms_H_

#include "verilated.h"

// INCLUDE MODULE CLASSES
#include "Vsimd_shifter_tb.h"

// SYMS CLASS
class Vsimd_shifter_tb__Syms : public VerilatedSyms {
  public:
    
    // LOCAL STATE
    const char* __Vm_namep;
    bool __Vm_activity;  ///< Used by trace routines to determine change occurred
    bool __Vm_didInit;
    
    // SUBCELL STATE
    Vsimd_shifter_tb*              TOPp;
    
    // CREATORS
    Vsimd_shifter_tb__Syms(Vsimd_shifter_tb* topp, const char* namep);
    ~Vsimd_shifter_tb__Syms() {}
    
    // METHODS
    inline const char* name() { return __Vm_namep; }
    inline bool getClearActivity() { bool r=__Vm_activity; __Vm_activity=false; return r; }
    
} VL_ATTR_ALIGNED(64);

#endif // guard
