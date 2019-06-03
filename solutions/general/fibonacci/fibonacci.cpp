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
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include "vobj/Vfibonacci.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
    __func(y, word_type)

struct Fibonacci {
  Fibonacci() : x0_(1), x1_(2), sel_r_(false) {}
  word_type operator()() {
    word_type ret;
    if (!sel_r_) {
      ret = x0_;
      x0_ += x1_;
    } else {
      ret = x1_;
      x1_ += x0_;
    }
    sel_r_ = !sel_r_;
    return ret;
  }
 private:
  bool sel_r_;
  word_type x0_, x1_;
};

struct TOP : tb::Top {
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
  Vfibonacci v{"fibonacci"};
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

TEST(FibonacciTest, Basic) {
  struct FibonacciSequenceTask : tb::Task {
    explicit FibonacciSequenceTask(std::size_t n = 16)
        : n_(n) {}
    bool is_completed() const override { return is_completed_ || fail(); }
    void execute() override {
      using namespace sc_core;

      top.t_apply_reset();

      for (std::size_t i = 0; i < n_; i++) {
        const word_type actual{top.y};
        const word_type expected{fib()};
        ASSERT_EQ(actual, expected) << "["
                                    << ::sc_core::sc_time_stamp()
                                    << "]: Fail!";
        top.t_await_cycles(1);
      }
      is_completed_ = true;
    }
   private:
    bool fail() const { return ::testing::Test::HasFatalFailure(); }
    bool is_completed_{false};
    std::size_t n_;
    Fibonacci fib;
  };
  std::unique_ptr<FibonacciSequenceTask> task =
      std::make_unique<FibonacciSequenceTask>();
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
