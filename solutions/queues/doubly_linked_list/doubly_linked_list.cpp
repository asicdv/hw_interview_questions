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
#include <vector>
#include <deque>
#include "vobj/Vdoubly_linked_list.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(cmd_pass, bool)                        \
  __func(cmd_op, word_type)                     \
  __func(cmd_id, word_type)                     \
  __func(cmd_push_data, word_type)              \
  __func(cmd_pop_data, word_type)               \
  __func(cmd_pop_data_vld_r, bool)              \
  __func(clear, bool)                           \
  __func(full_r, bool)                          \
  __func(empty_r, bool)                         \
  __func(nempty_r, word_type)                   \
  __func(busy_r, bool)

enum OP : word_type {
  OP_POP_FRONT = 0,
  OP_POP_BACK = 1,
  OP_PUSH_FRONT = 2,
  OP_PUSH_BACK = 3
};

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{id=" << stim.id() << ",data=" << stim.data() << "}";
  }
  Stimulus(word_type op, word_type id, word_type data)
    : op_(op), id_(id), data_(data) {}
  word_type op() const { return op_; }
  word_type id() const { return id_; }
  word_type data() const { return data_; }
 private:
  word_type op_, id_, data_;
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
    cmd_op = OP_POP_FRONT;
    cmd_id = word_type{};
    cmd_push_data = word_type{};
  }
  void t_issue(const stimulus_type & stim) {
    t_wait_not_busy();
    cmd_pass = true;
    cmd_op = stim.op();
    cmd_id = stim.id();
    cmd_push_data = stim.data();
    t_await_cycles(1);
    idle();
  }
  Expect t_get_expect() {
    while (!cmd_pop_data_vld_r)
      t_await_cycles();
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
  void t_wait_not_busy() {
    while (busy_r)
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vdoubly_linked_list v{"doubly_linked_list"};
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

TEST(DoublyLinkedListTest, Basic) {
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
          top_.t_issue(stim);
          switch (stim.op()) {
          case OP_POP_FRONT: {
            std::deque<Expect> & x{expect_[stim.id()]};
            output_.push_back(x.front());
            x.pop_front();
          } break;
          case OP_POP_BACK: {
            std::deque<Expect> & x{expect_[stim.id()]};
            output_.push_back(x.back());
            x.pop_back();
          } break;
          case OP_PUSH_FRONT:
            expect_[stim.id()].push_front(Expect{stim.data()});
            break;
          case OP_PUSH_BACK:
            expect_[stim.id()].push_back(Expect{stim.data()});
            break;
          }
          stimulus_.pop_front();
        } else {
          top_.t_await_cycles();
        }
      }
      top_.idle();
      finish();
    }
    void t_data_collector() {
      auto is_done = [&](){ return stimulus_.empty() && output_.empty(); };
      
      while (!is_done()) {
        const Expect rtl{top_.t_get_expect()};
        const Expect tb{output_.front()};
        ASSERT_EQ(rtl, tb);
        output_.pop_front();
      }
    }
    std::vector<std::deque<Expect> > expect_;
    std::deque<Stimulus> stimulus_;
    std::deque<Expect> output_;
    tb::Random::Bag<bool> bgbool;
    TOP & top_;
  };
  
  auto task = std::make_unique<FifoNTask>(top);
    tb::Random::UniformRandomInterval<word_type> rnd_id{3,0}, rnd_data{};

  std::vector<std::vector<word_type> > qs{4};
  tb::Random::Bag<word_type> bgop;

  bgop.add(OP_POP_FRONT);
  bgop.add(OP_POP_BACK);
  bgop.add(OP_PUSH_FRONT);
  bgop.add(OP_PUSH_BACK);
  bgop.finalize();
  
  for (std::size_t i = 0; i < n; i++) {
    word_type id{rnd_id()};
    word_type op;

    bool done{false};
    while (!done) {
      op = bgop();
      switch (op) {
      case OP_POP_FRONT:
      case OP_POP_BACK:
        done = !qs[id].empty();
        break;
      case OP_PUSH_FRONT:
      case OP_PUSH_BACK:
      default:
        done = true;
        break;
      }
    }
    task->add_stimulus(Stimulus{op, rnd_id(), rnd_data()});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
