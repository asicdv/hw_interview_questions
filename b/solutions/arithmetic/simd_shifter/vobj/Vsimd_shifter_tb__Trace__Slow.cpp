// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Tracing implementation internals
#include "verilated_vcd_c.h"
#include "Vsimd_shifter_tb__Syms.h"


//======================

void Vsimd_shifter_tb::trace(VerilatedVcdC* tfp, int, int) {
    tfp->spTrace()->addCallback(&Vsimd_shifter_tb::traceInit, &Vsimd_shifter_tb::traceFull, &Vsimd_shifter_tb::traceChg, this);
}
void Vsimd_shifter_tb::traceInit(VerilatedVcd* vcdp, void* userthis, uint32_t code) {
    // Callback from vcd->open()
    Vsimd_shifter_tb* t=(Vsimd_shifter_tb*)userthis;
    Vsimd_shifter_tb__Syms* __restrict vlSymsp = t->__VlSymsp;  // Setup global symbol table
    if (!Verilated::calcUnusedSigs()) {
	VL_FATAL_MT(__FILE__,__LINE__,__FILE__,"Turning on wave traces requires Verilated::traceEverOn(true) call before time 0.");
    }
    vcdp->scopeEscape(' ');
    t->traceInitThis(vlSymsp, vcdp, code);
    vcdp->scopeEscape('.');
}
void Vsimd_shifter_tb::traceFull(VerilatedVcd* vcdp, void* userthis, uint32_t code) {
    // Callback from vcd->dump()
    Vsimd_shifter_tb* t=(Vsimd_shifter_tb*)userthis;
    Vsimd_shifter_tb__Syms* __restrict vlSymsp = t->__VlSymsp;  // Setup global symbol table
    t->traceFullThis(vlSymsp, vcdp, code);
}

//======================


void Vsimd_shifter_tb::traceInitThis(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vsimd_shifter_tb* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c=code;
    if (0 && vcdp && c) {}  // Prevent unused
    vcdp->module(vlSymsp->name());  // Setup signal names
    // Body
    {
	vlTOPp->traceInitThis__1(vlSymsp, vcdp, code);
    }
}

void Vsimd_shifter_tb::traceFullThis(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vsimd_shifter_tb* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c=code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
	vlTOPp->traceFullThis__1(vlSymsp, vcdp, code);
    }
    // Final
    vlTOPp->__Vm_traceActivity = 0U;
}

