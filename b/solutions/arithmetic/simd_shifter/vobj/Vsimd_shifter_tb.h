// Verilated -*- SystemC -*-
// DESCRIPTION: Verilator output: Primary design header
//
// This header should be included by all source files instantiating the design.
// The class here is then constructed to instantiate the design.
// See the Verilator manual for examples.

#ifndef _Vsimd_shifter_tb_H_
#define _Vsimd_shifter_tb_H_

#include "systemc.h"
#include "verilated_sc.h"
#include "verilated.h"

class Vsimd_shifter_tb__Syms;
class VerilatedVcd;

//----------

SC_MODULE(Vsimd_shifter_tb) {
  public:
    
    // PORTS
    // The application code writes and reads these signals to
    // propagate new values into/out from the Verilated model.
    // Begin mtask footprint  all: 
    sc_in<bool> clk;
    sc_in<bool> rst;
    sc_in<uint32_t> mode;
    sc_in<uint32_t> op;
    sc_in<vluint64_t> shift;
    sc_out<bool> fail;
    sc_in<vluint64_t> in;
    
    // LOCAL SIGNALS
    // Internals; generally not touched by application code
    // Begin mtask footprint  all: 
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk1__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk2__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk3__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk4__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk5__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk6__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk1__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk1__DOT__unnamedblk2__DOT__shift_idx,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk3__DOT__i,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk3__DOT__unnamedblk4__DOT__shift_idx,31,0);
    VL_SIG(simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk5__DOT__i,31,0);
    VL_SIG64(simd_shifter_tb__DOT__simd_shifter___05Fout,63,0);
    VL_SIG64(simd_shifter_tb__DOT__simd_shifter_naive___05Fout,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1,63,0);
    VL_SIG64(simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0,63,0);
    
    // LOCAL VARIABLES
    // Internals; generally not touched by application code
    // Anonymous structures to workaround compiler member-count bugs
    struct {
	// Begin mtask footprint  all: 
	VL_SIG64(__Vcellinp__simd_shifter_tb__shift,47,0);
	VL_SIG8(__Vcellinp__simd_shifter_tb__op,2,0);
	VL_SIG8(__Vcellinp__simd_shifter_tb__mode,1,0);
	VL_SIG8(__Vcellinp__simd_shifter_tb__rst,0,0);
	VL_SIG8(__Vcellinp__simd_shifter_tb__clk,0,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__1__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__3__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__5__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__7__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__9__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__11__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__13__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__15__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__17__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__19__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__21__idx,31,0);
	VL_SIG(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__23__idx,31,0);
	VL_SIG(__Vm_traceActivity,31,0);
	VL_SIG64(__Vcellinp__simd_shifter_tb__in,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__0__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__0__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__1__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__1__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__2__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__2__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__3__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__3__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__4__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__4__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__5__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__5__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__6__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__6__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__7__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__7__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__8__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__8__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__9__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__9__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__10__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__10__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__11__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__11__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__12__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__12__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__13__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__13__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__14__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__14__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__15__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__15__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__16__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__16__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__17__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__17__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__18__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__18__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__19__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__19__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__20__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__20__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__21__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__21__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__22__Vfuncout,63,0);
    };
    struct {
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shr__22__w,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__23__Vfuncout,63,0);
	VL_SIG64(__Vfunc_simd_shifter_tb__DOT__u_simd_shifter__DOT__shl__23__w,63,0);
    };
    
    // INTERNAL VARIABLES
    // Internals; generally not touched by application code
    Vsimd_shifter_tb__Syms* __VlSymsp;  // Symbol table
    
    // PARAMETERS
    // Parameters marked /*verilator public*/ for use by application code
    
    // CONSTRUCTORS
  private:
    VL_UNCOPYABLE(Vsimd_shifter_tb);  ///< Copying not allowed
  public:
    SC_CTOR(Vsimd_shifter_tb);
    virtual ~Vsimd_shifter_tb();
    /// Trace signals in the model; called by application code
    void trace(VerilatedVcdC* tfp, int levels, int options=0);
    /// SC tracing; avoid overloaded virtual function lint warning
    virtual void trace(sc_trace_file* tfp) const { ::sc_core::sc_module::trace(tfp); }
    
    // API METHODS
  private:
    void eval();
  public:
    void final();
    
    // INTERNAL METHODS
  private:
    static void _eval_initial_loop(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
  public:
    void __Vconfigure(Vsimd_shifter_tb__Syms* symsp, bool first);
  private:
    static QData _change_request(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
  public:
    static void _combo__TOP__1(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
  private:
    void _ctor_var_reset();
  public:
    static void _eval(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
  private:
#ifdef VL_DEBUG
    void _eval_debug_assertions();
#endif // VL_DEBUG
  public:
    static void _eval_initial(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
    static void _eval_settle(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
    static void _settle__TOP__2(Vsimd_shifter_tb__Syms* __restrict vlSymsp);
    static void traceChgThis(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceChgThis__2(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceFullThis(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceFullThis__1(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceInitThis(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceInitThis__1(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code);
    static void traceInit(VerilatedVcd* vcdp, void* userthis, uint32_t code);
    static void traceFull(VerilatedVcd* vcdp, void* userthis, uint32_t code);
    static void traceChg(VerilatedVcd* vcdp, void* userthis, uint32_t code);
} VL_ATTR_ALIGNED(128);

#endif // guard
