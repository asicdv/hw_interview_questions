//========================================================================== //
// Copyright (c) 2016-18, Stephen Henry
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
#include "vobj/Vlatency.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
    __func(issue, bool)                         \
    __func(clear, bool)                         \
    __func(retire, bool)                        \
    __func(issue_cnt_r, word_type)              \
    __func(aggregate_cnt_r, word_type)

struct TOP : tb::Top {
  TOP() {
    v.clk(clk);
    v.rst(rst);
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
    issue = false;
    clear = false;
    retire = false;
  }
  void set_issue(bool x = true) { issue = x; }
  void set_clear(bool x = true) { clear = x; }
  void set_retire(bool x = true) { retire = x; }
  word_type get_issue_cnt_r() { return issue_cnt_r; }
  word_type get_aggregate_cnt_r() { return aggregate_cnt_r; }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  Vlatency v{"latency"};
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

struct LatencyModel {
  explicit LatencyModel() {}
  bool has_in_flight() const { return in_flight_cnt_ != 0; }

  std::size_t aggregate_cnt() const { return aggregate_cnt_; }
  std::size_t issue_cnt() const { return issue_cnt_; }
  void issue() { issue_cnt_++; in_flight_cnt_++; }
  void retire() { in_flight_cnt_--; }
  void step() { aggregate_cnt_ += in_flight_cnt_; };
 private:
  std::size_t aggregate_cnt_{0}, issue_cnt_{0};
  std::size_t in_flight_cnt_{0};
};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(DetectSequenceTest, Basic) {
  struct RunLatencyTask : ::tb::Task {
    RunLatencyTask(TOP & top, std::size_t n = 1024) : top_(top), n_(n) {}
    bool is_completed() const override { return is_completed_; }
    void execute() override {
      LatencyModel mdl{};
      
      tb::Random::Bag<bool> bg_issue;
      bg_issue.add(true, 80);
      bg_issue.add(false, 20);
      bg_issue.finalize();

      tb::Random::Bag<bool> bg_retire;
      bg_retire.add(true, 80);
      bg_retire.add(false, 20);
      bg_retire.finalize();

      // Issue/Retirements
      top_.t_apply_reset();
      for (std::size_t cycle = 0; cycle < n_; cycle++) {
        const bool issue{bg_issue()};
        const bool retire{mdl.has_in_flight() && bg_retire()};

        top_.set_idle();
        top_.set_issue(issue);
        top_.set_retire(retire);

        if (issue)
          mdl.issue();
        if (retire)
          mdl.retire();

        mdl.step();
        top_.t_await_cycles();
      }

      // Wind up
      while (mdl.has_in_flight()) {
        top_.set_idle();
        top_.set_retire(true);
        mdl.retire();
        mdl.step();
        top_.t_await_cycles();
      }

      // Return to quiescent state.
      top_.set_idle();

      ASSERT_EQ(top_.get_issue_cnt_r(), mdl.issue_cnt());
      ASSERT_EQ(top_.get_aggregate_cnt_r(), mdl.aggregate_cnt());

      is_completed_ = true;
    }
   private:
    TOP & top_;
    bool is_completed_{false};
    std::size_t n_;
  };
  auto task = std::make_unique<RunLatencyTask>(top);
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
