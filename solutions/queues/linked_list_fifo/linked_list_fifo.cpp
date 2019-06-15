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
#include <vector>
#include <deque>
#include "vobj/Vlinked_list_fifo.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(cmd_pass, bool)                        \
  __func(cmd_push, bool)                        \
  __func(cmd_id, word_type)                     \
  __func(cmd_push_data, word_type)              \
  __func(cmd_pop_data, word_type)               \
  __func(cmd_pop_data_vld_r, bool)              \
  __func(clear, bool)                           \
  __func(full_r, bool)                          \
  __func(empty_r, bool)                         \
  __func(nempty_r, word_type)                   \
  __func(busy_r, bool)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{id=" << stim.id() << ",data=" << stim.data() << "}";
  }
  Stimulus(word_type id, word_type data) : id_(id), data_(data) {}
  word_type id() const { return id_; }
  word_type data() const { return data_; }
 private:
  word_type id_, data_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{data=" << res.data() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return a.data() == b.data();
  }
  Expect(word_type data) : data_(data) {}
  word_type data() const { return data_; }
 private:
  word_type data_;
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
  void idle() {
    cmd_pass = false;
    cmd_push = word_type{};
    cmd_id = word_type{};
    cmd_push_data = word_type{};
  }
  void t_push(const stimulus_type & stim) {
    t_wait_not_full();
    cmd_pass = true;
    cmd_push = true;
    cmd_id = stim.id();
    cmd_push_data = stim.data();
    t_await_cycles(1);
    idle();
  }
  Expect t_pop(word_type vq) {
    t_wait_not_nempty(vq);
    cmd_pass = true;
    cmd_push = false;
    cmd_id = vq;
    t_await_cycles(1);
    idle();
    return Expect{cmd_pop_data};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_full() {
    while (full_r)
      t_await_cycles();
  }
  void t_wait_not_empty() {
    while (empty_r)
      t_await_cycles();
  }
  void t_wait_not_nempty(word_type id) {
    while (tb::bit(nempty_r, id))
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vlinked_list_fifo v{"linked_list_fifo"};
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

TEST(MultiplierTest, Basic) {
  const std::size_t n{1024 << 6};
  struct FifoNTask : tb::Task {
    FifoNTask(TOP & top) : top_(top) {
      expect_.resize(4);
      
      bgbool.add(false, 10);
      bgbool.add(true, 10);
      bgbool.finalize();
    }
    void add_stimulus(const Stimulus & s) {
      stimulus_.push_back(s);
    }
    void execute() override {
      using namespace sc_core;

      top_.t_apply_reset();
      
      sc_process_handle h_pusher = 
        sc_spawn(std::bind(&FifoNTask::t_command_issue, this), "t_command_issue");
      sc_process_handle h_popper = 
        sc_spawn(std::bind(&FifoNTask::t_data_collector, this), "t_data_collector");
    }
   private:
    void t_command_issue() {
      while (!stimulus_.empty()) {
        top_.idle();
        if (bgbool()) {
          const Stimulus & stim{stimulus_.front()};
          top_.t_push(stim);
          expect_[stim.id()].push_back(Expect{stim.data()});
          stimulus_.pop_front();
        } else {
          top_.t_await_cycles();
        }
      }
      top_.idle();
      finish();
    }
    void t_data_collector() {
      tb::Random::Bag<word_type> bg;
      std::vector<word_type> available_id;

      bool done{false};
      while (!done) {
        get_outstanding(available_id);
        if (available_id.empty()) {
          done = stimulus_.empty();
          if (!done)
            top_.t_await_cycles(10);
        } else {
          bg.add(available_id.begin(), available_id.end());
          const word_type id{bg()};
          const Expect rtl{top_.t_pop(id)};
          const Expect tb{expect_[id].front()};
          ASSERT_EQ(rtl, tb);
          expect_[id].pop_front();
          bg.clear();
        }
      }
    }
    void get_outstanding(std::vector<word_type> & v) const {
      v.clear();
      for (std::size_t q = 0; q < expect_.size(); q++) {
        if (!expect_[q].empty())
          v.push_back(q);
      }
    }
    std::vector<std::deque<Expect> > expect_;
    std::deque<Stimulus> stimulus_;
    tb::Random::Bag<bool> bgbool;
    TOP & top_;
  };
  
  auto task = std::make_unique<FifoNTask>(top);
    tb::Random::UniformRandomInterval<word_type> rnd_vq{3,0}, rnd_data{};
  for (std::size_t i = 0; i < n; i++)
    task->add_stimulus(Stimulus{rnd_vq(), rnd_data()});
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
