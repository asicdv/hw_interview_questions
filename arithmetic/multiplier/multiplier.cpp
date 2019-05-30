//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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

#define TB_ENABLE_TRACING

#include <systemc>
#include <scv.h>
#include <gtest/gtest.h>
#include <queue>
#include <sstream>
#include "vobj/Vmultiplier.h"
#ifdef TB_ENABLE_TRACING
#  include "verilated_vcd_sc.h"
#endif

using word_type = uint32_t;
using result_type = vluint64_t;

#define PORTS(__func)                           \
  __func(a, word_type)                          \
  __func(b, word_type)                          \
  __func(pass, bool)                            \
  __func(y, result_type)                        \
  __func(y_vld_r, bool)                         \
  __func(busy_r, bool)

static struct TOP {
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
    a = word_type{};
    b = word_type{};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vmultiplier v{"multiplier"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
  PORTS(__declare_signal)
#undef __declare_signal
 private:
  void start_tracing() {
#ifdef TB_ENABLE_TRACING
    Verilated::traceEverOn(true);
    vcd_ = std::make_unique<VerilatedVcdSc>();
    v.trace(vcd_.get(), 99);
    vcd_->open("sim.vcd");
#endif
  }
  void stop_tracing() {
#ifdef TB_ENABLE_TRACING
    vcd_->close();
#endif
  }
#ifdef TB_ENABLE_TRACING
  std::unique_ptr<VerilatedVcdSc> vcd_;
#endif
} TOP;

struct Expect {
  Expect(word_type a, word_type b, result_type y)
      : a_(a), b_(b), y_(y) {}
  word_type a() const { return a_; }
  word_type b() const { return b_; }
  result_type y() const { return y_; }
 private:
  word_type a_, b_;
  result_type y_;
};

std::ostream & operator<<(std::ostream & os, const Expect & ex) {
  return os << "'{a=" << ex.a() << ", b=" << ex.b() << ", y=" << ex.y() << "}";
}

static class TB : public ::sc_core::sc_module {
 public:
  SC_HAS_PROCESS(TB);
  TB() : ::sc_core::sc_module{
    ::sc_core::sc_module_name{"MultiplierTest"}} {
    SC_THREAD(t_stimulus);
    
    SC_METHOD(m_checker);
    sensitive << TOP.clk.negedge_event();
    dont_initialize();
  }
  void add_stimulus(word_type a, word_type b, result_type y) {
    stimulus_.push(Expect{a,b,y});
  }
  void run_until_exhausted() {
    e_start_tb_.notify(1, SC_NS);
    while (!stimulus_.empty())
      ::sc_core::sc_start(10, SC_US);
  }

 private:
  void t_stimulus() {
    while (true) {
      wait(e_start_tb_);
      t_main_loop();
    }
  }
  void t_main_loop() {
    TOP.set_idle();
    TOP.t_apply_reset();
    while (!stimulus_.empty()) {
      const Expect & ex{stimulus_.front()};

      TOP.pass = true;
      TOP.a = ex.a();
      TOP.b = ex.b();
      TOP.t_await_cycles(1);

      expected_.push(ex);
      stimulus_.pop();
    }
    TOP.set_idle();
    wait (100, SC_NS);
  }
  void m_checker() {
    if (TOP.y_vld_r) {
      EXPECT_FALSE(expected_.empty());

      const Expect & ex{expected_.front()};
      EXPECT_EQ(TOP.y, ex.y());
      if (TOP.y != ex.y()) {
        std::ostringstream os;
        os << "Stimulus mismatch: " << ex;
        const std::string s{os.str()};
        SC_REPORT_ERROR("/stephenry", s.c_str());
      }
      expected_.pop();
    }
  }
  std::queue<Expect> stimulus_, expected_;
  ::sc_core::sc_event e_start_tb_;
} TB;

TEST(MultiplierTest, Basic) {
  struct stimulus_constraint : scv_constraint_base {
    scv_smart_ptr<uint32_t> v;
    SCV_CONSTRAINT_CTOR(stimulus_constraint) {
      SCV_CONSTRAINT(v() < 1024);
    }
  } a_ptr("a_constrained"), b_ptr("b_constrained");

  for (std::size_t i = 0; i < 1024; i++) {
    const uint32_t a{*a_ptr.v};
    const uint32_t b{*b_ptr.v};

    TB.add_stimulus(a, b, (a * b));

    a_ptr.next();
    b_ptr.next();
  }
  TB.run_until_exhausted();
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::sc_core::sc_report_handler::set_actions(
       "/stephenry", (SC_LOG | SC_DISPLAY | SC_STOP));
  return RUN_ALL_TESTS();
}
