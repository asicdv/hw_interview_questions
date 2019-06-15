//========================================================================== //
// Copyright (c) 2016-17, Stephen Henry
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
#include <vector>
#include <deque>
#include <tuple>
#include <set>
#include <algorithm>
#include "vobj/Vrob.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(alloc_vld, bool)                       \
  __func(alloc_data, word_type)                 \
  __func(alloc_rdy, bool)                       \
  __func(alloc_id, word_type)                   \
  __func(cmpl_vld, bool)                        \
  __func(cmpl_id, word_type)                    \
  __func(cmpl_data, word_type)                  \
  __func(retire_rdy, bool)                      \
  __func(retire_cmpl_data, word_type)           \
  __func(retire_alloc_data, word_type)          \
  __func(retire_vld, bool)                      \
  __func(clear, bool)                           \
  __func(idle_r, bool)                          \
  __func(full_r, bool)

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
  void alloc_idle() {
    alloc_vld = false;
    alloc_data = word_type{};
  }
  word_type t_alloc_issue(word_type data) {
    word_type id;
    alloc_vld = true;
    alloc_data = data;
    id = alloc_id;
    do { t_await_cycles(1); } while (!alloc_rdy);
    alloc_idle();
    return id;
  }
  void cmpl_idle() {
    cmpl_vld = false;
    cmpl_id = word_type{};
    cmpl_data = word_type{};
  }
  void cmpl_issue(word_type id, word_type data) {
    cmpl_vld = true;
    cmpl_id = id;
    cmpl_data = data;
    t_await_cycles(1);
    cmpl_idle();
  }
  auto t_retire() {
    retire_rdy = true;
    while (!retire_vld)
      t_await_cycles();

    auto res{std::make_tuple(retire_cmpl_data.read(), retire_alloc_data.read())};
    t_await_cycles();
    return res;
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
  Vrob v{"rob"};
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

TEST(RobTest, Basic) {
  const std::size_t n{1024 << 6};
  struct FifoNTask : tb::Task {
    FifoNTask(TOP & top) : top_(top) {
    }
    void add_stimulus(word_type w) {
      stimulus_.push_back(w);
    }
    void execute() override {
      using namespace sc_core;

      top_.t_apply_reset();
      
      sc_process_handle h_alloc = 
        sc_spawn(std::bind(&FifoNTask::t_alloc, this), "t_alloc");
      sc_process_handle h_cmpl = 
        sc_spawn(std::bind(&FifoNTask::t_cmpl, this), "t_cmpl");
      sc_process_handle h_retire = 
        sc_spawn(std::bind(&FifoNTask::t_retire, this), "t_retire");
    }
   private:
    void t_alloc() {
      tb::Random::UniformRandomInterval<word_type> dly{9, 0};
      for (std::size_t i = 0; i < stimulus_.size(); i++) {
        const word_type alloc_data{stimulus_[i]};
        const word_type id{top_.t_alloc_issue(alloc_data)};

        alloc_data_[id] = alloc_data;
        pending_.insert(id);
        expect_.push_back(id);

        top_.t_await_cycles(dly());
      }
      finish();
    }
    void t_cmpl() {
      tb::Random::UniformRandomInterval<word_type> dly{9, 0};
      tb::Random::UniformRandomInterval<word_type> word;
      tb::Random::Bag<word_type> bg;
      while (true) {
        top_.t_await_cycles(dly());
        if (!pending_.empty()) {
          bg.add(pending_.begin(), pending_.end());

          const word_type cmpl_id{bg()};
          pending_.erase(pending_.find(cmpl_id));

          const word_type cmpl_data{word()};
          top_.cmpl_issue(cmpl_id, cmpl_data);
          cmpl_data_[cmpl_id] = cmpl_data;
          bg.clear();
        }
      }
    }
    void t_retire() {
      while (true) {
        auto const rtlout{top_.t_retire()};
        const word_type id{expect_.front()};
        auto const tbout{std::make_tuple(cmpl_data_[id], alloc_data_[id])};
        ASSERT_EQ(rtlout, tbout) << ::sc_core::sc_time_stamp();
        expect_.pop_front();
      }
    }
    TOP & top_;
    std::map<word_type, word_type> alloc_data_, cmpl_data_;
    std::set<word_type> pending_;
    std::deque<word_type> stimulus_;
    std::deque<word_type> expect_;
  };
  
  auto task = std::make_unique<FifoNTask>(top);
    tb::Random::UniformRandomInterval<word_type> rnd_data{};
  for (std::size_t i = 0; i < n; i++)
    task->add_stimulus(rnd_data());
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
