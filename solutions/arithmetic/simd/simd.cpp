//========================================================================== //
// Copyright (c) 2016, Stephen Henry
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//========================================================================== //

#include <libtb2.hpp>
#include <deque>
#include "vobj/Vsimd.h"

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(op, uint32_t)                          \
  __func(A, uint32_t)                           \
  __func(B, uint32_t)                           \
  __func(Y_r, uint32_t)                         \
  __func(valid_r, bool)

enum OpT { OP_SEL0 = 0,
    OP_SEL1, OP_ADD32, OP_SUB32, OP_ADD16, OP_SUB16,
    OP_ADD8, OP_SUB8, OP_ADDSUB16, OP_SUBADD16, OP_ADDSUB8,
    OP_SUBADD8
};

struct SIMDCmd {
  OpT op;
  uint32_t A, B;
};


template<>
struct scv_extensions<OpT> : scv_enum_base<OpT> {
  SCV_ENUM_CTOR(OpT) {
    SCV_ENUM(OP_SEL0);
    SCV_ENUM(OP_SEL1);
    SCV_ENUM(OP_ADD32);
    SCV_ENUM(OP_SUB32);
    SCV_ENUM(OP_ADD16);
    SCV_ENUM(OP_SUB16);
    SCV_ENUM(OP_ADD8);
    SCV_ENUM(OP_SUB8);
    SCV_ENUM(OP_ADDSUB16);
    SCV_ENUM(OP_SUBADD16);
    SCV_ENUM(OP_ADDSUB8);
    SCV_ENUM(OP_SUBADD8);
  }
};

template<>
struct scv_extensions<SIMDCmd> : scv_extensions_base<SIMDCmd> {
  scv_extensions<OpT> op;
  scv_extensions<uint32_t> A, B;
  SCV_EXTENSIONS_CTOR(SIMDCmd) {
    SCV_FIELD(op);
    SCV_FIELD(A);
    SCV_FIELD(B);
  }
};

const char * to_string(const OpT op) {
  switch (op) {
#define _stringify(OP) case OP: return #OP; break;
    _stringify(OP_SEL0)
    _stringify(OP_SEL1)
    _stringify(OP_ADD32)
    _stringify(OP_SUB32)
    _stringify(OP_ADD16)
    _stringify(OP_SUB16)
    _stringify(OP_ADDSUB16)
    _stringify(OP_SUBADD16)
    _stringify(OP_ADD8)
    _stringify(OP_SUB8)
    _stringify(OP_ADDSUB8)
    _stringify(OP_SUBADD8)
#undef _stringify
  }
  return "<Invalid Op>";
}

typedef Vsimd uut_t;

struct SIMDEngine {
  static uint32_t compute(const SIMDCmd & c) {
    uint32_t r = 0;
    switch (c.op) {
    case OP_SEL0: {
      r = c.A;
    } break;
    case OP_SEL1: {
      r = c.B;
    } break;
    case OP_ADD32: {
      r = (c.A + c.B);
    } break;
    case OP_SUB32: {
      r = (c.A - c.B);
    } break;
    case OP_ADD16: {
      for (int i = 0; i < 2; i++)
        set16(r, get16(c.A, i) + get16(c.B, i), i);
    } break;
    case OP_ADDSUB16: {
      set16(r, get16(c.A, 0) - get16(c.B, 0), 0);
      set16(r, get16(c.A, 1) + get16(c.B, 1), 1);
    } break;
    case OP_SUBADD16: {
      set16(r, get16(c.A, 0) + get16(c.B, 0), 0);
      set16(r, get16(c.A, 1) - get16(c.B, 1), 1);
    } break;
    case OP_ADD8: {
      for (int i = 0; i < 4; i++)
        set8(r, get8(c.A, i) + get8(c.B, i), i);
    } break;
    case OP_ADDSUB8: {
      set8(r, get8(c.A, 0) - get8(c.B, 0), 0);
      set8(r, get8(c.A, 1) + get8(c.B, 1), 1);
      set8(r, get8(c.A, 2) - get8(c.B, 2), 2);
      set8(r, get8(c.A, 3) + get8(c.B, 3), 3);
    } break;
    case OP_SUBADD8: {
      set8(r, get8(c.A, 0) + get8(c.B, 0), 0);
      set8(r, get8(c.A, 1) - get8(c.B, 1), 1);
      set8(r, get8(c.A, 2) + get8(c.B, 2), 2);
      set8(r, get8(c.A, 3) - get8(c.B, 3), 3);
    } break;
    case OP_SUB16: {
      for (int i = 0; i < 2; i++)
        set16(r, get16(c.A, i) - get16(c.B, i), i);
    } break;
    case OP_SUB8: {
      for (int i = 0; i < 4; i++)
        set8(r, get8(c.A, i) - get8(c.B, i), i);
    } break;
    }
    return r;
  }
private:

  static void set16(uint32_t &y, uint32_t a, int i) {
    uint32_t mask = 0xFFFF << (i * 16);
    y &= (~mask);
    y |= (a << (i * 16));
  }
  static uint32_t get16(uint32_t y, int i) {
    return (y >> (16 * i)) & 0xFFFF;
  }
  static void set8(uint32_t &y, uint32_t a, int i) {
    uint32_t mask = 0xFF << (i * 8);
    y &= (~mask);
    y |= (a << (i * 8));
  }
  static uint32_t get8(uint32_t y, int i) {
    return (y >> (8 * i)) & 0xFF;
  }
};

struct SimdTb : libtb2::Top<SimdTb> {
  SC_HAS_PROCESS(SimdTb);
  SimdTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {

    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports

    SC_THREAD(t_stimulus);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();

    st_.reset();
  }
  void end_of_simulation() {
    LOGGER(INFO) << "Transaction count = " << st_.n << "\n";
  }
private:
  void m_checker() {
    if (valid_r_) {
      const uint32_t expected = expected_.front(); expected_.pop_front();
      const uint32_t actual = Y_r_;

      LOGGER(INFO) << " Expected = " << expected
                   << " Actual = " << actual
                   << "\n";
      LIBTB2_ERROR_ON(expected != actual);
    }
  }
  void t_stimulus() {
    resetter_.wait_reset_done();

    scv_smart_ptr<SIMDCmd> cmd;
    
    scv_bag<OpT> opts;
    for (int op = OP_SEL0; op != OP_SUBADD8; op++)
      opts.add(static_cast<OpT>(op), 10);

    cmd->op.set_mode(opts);

    while (true) {
      cmd->next();

      // Send to RTL
      pass_ = true; op_ = cmd->op; A_ = cmd->A; B_ = cmd->B;
      expected_.push_back(SIMDEngine::compute(*cmd));

      st_.n++;
      wait(clk_.posedge_event());
    }
  }
  struct {
    void reset() {
      n = 0;
    }
    std::size_t n;
  } st_;
  std::deque<uint32_t> expected_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};

SC_MODULE_EXPORT(SimdTb);

int sc_main(int argc, char **argv) {
  SimdTb tb;
  return libtb2::Sim::start(argc, argv);
}
