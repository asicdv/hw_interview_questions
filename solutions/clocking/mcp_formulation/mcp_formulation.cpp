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
#include "vobj/Vmcp_formulation.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(l_in_pass_r, bool)                     \
  __func(l_in_r, word_type)                     \
  __func(l_busy_r, bool)                        \
  __func(c_out_pass_r, bool)                    \
  __func(c_out_r, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{dat=" << stim.dat() << "}";
  }
  Stimulus(word_type dat) : dat_(dat) {}
  word_type dat() const { return dat_; }
 private:
  word_type dat_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & x) {
    return os << "'{dat=" << x.dat() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.dat() == b.dat());
  }
  Expect(word_type dat) : dat_(dat) {}
  word_type dat() const { return dat_; }
 private:
  word_type dat_;
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;
  using expected_type = Expect;

  TOP() {
    v.l_rst(l_rst);
    v.l_clk(l_clk);
    v.c_rst(c_rst);
    v.c_clk(c_clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void t_apply_c_reset() {
    c_rst = true;
    t_await_c_cycles(2);
    c_rst = false;
    t_await_c_cycles(2);
  }
  void t_apply_l_reset() {
    l_rst = true;
    t_await_l_cycles(2);
    l_rst = false;
    t_await_l_cycles(2);
  }
  void t_await_c_cycles(std::size_t n = 1) {
    while (n--)
      wait(c_clk.negedge_event());
  }
  void t_await_l_cycles(std::size_t n = 1) {
    while (n--)
      wait(l_clk.negedge_event());
  }
  Vmcp_formulation v{"mcp_formulation"};
  ::sc_core::sc_clock c_clk{"c_clk"};
  ::sc_core::sc_signal<bool> c_rst{"c_rst"};
  ::sc_core::sc_clock l_clk{"l_clk"};
  ::sc_core::sc_signal<bool> l_rst{"l_rst"};
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

TEST(McpFormulationTest, Basic) {
  struct MCPFormulationTask : ::tb::Task {
    MCPFormulationTask(TOP & top) : top_(top){
      bgbool.add(false, 10);
      bgbool.add(true, 10);
      bgbool.finalize();
    }
    void add_stimulus(const Stimulus & s) {
      stimulus_.push_back(s);
      expect_.push_back(Expect{s.dat()});
    }
    void execute() override {
      using namespace sc_core;

      sc_process_handle h_stimulus = 
          sc_spawn(std::bind(&MCPFormulationTask::t_stimulus, this), "t_stimulus");

      sc_process_handle h_checker = 
          sc_spawn(std::bind(&MCPFormulationTask::t_checker, this), "t_checker");
    }
   private:
    void t_stimulus() {
      top_.t_apply_l_reset();
      top_.t_await_l_cycles();
      for (std::size_t i = 0; i < stimulus_.size();) {
        top_.l_in_pass_r = false;
        if (bgbool() && !top_.l_busy_r) {
          top_.l_in_pass_r = true;
          top_.l_in_r = stimulus_[i].dat();
          i++;
        }
        top_.t_await_l_cycles();
      }
      finish();
    }
    void t_checker() {
      std::size_t i = 0;
      top_.t_apply_c_reset();
      top_.t_apply_c_reset();
      while (i < expect_.size()) {
        if (top_.c_out_pass_r) {
          const Expect expect{expect_[i]};
          const Expect actual{top_.c_out_r};

          ASSERT_EQ(expect, actual) << sc_core::sc_time_stamp();
          ++i;
        }
        top_.t_await_c_cycles();
      }
    }
    std::vector<Stimulus> stimulus_;
    std::vector<Expect> expect_;
    tb::Random::Bag<bool> bgbool;
    TOP & top_;
  };
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<MCPFormulationTask>(top);
  tb::Random::UniformRandomInterval<word_type> rnd{};
  for (std::size_t i = 0; i < n; i++)
    task->add_stimulus(Stimulus{rnd()});
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
