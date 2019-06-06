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
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include <queue>
#include <cmath>
#include "vobj/Vclk_div_by_3.h"

#define PORTS(__func)                           \
  __func(clk_div_3, bool)

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
  void t_await_cycles_slow(std::size_t n = 1) {
    while (n--)
      wait(clk_div_3.negedge_event());
  }
  Vclk_div_by_3 v{"clk_div_by_3"};
  ::sc_core::sc_clock clk{"clk"};
  ::sc_core::sc_signal<bool> rst{"rst"};
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

TEST(ClkDivBy3Test, Basic) {
  struct ClkCounterTask : ::tb::Task {
    ClkCounterTask(TOP & top) : top_(top) {}
    void execute() override {
      {
        sc_spawn_options opts;
        opts.set_sensitivity(&top_.clk.negedge_event());
        opts.spawn_method();
        opts.dont_initialize();

        sc_process_handle h_check = 
            sc_spawn(std::bind(&ClkCounterTask::m_fast_clock, this),
                     "m_fast_clock", &opts );
      }
      {
        sc_spawn_options opts;
        opts.set_sensitivity(&top_.clk_div_3.negedge_event());
        opts.spawn_method();
        opts.dont_initialize();

        sc_process_handle h_check = 
            sc_spawn(std::bind(&ClkCounterTask::m_slow_clock, this),
                     "m_slow_clock", &opts );
      }

      slow_n_ = 0;
      fast_n_ = 0;

      // Run fast clock for 300 cycles. Once elapsed, the slow clock
      // should be (approximately) 300 / 3 = 100.
      //
      top_.t_await_cycles(300);

      ASSERT_EQ(fast_n_, 300);

      top_.t_await_cycles_slow();
      ASSERT_EQ(slow_n_, 100);
      
      finish();
    }
   private:
    void m_slow_clock() {
      ++slow_n_;
    }
    void m_fast_clock() {
      ++fast_n_;
    }
    std::size_t fast_n_, slow_n_;
    TOP & top_;
  };
  auto task = std::make_unique<ClkCounterTask>(top);
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
