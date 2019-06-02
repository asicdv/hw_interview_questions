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

#include "libtb/libtb.hpp"
#include <gtest/gtest.h>
#include <queue>
#include <sstream>
#include <functional>
#include "vobj/Vmultiplier.h"

using word_type = uint32_t;
using result_type = vluint64_t;

#define PORTS(__func)                           \
  __func(a, word_type)                          \
  __func(b, word_type)                          \
  __func(pass, bool)                            \
  __func(y, result_type)                        \
  __func(y_vld_r, bool)                         \
  __func(busy_r, bool)

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & ex) {
    return os << "'{a=" << ex.a() << ", b=" << ex.b() << ", y=" << ex.y() << "}";
  }

  Expect(word_type a, word_type b, result_type y)
      : a_(a), b_(b), y_(y) {}
  word_type a() const { return a_; }
  word_type b() const { return b_; }
  result_type y() const { return y_; }
 private:
  word_type a_, b_;
  result_type y_;
};

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
  void idle() {
    pass = false;
    a = word_type{};
    b = word_type{};
  }
  void pass_cmd(word_type set_a, word_type set_b) {
    t_wait_not_busy();
    pass = true;
    a = set_a;
    b = set_b;
    t_await_cycles(1);
    idle();
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {
    while (busy_r)
      t_await_cycles();
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
} TOP;

namespace { tb::TaskRunner TaskRunner; } // namespace

TEST(MultiplierTest, Basic) {
  struct BasicTask : tb::Task {
    explicit BasicTask(std::size_t n = 10)
        : n_(n) {}
    void execute() override {
      setup();
      launch();
    }
   private:
    void setup() {
      struct stimulus_constraint : scv_constraint_base {
        scv_smart_ptr<uint32_t> v;
        SCV_CONSTRAINT_CTOR(stimulus_constraint) {
          SCV_CONSTRAINT(v() < 1024);
        }
      } a_ptr("a_constrained"), b_ptr("b_constrained");

      for (std::size_t i = 0; i < n_; i++) {
        const uint32_t a{*a_ptr.v};
        const uint32_t b{*b_ptr.v};

        stimulus_.push(Expect{a, b, (a * b)});

        a_ptr.next();
        b_ptr.next();
      }
    }
    bool is_completed() const override {
      return is_completed_;
    }
    void launch() {
      using namespace sc_core;

      // Fork:
      sc_process_handle h_stim =
          sc_spawn(std::bind(&BasicTask::t_stimulus, this), "t_stimulus");

      sc_spawn_options copts;
      copts.set_sensitivity(&TOP.clk.negedge_event());
      copts.spawn_method();
      copts.dont_initialize();

      sc_process_handle h_check = 
          sc_spawn(std::bind(&BasicTask::m_checker, this), "m_checker", &copts );

      // Join:
      wait(h_stim.terminated_event());
      if (!h_check.terminated())
        h_check.kill();
    }
    void t_stimulus() {
      TOP.idle();
      TOP.t_apply_reset();
      while (!stimulus_.empty() && !fail()) {
        const Expect & ex{stimulus_.front()};

        TOP.pass_cmd(ex.a(), ex.b());

        expected_.push(ex);
        stimulus_.pop();
      }
      TOP.idle();
      wait (100, SC_NS);
      is_completed_ = true; 
    }
    void m_checker() {
      if (TOP.y_vld_r) {
        ASSERT_FALSE(expected_.empty());

        const Expect & ex{expected_.front()};
        ASSERT_EQ(TOP.y, ex.y()) << "["
                                 << ::sc_core::sc_time_stamp()
                                 << "]: Stimulus mismatch: "
                                 << ex;
        expected_.pop();
      }
    }
    bool fail() const { return ::testing::Test::HasFatalFailure(); }
    bool is_completed_{false};
    std::size_t n_;
    std::queue<Expect> stimulus_, expected_;
  };
  TaskRunner.set_task(std::make_unique<BasicTask>(1024 << 3));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
