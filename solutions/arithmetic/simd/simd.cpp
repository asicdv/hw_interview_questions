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

#include "libtb/libtb.hpp"
#include <gtest/gtest.h>
#include "vobj/Vsimd.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(op, word_type)                         \
  __func(A, word_type)                          \
  __func(B, word_type)                          \
  __func(Y_r, word_type)                        \
  __func(valid_r, bool)

enum OpT { OP_SEL0 = 0,
    OP_SEL1, OP_ADD32, OP_SUB32, OP_ADD16, OP_SUB16,
    OP_ADD8, OP_SUB8, OP_ADDSUB16, OP_SUBADD16, OP_ADDSUB8,
    OP_SUBADD8
};

struct Stimulus {
  friend struct scv_extensions<Stimulus>;
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{a=" << stim.a() << "}";
  }
  Stimulus() {}
  Stimulus(OpT op, word_type a, word_type b) : op_(op), a_(a), b_(b) {}
  OpT op() const { return op_; }
  word_type a() const { return a_; }
  word_type b() const { return b_; }
 private:
  OpT op_;
  word_type a_, b_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.y() == b.y());
  }
  Expect(word_type y) : y_(y) {}
  word_type y() const { return y_; }
 private:
  word_type y_;
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
struct scv_extensions<Stimulus> : scv_extensions_base<Stimulus> {
  scv_extensions<OpT> op_;
  scv_extensions<word_type> a_, b_;
  SCV_EXTENSIONS_CTOR(Stimulus) {
    SCV_FIELD(op_);
    SCV_FIELD(a_);
    SCV_FIELD(b_);
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

struct SIMDEngine {
  static Expect compute(const Stimulus & stim) {
    word_type r = 0;
    switch (stim.op()) {
    case OP_SEL0: {
      r = stim.a();
    } break;
    case OP_SEL1: {
      r = stim.b();
    } break;
    case OP_ADD32: {
      r = (stim.a() + stim.b());
    } break;
    case OP_SUB32: {
      r = (stim.a() - stim.b());
    } break;
    case OP_ADD16: {
      for (int i = 0; i < 2; i++)
        set16(r, get16(stim.a(), i) + get16(stim.b(), i), i);
    } break;
    case OP_ADDSUB16: {
      set16(r, get16(stim.a(), 0) - get16(stim.b(), 0), 0);
      set16(r, get16(stim.a(), 1) + get16(stim.b(), 1), 1);
    } break;
    case OP_SUBADD16: {
      set16(r, get16(stim.a(), 0) + get16(stim.b(), 0), 0);
      set16(r, get16(stim.a(), 1) - get16(stim.b(), 1), 1);
    } break;
    case OP_ADD8: {
      for (int i = 0; i < 4; i++)
        set8(r, get8(stim.a(), i) + get8(stim.b(), i), i);
    } break;
    case OP_ADDSUB8: {
      set8(r, get8(stim.a(), 0) - get8(stim.b(), 0), 0);
      set8(r, get8(stim.a(), 1) + get8(stim.b(), 1), 1);
      set8(r, get8(stim.a(), 2) - get8(stim.b(), 2), 2);
      set8(r, get8(stim.a(), 3) + get8(stim.b(), 3), 3);
    } break;
    case OP_SUBADD8: {
      set8(r, get8(stim.a(), 0) + get8(stim.b(), 0), 0);
      set8(r, get8(stim.a(), 1) - get8(stim.b(), 1), 1);
      set8(r, get8(stim.a(), 2) + get8(stim.b(), 2), 2);
      set8(r, get8(stim.a(), 3) - get8(stim.b(), 3), 3);
    } break;
    case OP_SUB16: {
      for (int i = 0; i < 2; i++)
        set16(r, get16(stim.a(), i) - get16(stim.b(), i), i);
    } break;
    case OP_SUB8: {
      for (int i = 0; i < 4; i++)
        set8(r, get8(stim.a(), i) - get8(stim.b(), i), i);
    } break;
    }
    return Expect{r};
  }
private:

  static void set16(word_type &y, word_type a, int i) {
    word_type mask = 0xFFFF << (i * 16);
    y &= (~mask);
    y |= (a << (i * 16));
  }
  static word_type get16(word_type y, int i) {
    return (y >> (16 * i)) & 0xFFFF;
  }
  static void set8(word_type &y, word_type a, int i) {
    word_type mask = 0xFF << (i * 8);
    y &= (~mask);
    y |= (a << (i * 8));
  }
  static word_type get8(word_type y, int i) {
    return (y >> (8 * i)) & 0xFF;
  }
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;
  using expected_type = Expect;

  TOP() {
    v.rst(rst);
    v.clk(clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void set_idle() {
    pass = false;
    A = word_type{};
    B = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    pass = true;
    A = stim.a();
    B = stim.b();
    op = stim.op();
    t_await_cycles(1);
    set_idle();
  }
  bool out_is_valid() const { return valid_r; }
  Expect get_expect() const { return Expect{Y_r}; }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {}
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vsimd v{"SIMD"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
  PORTS(__declare_signal)
#undef __declare_signal
 private:
  void start_tracing() {
#ifdef OPT_ENABLE_TRACE
    Verilated::traceEverOn(true);
    vcd_ = std::make_unique<VerilatedVcdSc>();
    v.trace(vcd_.get(), 99);
    vcd_->open("sim.vcd");
#endif
  }
  void stop_tracing() {
#ifdef OPT_ENABLE_TRACE
    vcd_->close();
#endif
  }
#ifdef OPT_ENABLE_TRACE
  std::unique_ptr<VerilatedVcdSc> vcd_;
#endif
};

namespace {

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(SIMDTest, Basic) {
  const std::size_t n{1024 << 5};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  scv_smart_ptr<Stimulus> pstimulus;
  scv_bag<OpT> opts;
  for (int op = OP_SEL0; op != OP_SUBADD8; op++)
    opts.add(static_cast<OpT>(op), 10);
  pstimulus->op_.set_mode(opts);
  
  for (std::size_t i = 0; i < n; i++) {
    const Stimulus stim{*pstimulus};
    task->add_stimulus(stim);
    task->add_expected(SIMDEngine::compute(stim));

    pstimulus->next();
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
