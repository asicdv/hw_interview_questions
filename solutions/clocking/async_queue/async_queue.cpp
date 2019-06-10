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
#include <queue>
#include <cmath>
#include "vobj/Vasync_queue.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(push, bool)                            \
  __func(push_data, word_type)                  \
  __func(pop, bool)                             \
  __func(pop_data, word_type)                   \
  __func(pop_data_vld_r, bool)                  \
  __func(empty_r, bool)                         \
  __func(full_r, bool)

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
    v.wrst(wrst);
    v.wclk(wclk);
    v.rrst(rrst);
    v.rclk(rclk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void t_apply_rreset() {
    rrst = true;
    t_await_rcycles(2);
    rrst = false;
    t_await_rcycles(2);
  }
  void t_apply_wreset() {
    wrst = true;
    t_await_wcycles(2);
    wrst = false;
    t_await_wcycles(2);
  }
  void t_wait_not_full() {
    while (full_r)
      t_await_wcycles();
  }
  void t_wait_not_empty() {
    while (empty_r)
      t_await_rcycles();
  }
  void t_await_rcycles(std::size_t n = 1) {
    while (n--)
      wait(rclk.negedge_event());
  }
  void t_await_wcycles(std::size_t n = 1) {
    while (n--)
      wait(wclk.negedge_event());
  }
  Vasync_queue v{"async_queue"};
  ::sc_core::sc_clock wclk{"wclk"}, rclk{"rclk"};
  ::sc_core::sc_signal<bool> wrst{"wrst"}, rrst{"rrst"};
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

TEST(AsyncQueueTest, Basic) {
  const std::size_t n{1024 << 3};
  struct AsyncQueueTask : tb::Task {
    AsyncQueueTask(TOP & top) : top_(top) {
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
          sc_spawn(std::bind(&AsyncQueueTask::t_stimulus, this), "t_stimulus");

      sc_process_handle h_checker = 
          sc_spawn(std::bind(&AsyncQueueTask::t_checker, this), "t_checker");
    }
   private:
    void t_stimulus() {
      top_.t_apply_wreset();
      
      top_.t_await_wcycles();
      for (std::size_t i = 0; i < stimulus_.size();) {
        top_.push = false;
        if (bgbool() && !top_.full_r) {
          top_.push = true;
          top_.push_data = stimulus_[i].dat();
          i++;
        }
        top_.t_await_wcycles();
      }
      top_.push = false;
      finish();
    }
    void t_checker() {
      std::size_t i = 0;
      
      top_.t_apply_rreset();
      while (i < expect_.size()) {
        top_.pop = bgbool() && !top_.empty_r;

        if (top_.pop_data_vld_r) {
          const Expect expect{expect_[i]};
          const Expect actual{top_.pop_data};

          ASSERT_EQ(expect, actual) << sc_core::sc_time_stamp();
          ++i;
        }
        top_.t_await_rcycles();
      }
    }
    std::vector<Stimulus> stimulus_;
    std::vector<Expect> expect_;
    tb::Random::Bag<bool> bgbool;
    TOP & top_;
  };
  
  auto task = std::make_unique<AsyncQueueTask>(top);
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