void Vsimd_shifter_tb::traceInitThis__1(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vsimd_shifter_tb* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c=code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
	// Tracing: clk // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:32
	// Tracing: rst // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:33
	// Tracing: in // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:35
	// Tracing: mode // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:36
	// Tracing: op // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:37
	// Tracing: shift // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:38
	// Tracing: fail // Ignored: Verilator trace_off at /home/stephenry/github.com/hw_interview_questions/solutions/arithmetic/simd_shifter/simd_shifter_tb.sv:40
	vcdp->declBit(c+1,"simd_shifter_tb clk",-1);
	vcdp->declBit(c+2,"simd_shifter_tb rst",-1);
	vcdp->declQuad(c+3,"simd_shifter_tb in",-1,63,0);
	vcdp->declBus(c+5,"simd_shifter_tb mode",-1,1,0);
	vcdp->declBus(c+6,"simd_shifter_tb op",-1,2,0);
	vcdp->declBus(c+7,"simd_shifter_tb shift(0)",-1,5,0);
	vcdp->declBus(c+8,"simd_shifter_tb shift(1)",-1,5,0);
	vcdp->declBus(c+9,"simd_shifter_tb shift(2)",-1,5,0);
	vcdp->declBus(c+10,"simd_shifter_tb shift(3)",-1,5,0);
	vcdp->declBus(c+11,"simd_shifter_tb shift(4)",-1,5,0);
	vcdp->declBus(c+12,"simd_shifter_tb shift(5)",-1,5,0);
	vcdp->declBus(c+13,"simd_shifter_tb shift(6)",-1,5,0);
	vcdp->declBus(c+14,"simd_shifter_tb shift(7)",-1,5,0);
	vcdp->declBit(c+15,"simd_shifter_tb fail",-1);
	vcdp->declQuad(c+16,"simd_shifter_tb simd_shifter__out w64",-1,63,0);
	vcdp->declBus(c+18,"simd_shifter_tb simd_shifter__out w32(0)",-1,31,0);
	vcdp->declBus(c+19,"simd_shifter_tb simd_shifter__out w32(1)",-1,31,0);
	vcdp->declBus(c+20,"simd_shifter_tb simd_shifter__out w16(0)",-1,15,0);
	vcdp->declBus(c+21,"simd_shifter_tb simd_shifter__out w16(1)",-1,15,0);
	vcdp->declBus(c+22,"simd_shifter_tb simd_shifter__out w16(2)",-1,15,0);
	vcdp->declBus(c+23,"simd_shifter_tb simd_shifter__out w16(3)",-1,15,0);
	vcdp->declBus(c+24,"simd_shifter_tb simd_shifter__out w8(0)",-1,7,0);
	vcdp->declBus(c+25,"simd_shifter_tb simd_shifter__out w8(1)",-1,7,0);
	vcdp->declBus(c+26,"simd_shifter_tb simd_shifter__out w8(2)",-1,7,0);
	vcdp->declBus(c+27,"simd_shifter_tb simd_shifter__out w8(3)",-1,7,0);
	vcdp->declBus(c+28,"simd_shifter_tb simd_shifter__out w8(4)",-1,7,0);
	vcdp->declBus(c+29,"simd_shifter_tb simd_shifter__out w8(5)",-1,7,0);
	vcdp->declBus(c+30,"simd_shifter_tb simd_shifter__out w8(6)",-1,7,0);
	vcdp->declBus(c+31,"simd_shifter_tb simd_shifter__out w8(7)",-1,7,0);
	vcdp->declQuad(c+32,"simd_shifter_tb simd_shifter_naive__out w64",-1,63,0);
	vcdp->declBus(c+34,"simd_shifter_tb simd_shifter_naive__out w32(0)",-1,31,0);
	vcdp->declBus(c+35,"simd_shifter_tb simd_shifter_naive__out w32(1)",-1,31,0);
	vcdp->declBus(c+36,"simd_shifter_tb simd_shifter_naive__out w16(0)",-1,15,0);
	vcdp->declBus(c+37,"simd_shifter_tb simd_shifter_naive__out w16(1)",-1,15,0);
	vcdp->declBus(c+38,"simd_shifter_tb simd_shifter_naive__out w16(2)",-1,15,0);
	vcdp->declBus(c+39,"simd_shifter_tb simd_shifter_naive__out w16(3)",-1,15,0);
	vcdp->declBus(c+40,"simd_shifter_tb simd_shifter_naive__out w8(0)",-1,7,0);
	vcdp->declBus(c+41,"simd_shifter_tb simd_shifter_naive__out w8(1)",-1,7,0);
	vcdp->declBus(c+42,"simd_shifter_tb simd_shifter_naive__out w8(2)",-1,7,0);
	vcdp->declBus(c+43,"simd_shifter_tb simd_shifter_naive__out w8(3)",-1,7,0);
	vcdp->declBus(c+44,"simd_shifter_tb simd_shifter_naive__out w8(4)",-1,7,0);
	vcdp->declBus(c+45,"simd_shifter_tb simd_shifter_naive__out w8(5)",-1,7,0);
	vcdp->declBus(c+46,"simd_shifter_tb simd_shifter_naive__out w8(6)",-1,7,0);
	vcdp->declBus(c+47,"simd_shifter_tb simd_shifter_naive__out w8(7)",-1,7,0);
	vcdp->declQuad(c+3,"simd_shifter_tb u_simd_shifter in w64",-1,63,0);
	vcdp->declBus(c+48,"simd_shifter_tb u_simd_shifter in w32(0)",-1,31,0);
	vcdp->declBus(c+49,"simd_shifter_tb u_simd_shifter in w32(1)",-1,31,0);
	vcdp->declBus(c+50,"simd_shifter_tb u_simd_shifter in w16(0)",-1,15,0);
	vcdp->declBus(c+51,"simd_shifter_tb u_simd_shifter in w16(1)",-1,15,0);
	vcdp->declBus(c+52,"simd_shifter_tb u_simd_shifter in w16(2)",-1,15,0);
	vcdp->declBus(c+53,"simd_shifter_tb u_simd_shifter in w16(3)",-1,15,0);
	vcdp->declBus(c+54,"simd_shifter_tb u_simd_shifter in w8(0)",-1,7,0);
	vcdp->declBus(c+55,"simd_shifter_tb u_simd_shifter in w8(1)",-1,7,0);
	vcdp->declBus(c+56,"simd_shifter_tb u_simd_shifter in w8(2)",-1,7,0);
	vcdp->declBus(c+57,"simd_shifter_tb u_simd_shifter in w8(3)",-1,7,0);
	vcdp->declBus(c+58,"simd_shifter_tb u_simd_shifter in w8(4)",-1,7,0);
	vcdp->declBus(c+59,"simd_shifter_tb u_simd_shifter in w8(5)",-1,7,0);
	vcdp->declBus(c+60,"simd_shifter_tb u_simd_shifter in w8(6)",-1,7,0);
	vcdp->declBus(c+61,"simd_shifter_tb u_simd_shifter in w8(7)",-1,7,0);
	vcdp->declBus(c+5,"simd_shifter_tb u_simd_shifter mode",-1,1,0);
	vcdp->declBus(c+6,"simd_shifter_tb u_simd_shifter op",-1,2,0);
	vcdp->declBus(c+7,"simd_shifter_tb u_simd_shifter shift(0)",-1,5,0);
	vcdp->declBus(c+8,"simd_shifter_tb u_simd_shifter shift(1)",-1,5,0);
	vcdp->declBus(c+9,"simd_shifter_tb u_simd_shifter shift(2)",-1,5,0);
	vcdp->declBus(c+10,"simd_shifter_tb u_simd_shifter shift(3)",-1,5,0);
	vcdp->declQuad(c+16,"simd_shifter_tb u_simd_shifter out w64",-1,63,0);
	vcdp->declBus(c+18,"simd_shifter_tb u_simd_shifter out w32(0)",-1,31,0);
	vcdp->declBus(c+19,"simd_shifter_tb u_simd_shifter out w32(1)",-1,31,0);
	vcdp->declBus(c+20,"simd_shifter_tb u_simd_shifter out w16(0)",-1,15,0);
	vcdp->declBus(c+21,"simd_shifter_tb u_simd_shifter out w16(1)",-1,15,0);
	vcdp->declBus(c+22,"simd_shifter_tb u_simd_shifter out w16(2)",-1,15,0);
	vcdp->declBus(c+23,"simd_shifter_tb u_simd_shifter out w16(3)",-1,15,0);
	vcdp->declBus(c+24,"simd_shifter_tb u_simd_shifter out w8(0)",-1,7,0);
	vcdp->declBus(c+25,"simd_shifter_tb u_simd_shifter out w8(1)",-1,7,0);
	vcdp->declBus(c+26,"simd_shifter_tb u_simd_shifter out w8(2)",-1,7,0);
	vcdp->declBus(c+27,"simd_shifter_tb u_simd_shifter out w8(3)",-1,7,0);
	vcdp->declBus(c+28,"simd_shifter_tb u_simd_shifter out w8(4)",-1,7,0);
	vcdp->declBus(c+29,"simd_shifter_tb u_simd_shifter out w8(5)",-1,7,0);
	vcdp->declBus(c+30,"simd_shifter_tb u_simd_shifter out w8(6)",-1,7,0);
	vcdp->declBus(c+31,"simd_shifter_tb u_simd_shifter out w8(7)",-1,7,0);
	vcdp->declQuad(c+62,"simd_shifter_tb u_simd_shifter word_rnd_6 w64",-1,63,0);
	vcdp->declBus(c+64,"simd_shifter_tb u_simd_shifter word_rnd_6 w32(0)",-1,31,0);
	vcdp->declBus(c+65,"simd_shifter_tb u_simd_shifter word_rnd_6 w32(1)",-1,31,0);
	vcdp->declBus(c+66,"simd_shifter_tb u_simd_shifter word_rnd_6 w16(0)",-1,15,0);
	vcdp->declBus(c+67,"simd_shifter_tb u_simd_shifter word_rnd_6 w16(1)",-1,15,0);
	vcdp->declBus(c+68,"simd_shifter_tb u_simd_shifter word_rnd_6 w16(2)",-1,15,0);
	vcdp->declBus(c+69,"simd_shifter_tb u_simd_shifter word_rnd_6 w16(3)",-1,15,0);
	vcdp->declBus(c+70,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(0)",-1,7,0);
	vcdp->declBus(c+71,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(1)",-1,7,0);
	vcdp->declBus(c+72,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(2)",-1,7,0);
	vcdp->declBus(c+73,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(3)",-1,7,0);
	vcdp->declBus(c+74,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(4)",-1,7,0);
	vcdp->declBus(c+75,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(5)",-1,7,0);
	vcdp->declBus(c+76,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(6)",-1,7,0);
	vcdp->declBus(c+77,"simd_shifter_tb u_simd_shifter word_rnd_6 w8(7)",-1,7,0);
	vcdp->declQuad(c+78,"simd_shifter_tb u_simd_shifter word_rnd_5 w64",-1,63,0);
	vcdp->declBus(c+80,"simd_shifter_tb u_simd_shifter word_rnd_5 w32(0)",-1,31,0);
	vcdp->declBus(c+81,"simd_shifter_tb u_simd_shifter word_rnd_5 w32(1)",-1,31,0);
	vcdp->declBus(c+82,"simd_shifter_tb u_simd_shifter word_rnd_5 w16(0)",-1,15,0);
	vcdp->declBus(c+83,"simd_shifter_tb u_simd_shifter word_rnd_5 w16(1)",-1,15,0);
	vcdp->declBus(c+84,"simd_shifter_tb u_simd_shifter word_rnd_5 w16(2)",-1,15,0);
	vcdp->declBus(c+85,"simd_shifter_tb u_simd_shifter word_rnd_5 w16(3)",-1,15,0);
	vcdp->declBus(c+86,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(0)",-1,7,0);
	vcdp->declBus(c+87,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(1)",-1,7,0);
	vcdp->declBus(c+88,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(2)",-1,7,0);
	vcdp->declBus(c+89,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(3)",-1,7,0);
	vcdp->declBus(c+90,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(4)",-1,7,0);
	vcdp->declBus(c+91,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(5)",-1,7,0);
	vcdp->declBus(c+92,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(6)",-1,7,0);
	vcdp->declBus(c+93,"simd_shifter_tb u_simd_shifter word_rnd_5 w8(7)",-1,7,0);
	vcdp->declQuad(c+94,"simd_shifter_tb u_simd_shifter word_rnd_4 w64",-1,63,0);
	vcdp->declBus(c+96,"simd_shifter_tb u_simd_shifter word_rnd_4 w32(0)",-1,31,0);
	vcdp->declBus(c+97,"simd_shifter_tb u_simd_shifter word_rnd_4 w32(1)",-1,31,0);
	vcdp->declBus(c+98,"simd_shifter_tb u_simd_shifter word_rnd_4 w16(0)",-1,15,0);
	vcdp->declBus(c+99,"simd_shifter_tb u_simd_shifter word_rnd_4 w16(1)",-1,15,0);
	vcdp->declBus(c+100,"simd_shifter_tb u_simd_shifter word_rnd_4 w16(2)",-1,15,0);
	vcdp->declBus(c+101,"simd_shifter_tb u_simd_shifter word_rnd_4 w16(3)",-1,15,0);
	vcdp->declBus(c+102,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(0)",-1,7,0);
	vcdp->declBus(c+103,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(1)",-1,7,0);
	vcdp->declBus(c+104,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(2)",-1,7,0);
	vcdp->declBus(c+105,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(3)",-1,7,0);
	vcdp->declBus(c+106,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(4)",-1,7,0);
	vcdp->declBus(c+107,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(5)",-1,7,0);
	vcdp->declBus(c+108,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(6)",-1,7,0);
	vcdp->declBus(c+109,"simd_shifter_tb u_simd_shifter word_rnd_4 w8(7)",-1,7,0);
	vcdp->declQuad(c+110,"simd_shifter_tb u_simd_shifter word_rnd_3 w64",-1,63,0);
	vcdp->declBus(c+112,"simd_shifter_tb u_simd_shifter word_rnd_3 w32(0)",-1,31,0);
	vcdp->declBus(c+113,"simd_shifter_tb u_simd_shifter word_rnd_3 w32(1)",-1,31,0);
	vcdp->declBus(c+114,"simd_shifter_tb u_simd_shifter word_rnd_3 w16(0)",-1,15,0);
	vcdp->declBus(c+115,"simd_shifter_tb u_simd_shifter word_rnd_3 w16(1)",-1,15,0);
	vcdp->declBus(c+116,"simd_shifter_tb u_simd_shifter word_rnd_3 w16(2)",-1,15,0);
	vcdp->declBus(c+117,"simd_shifter_tb u_simd_shifter word_rnd_3 w16(3)",-1,15,0);
	vcdp->declBus(c+118,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(0)",-1,7,0);
	vcdp->declBus(c+119,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(1)",-1,7,0);
	vcdp->declBus(c+120,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(2)",-1,7,0);
	vcdp->declBus(c+121,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(3)",-1,7,0);
	vcdp->declBus(c+122,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(4)",-1,7,0);
	vcdp->declBus(c+123,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(5)",-1,7,0);
	vcdp->declBus(c+124,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(6)",-1,7,0);
	vcdp->declBus(c+125,"simd_shifter_tb u_simd_shifter word_rnd_3 w8(7)",-1,7,0);
	vcdp->declQuad(c+126,"simd_shifter_tb u_simd_shifter word_rnd_2 w64",-1,63,0);
	vcdp->declBus(c+128,"simd_shifter_tb u_simd_shifter word_rnd_2 w32(0)",-1,31,0);
	vcdp->declBus(c+129,"simd_shifter_tb u_simd_shifter word_rnd_2 w32(1)",-1,31,0);
	vcdp->declBus(c+130,"simd_shifter_tb u_simd_shifter word_rnd_2 w16(0)",-1,15,0);
	vcdp->declBus(c+131,"simd_shifter_tb u_simd_shifter word_rnd_2 w16(1)",-1,15,0);
	vcdp->declBus(c+132,"simd_shifter_tb u_simd_shifter word_rnd_2 w16(2)",-1,15,0);
	vcdp->declBus(c+133,"simd_shifter_tb u_simd_shifter word_rnd_2 w16(3)",-1,15,0);
	vcdp->declBus(c+134,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(0)",-1,7,0);
	vcdp->declBus(c+135,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(1)",-1,7,0);
	vcdp->declBus(c+136,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(2)",-1,7,0);
	vcdp->declBus(c+137,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(3)",-1,7,0);
	vcdp->declBus(c+138,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(4)",-1,7,0);
	vcdp->declBus(c+139,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(5)",-1,7,0);
	vcdp->declBus(c+140,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(6)",-1,7,0);
	vcdp->declBus(c+141,"simd_shifter_tb u_simd_shifter word_rnd_2 w8(7)",-1,7,0);
	vcdp->declQuad(c+142,"simd_shifter_tb u_simd_shifter word_rnd_1 w64",-1,63,0);
	vcdp->declBus(c+144,"simd_shifter_tb u_simd_shifter word_rnd_1 w32(0)",-1,31,0);
	vcdp->declBus(c+145,"simd_shifter_tb u_simd_shifter word_rnd_1 w32(1)",-1,31,0);
	vcdp->declBus(c+146,"simd_shifter_tb u_simd_shifter word_rnd_1 w16(0)",-1,15,0);
	vcdp->declBus(c+147,"simd_shifter_tb u_simd_shifter word_rnd_1 w16(1)",-1,15,0);
	vcdp->declBus(c+148,"simd_shifter_tb u_simd_shifter word_rnd_1 w16(2)",-1,15,0);
	vcdp->declBus(c+149,"simd_shifter_tb u_simd_shifter word_rnd_1 w16(3)",-1,15,0);
	vcdp->declBus(c+150,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(0)",-1,7,0);
	vcdp->declBus(c+151,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(1)",-1,7,0);
	vcdp->declBus(c+152,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(2)",-1,7,0);
	vcdp->declBus(c+153,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(3)",-1,7,0);
	vcdp->declBus(c+154,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(4)",-1,7,0);
	vcdp->declBus(c+155,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(5)",-1,7,0);
	vcdp->declBus(c+156,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(6)",-1,7,0);
	vcdp->declBus(c+157,"simd_shifter_tb u_simd_shifter word_rnd_1 w8(7)",-1,7,0);
	vcdp->declQuad(c+158,"simd_shifter_tb u_simd_shifter word_rnd_0 w64",-1,63,0);
	vcdp->declBus(c+160,"simd_shifter_tb u_simd_shifter word_rnd_0 w32(0)",-1,31,0);
	vcdp->declBus(c+161,"simd_shifter_tb u_simd_shifter word_rnd_0 w32(1)",-1,31,0);
	vcdp->declBus(c+162,"simd_shifter_tb u_simd_shifter word_rnd_0 w16(0)",-1,15,0);
	vcdp->declBus(c+163,"simd_shifter_tb u_simd_shifter word_rnd_0 w16(1)",-1,15,0);
	vcdp->declBus(c+164,"simd_shifter_tb u_simd_shifter word_rnd_0 w16(2)",-1,15,0);
	vcdp->declBus(c+165,"simd_shifter_tb u_simd_shifter word_rnd_0 w16(3)",-1,15,0);
	vcdp->declBus(c+166,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(0)",-1,7,0);
	vcdp->declBus(c+167,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(1)",-1,7,0);
	vcdp->declBus(c+168,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(2)",-1,7,0);
	vcdp->declBus(c+169,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(3)",-1,7,0);
	vcdp->declBus(c+170,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(4)",-1,7,0);
	vcdp->declBus(c+171,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(5)",-1,7,0);
	vcdp->declBus(c+172,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(6)",-1,7,0);
	vcdp->declBus(c+173,"simd_shifter_tb u_simd_shifter word_rnd_0 w8(7)",-1,7,0);
	vcdp->declBus(c+174,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk1 i",-1,31,0);
	vcdp->declBus(c+175,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk2 i",-1,31,0);
	vcdp->declBus(c+176,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk3 i",-1,31,0);
	vcdp->declBus(c+177,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk4 i",-1,31,0);
	vcdp->declBus(c+178,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk5 i",-1,31,0);
	vcdp->declBus(c+179,"simd_shifter_tb u_simd_shifter shifter_PROC unnamedblk6 i",-1,31,0);
	vcdp->declQuad(c+3,"simd_shifter_tb u_simd_shifter_naive in w64",-1,63,0);
	vcdp->declBus(c+48,"simd_shifter_tb u_simd_shifter_naive in w32(0)",-1,31,0);
	vcdp->declBus(c+49,"simd_shifter_tb u_simd_shifter_naive in w32(1)",-1,31,0);
	vcdp->declBus(c+50,"simd_shifter_tb u_simd_shifter_naive in w16(0)",-1,15,0);
	vcdp->declBus(c+51,"simd_shifter_tb u_simd_shifter_naive in w16(1)",-1,15,0);
	vcdp->declBus(c+52,"simd_shifter_tb u_simd_shifter_naive in w16(2)",-1,15,0);
	vcdp->declBus(c+53,"simd_shifter_tb u_simd_shifter_naive in w16(3)",-1,15,0);
	vcdp->declBus(c+54,"simd_shifter_tb u_simd_shifter_naive in w8(0)",-1,7,0);
	vcdp->declBus(c+55,"simd_shifter_tb u_simd_shifter_naive in w8(1)",-1,7,0);
	vcdp->declBus(c+56,"simd_shifter_tb u_simd_shifter_naive in w8(2)",-1,7,0);
	vcdp->declBus(c+57,"simd_shifter_tb u_simd_shifter_naive in w8(3)",-1,7,0);
	vcdp->declBus(c+58,"simd_shifter_tb u_simd_shifter_naive in w8(4)",-1,7,0);
	vcdp->declBus(c+59,"simd_shifter_tb u_simd_shifter_naive in w8(5)",-1,7,0);
	vcdp->declBus(c+60,"simd_shifter_tb u_simd_shifter_naive in w8(6)",-1,7,0);
	vcdp->declBus(c+61,"simd_shifter_tb u_simd_shifter_naive in w8(7)",-1,7,0);
	vcdp->declBus(c+5,"simd_shifter_tb u_simd_shifter_naive mode",-1,1,0);
	vcdp->declBus(c+6,"simd_shifter_tb u_simd_shifter_naive op",-1,2,0);
	vcdp->declBus(c+7,"simd_shifter_tb u_simd_shifter_naive shift(0)",-1,5,0);
	vcdp->declBus(c+8,"simd_shifter_tb u_simd_shifter_naive shift(1)",-1,5,0);
	vcdp->declBus(c+9,"simd_shifter_tb u_simd_shifter_naive shift(2)",-1,5,0);
	vcdp->declBus(c+10,"simd_shifter_tb u_simd_shifter_naive shift(3)",-1,5,0);
	vcdp->declQuad(c+32,"simd_shifter_tb u_simd_shifter_naive out w64",-1,63,0);
	vcdp->declBus(c+34,"simd_shifter_tb u_simd_shifter_naive out w32(0)",-1,31,0);
	vcdp->declBus(c+35,"simd_shifter_tb u_simd_shifter_naive out w32(1)",-1,31,0);
	vcdp->declBus(c+36,"simd_shifter_tb u_simd_shifter_naive out w16(0)",-1,15,0);
	vcdp->declBus(c+37,"simd_shifter_tb u_simd_shifter_naive out w16(1)",-1,15,0);
	vcdp->declBus(c+38,"simd_shifter_tb u_simd_shifter_naive out w16(2)",-1,15,0);
	vcdp->declBus(c+39,"simd_shifter_tb u_simd_shifter_naive out w16(3)",-1,15,0);
	vcdp->declBus(c+40,"simd_shifter_tb u_simd_shifter_naive out w8(0)",-1,7,0);
	vcdp->declBus(c+41,"simd_shifter_tb u_simd_shifter_naive out w8(1)",-1,7,0);
	vcdp->declBus(c+42,"simd_shifter_tb u_simd_shifter_naive out w8(2)",-1,7,0);
	vcdp->declBus(c+43,"simd_shifter_tb u_simd_shifter_naive out w8(3)",-1,7,0);
	vcdp->declBus(c+44,"simd_shifter_tb u_simd_shifter_naive out w8(4)",-1,7,0);
	vcdp->declBus(c+45,"simd_shifter_tb u_simd_shifter_naive out w8(5)",-1,7,0);
	vcdp->declBus(c+46,"simd_shifter_tb u_simd_shifter_naive out w8(6)",-1,7,0);
	vcdp->declBus(c+47,"simd_shifter_tb u_simd_shifter_naive out w8(7)",-1,7,0);
	vcdp->declBus(c+180,"simd_shifter_tb u_simd_shifter_naive shift_naive_PROC unnamedblk1 i",-1,31,0);
	vcdp->declBus(c+181,"simd_shifter_tb u_simd_shifter_naive shift_naive_PROC unnamedblk1 unnamedblk2 shift_idx",-1,31,0);
	vcdp->declBus(c+182,"simd_shifter_tb u_simd_shifter_naive shift_naive_PROC unnamedblk3 i",-1,31,0);
	vcdp->declBus(c+183,"simd_shifter_tb u_simd_shifter_naive shift_naive_PROC unnamedblk3 unnamedblk4 shift_idx",-1,31,0);
	vcdp->declBus(c+184,"simd_shifter_tb u_simd_shifter_naive shift_naive_PROC unnamedblk5 i",-1,31,0);
    }
}

void Vsimd_shifter_tb::traceFullThis__1(Vsimd_shifter_tb__Syms* __restrict vlSymsp, VerilatedVcd* vcdp, uint32_t code) {
    Vsimd_shifter_tb* __restrict vlTOPp VL_ATTR_UNUSED = vlSymsp->TOPp;
    int c=code;
    if (0 && vcdp && c) {}  // Prevent unused
    // Body
    {
	vcdp->fullBit(c+1,(vlTOPp->__Vcellinp__simd_shifter_tb__clk));
	vcdp->fullBit(c+2,(vlTOPp->__Vcellinp__simd_shifter_tb__rst));
	vcdp->fullQuad(c+3,(vlTOPp->__Vcellinp__simd_shifter_tb__in),64);
	vcdp->fullBus(c+5,(vlTOPp->__Vcellinp__simd_shifter_tb__mode),2);
	vcdp->fullBus(c+6,(vlTOPp->__Vcellinp__simd_shifter_tb__op),3);
	vcdp->fullBus(c+7,((0x3fU & (IData)(vlTOPp->__Vcellinp__simd_shifter_tb__shift))),6);
	vcdp->fullBus(c+8,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					     >> 6U)))),6);
	vcdp->fullBus(c+9,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					     >> 0xcU)))),6);
	vcdp->fullBus(c+10,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					      >> 0x12U)))),6);
	vcdp->fullBus(c+11,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					      >> 0x18U)))),6);
	vcdp->fullBus(c+12,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					      >> 0x1eU)))),6);
	vcdp->fullBus(c+13,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					      >> 0x24U)))),6);
	vcdp->fullBus(c+14,((0x3fU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__shift 
					      >> 0x2aU)))),6);
	vcdp->fullBit(c+15,((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
			     != vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout)));
	vcdp->fullQuad(c+16,(vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout),64);
	vcdp->fullBus(c+18,((IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout)),32);
	vcdp->fullBus(c+19,((IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
				     >> 0x20U))),32);
	vcdp->fullBus(c+20,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout))),16);
	vcdp->fullBus(c+21,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
						>> 0x10U)))),16);
	vcdp->fullBus(c+22,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
						>> 0x20U)))),16);
	vcdp->fullBus(c+23,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
						>> 0x30U)))),16);
	vcdp->fullBus(c+24,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout))),8);
	vcdp->fullBus(c+25,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 8U)))),8);
	vcdp->fullBus(c+26,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x10U)))),8);
	vcdp->fullBus(c+27,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x18U)))),8);
	vcdp->fullBus(c+28,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x20U)))),8);
	vcdp->fullBus(c+29,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x28U)))),8);
	vcdp->fullBus(c+30,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x30U)))),8);
	vcdp->fullBus(c+31,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter___05Fout 
					      >> 0x38U)))),8);
	vcdp->fullQuad(c+32,(vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout),64);
	vcdp->fullBus(c+34,((IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout)),32);
	vcdp->fullBus(c+35,((IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
				     >> 0x20U))),32);
	vcdp->fullBus(c+36,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout))),16);
	vcdp->fullBus(c+37,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
						>> 0x10U)))),16);
	vcdp->fullBus(c+38,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
						>> 0x20U)))),16);
	vcdp->fullBus(c+39,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
						>> 0x30U)))),16);
	vcdp->fullBus(c+40,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout))),8);
	vcdp->fullBus(c+41,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 8U)))),8);
	vcdp->fullBus(c+42,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x10U)))),8);
	vcdp->fullBus(c+43,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x18U)))),8);
	vcdp->fullBus(c+44,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x20U)))),8);
	vcdp->fullBus(c+45,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x28U)))),8);
	vcdp->fullBus(c+46,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x30U)))),8);
	vcdp->fullBus(c+47,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__simd_shifter_naive___05Fout 
					      >> 0x38U)))),8);
	vcdp->fullBus(c+48,((IData)(vlTOPp->__Vcellinp__simd_shifter_tb__in)),32);
	vcdp->fullBus(c+49,((IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
				     >> 0x20U))),32);
	vcdp->fullBus(c+50,((0xffffU & (IData)(vlTOPp->__Vcellinp__simd_shifter_tb__in))),16);
	vcdp->fullBus(c+51,((0xffffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
						>> 0x10U)))),16);
	vcdp->fullBus(c+52,((0xffffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
						>> 0x20U)))),16);
	vcdp->fullBus(c+53,((0xffffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
						>> 0x30U)))),16);
	vcdp->fullBus(c+54,((0xffU & (IData)(vlTOPp->__Vcellinp__simd_shifter_tb__in))),8);
	vcdp->fullBus(c+55,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 8U)))),8);
	vcdp->fullBus(c+56,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x10U)))),8);
	vcdp->fullBus(c+57,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x18U)))),8);
	vcdp->fullBus(c+58,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x20U)))),8);
	vcdp->fullBus(c+59,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x28U)))),8);
	vcdp->fullBus(c+60,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x30U)))),8);
	vcdp->fullBus(c+61,((0xffU & (IData)((vlTOPp->__Vcellinp__simd_shifter_tb__in 
					      >> 0x38U)))),8);
	vcdp->fullQuad(c+62,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6),64);
	vcdp->fullBus(c+64,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6)),32);
	vcdp->fullBus(c+65,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
				     >> 0x20U))),32);
	vcdp->fullBus(c+66,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6))),16);
	vcdp->fullBus(c+67,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
						>> 0x10U)))),16);
	vcdp->fullBus(c+68,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
						>> 0x20U)))),16);
	vcdp->fullBus(c+69,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
						>> 0x30U)))),16);
	vcdp->fullBus(c+70,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6))),8);
	vcdp->fullBus(c+71,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 8U)))),8);
	vcdp->fullBus(c+72,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x10U)))),8);
	vcdp->fullBus(c+73,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x18U)))),8);
	vcdp->fullBus(c+74,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x20U)))),8);
	vcdp->fullBus(c+75,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x28U)))),8);
	vcdp->fullBus(c+76,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x30U)))),8);
	vcdp->fullBus(c+77,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_6 
					      >> 0x38U)))),8);
	vcdp->fullQuad(c+78,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5),64);
	vcdp->fullBus(c+80,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5)),32);
	vcdp->fullBus(c+81,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
				     >> 0x20U))),32);
	vcdp->fullBus(c+82,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5))),16);
	vcdp->fullBus(c+83,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
						>> 0x10U)))),16);
	vcdp->fullBus(c+84,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
						>> 0x20U)))),16);
	vcdp->fullBus(c+85,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
						>> 0x30U)))),16);
	vcdp->fullBus(c+86,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5))),8);
	vcdp->fullBus(c+87,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 8U)))),8);
	vcdp->fullBus(c+88,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x10U)))),8);
	vcdp->fullBus(c+89,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x18U)))),8);
	vcdp->fullBus(c+90,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x20U)))),8);
	vcdp->fullBus(c+91,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x28U)))),8);
	vcdp->fullBus(c+92,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x30U)))),8);
	vcdp->fullBus(c+93,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_5 
					      >> 0x38U)))),8);
	vcdp->fullQuad(c+94,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4),64);
	vcdp->fullBus(c+96,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4)),32);
	vcdp->fullBus(c+97,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
				     >> 0x20U))),32);
	vcdp->fullBus(c+98,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4))),16);
	vcdp->fullBus(c+99,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
						>> 0x10U)))),16);
	vcdp->fullBus(c+100,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
						 >> 0x20U)))),16);
	vcdp->fullBus(c+101,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
						 >> 0x30U)))),16);
	vcdp->fullBus(c+102,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4))),8);
	vcdp->fullBus(c+103,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 8U)))),8);
	vcdp->fullBus(c+104,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x10U)))),8);
	vcdp->fullBus(c+105,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x18U)))),8);
	vcdp->fullBus(c+106,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x20U)))),8);
	vcdp->fullBus(c+107,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x28U)))),8);
	vcdp->fullBus(c+108,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x30U)))),8);
	vcdp->fullBus(c+109,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_4 
					       >> 0x38U)))),8);
	vcdp->fullQuad(c+110,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3),64);
	vcdp->fullBus(c+112,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3)),32);
	vcdp->fullBus(c+113,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
				      >> 0x20U))),32);
	vcdp->fullBus(c+114,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3))),16);
	vcdp->fullBus(c+115,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
						 >> 0x10U)))),16);
	vcdp->fullBus(c+116,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
						 >> 0x20U)))),16);
	vcdp->fullBus(c+117,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
						 >> 0x30U)))),16);
	vcdp->fullBus(c+118,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3))),8);
	vcdp->fullBus(c+119,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 8U)))),8);
	vcdp->fullBus(c+120,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x10U)))),8);
	vcdp->fullBus(c+121,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x18U)))),8);
	vcdp->fullBus(c+122,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x20U)))),8);
	vcdp->fullBus(c+123,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x28U)))),8);
	vcdp->fullBus(c+124,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x30U)))),8);
	vcdp->fullBus(c+125,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_3 
					       >> 0x38U)))),8);
	vcdp->fullQuad(c+126,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2),64);
	vcdp->fullBus(c+128,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2)),32);
	vcdp->fullBus(c+129,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
				      >> 0x20U))),32);
	vcdp->fullBus(c+130,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2))),16);
	vcdp->fullBus(c+131,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
						 >> 0x10U)))),16);
	vcdp->fullBus(c+132,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
						 >> 0x20U)))),16);
	vcdp->fullBus(c+133,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
						 >> 0x30U)))),16);
	vcdp->fullBus(c+134,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2))),8);
	vcdp->fullBus(c+135,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 8U)))),8);
	vcdp->fullBus(c+136,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x10U)))),8);
	vcdp->fullBus(c+137,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x18U)))),8);
	vcdp->fullBus(c+138,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x20U)))),8);
	vcdp->fullBus(c+139,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x28U)))),8);
	vcdp->fullBus(c+140,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x30U)))),8);
	vcdp->fullBus(c+141,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_2 
					       >> 0x38U)))),8);
	vcdp->fullQuad(c+142,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1),64);
	vcdp->fullBus(c+144,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1)),32);
	vcdp->fullBus(c+145,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
				      >> 0x20U))),32);
	vcdp->fullBus(c+146,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1))),16);
	vcdp->fullBus(c+147,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
						 >> 0x10U)))),16);
	vcdp->fullBus(c+148,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
						 >> 0x20U)))),16);
	vcdp->fullBus(c+149,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
						 >> 0x30U)))),16);
	vcdp->fullBus(c+150,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1))),8);
	vcdp->fullBus(c+151,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 8U)))),8);
	vcdp->fullBus(c+152,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x10U)))),8);
	vcdp->fullBus(c+153,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x18U)))),8);
	vcdp->fullBus(c+154,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x20U)))),8);
	vcdp->fullBus(c+155,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x28U)))),8);
	vcdp->fullBus(c+156,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x30U)))),8);
	vcdp->fullBus(c+157,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_1 
					       >> 0x38U)))),8);
	vcdp->fullQuad(c+158,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0),64);
	vcdp->fullBus(c+160,((IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0)),32);
	vcdp->fullBus(c+161,((IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
				      >> 0x20U))),32);
	vcdp->fullBus(c+162,((0xffffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0))),16);
	vcdp->fullBus(c+163,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
						 >> 0x10U)))),16);
	vcdp->fullBus(c+164,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
						 >> 0x20U)))),16);
	vcdp->fullBus(c+165,((0xffffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
						 >> 0x30U)))),16);
	vcdp->fullBus(c+166,((0xffU & (IData)(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0))),8);
	vcdp->fullBus(c+167,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 8U)))),8);
	vcdp->fullBus(c+168,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x10U)))),8);
	vcdp->fullBus(c+169,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x18U)))),8);
	vcdp->fullBus(c+170,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x20U)))),8);
	vcdp->fullBus(c+171,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x28U)))),8);
	vcdp->fullBus(c+172,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x30U)))),8);
	vcdp->fullBus(c+173,((0xffU & (IData)((vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__word_rnd_0 
					       >> 0x38U)))),8);
	vcdp->fullBus(c+174,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk1__DOT__i),32);
	vcdp->fullBus(c+175,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk2__DOT__i),32);
	vcdp->fullBus(c+176,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk3__DOT__i),32);
	vcdp->fullBus(c+177,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk4__DOT__i),32);
	vcdp->fullBus(c+178,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk5__DOT__i),32);
	vcdp->fullBus(c+179,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter__DOT__shifter_PROC__DOT__unnamedblk6__DOT__i),32);
	vcdp->fullBus(c+180,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk1__DOT__i),32);
	vcdp->fullBus(c+181,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk1__DOT__unnamedblk2__DOT__shift_idx),32);
	vcdp->fullBus(c+182,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk3__DOT__i),32);
	vcdp->fullBus(c+183,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk3__DOT__unnamedblk4__DOT__shift_idx),32);
	vcdp->fullBus(c+184,(vlTOPp->simd_shifter_tb__DOT__u_simd_shifter_naive__DOT__shift_naive_PROC__DOT__unnamedblk5__DOT__i),32);
    }
}
