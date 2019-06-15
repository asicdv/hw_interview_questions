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
#include "vobj/Vfifo_multi_push.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(push_0, bool)                          \
  __func(push_0_data, word_type)                \
  __func(push_1, bool)                          \
  __func(push_1_data, word_type)                \
  __func(push_2, bool)                          \
  __func(push_2_data, word_type)                \
  __func(push_3, bool)                          \
  __func(push_3_data, word_type)                \
  __func(pop_0, bool)                           \
  __func(pop_0_data_r, word_type)               \
  __func(pop_0_valid_r, bool)                   \
  __func(empty_r, bool)                         \
  __func(full_r, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    os << "'{";
    for (std::size_t i = 0; i < 4; i++) {
      os << ((i != 0) ? "," : "")
         << "push[" << i << "]=" << stim.push(i)
         << ",push_data[" << i << "]=" << stim.push_data(i);
    }
    return os << "}";
  }
  Stimulus() { reset(); }
  void reset() {
    for (std::size_t i = 0; i < 4; i++)
      push_[i] = false;
  }
  void set_push(std::size_t i, bool p = true) { push_[i] = p; }
  void set_push_data(std::size_t i, word_type w = word_type{}) { push_data_[i] = w; }
  bool push(std::size_t i) const { return push_[i]; }
  word_type push_data(std::size_t i) const { return push_data_[i]; }
 private:
  bool push_[4];
  word_type push_data_[4];
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{data=" << res.data() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.data() != b.data())
      return false;

    return true;
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
  void push_idle() {
    push_0 = false;
    push_0_data = word_type{};
    push_1 = false;
    push_1_data = word_type{};
    push_2 = false;
    push_2_data = word_type{};
    push_3 = false;
    push_3_data = word_type{};
  }
  void t_push(const stimulus_type & stim) {
    t_wait_not_full();
    //
    push_0 = stim.push(0);
    push_0_data = stim.push_data(0);
    //
    push_1 = stim.push(1);
    push_1_data = stim.push_data(1);
    //
    push_2 = stim.push(2);
    push_2_data = stim.push_data(2);
    //
    push_3 = stim.push(3);
    push_3_data = stim.push_data(3);

    t_await_cycles(1);
    push_idle();
  }
  void pop_idle() {
    pop_0 = false;
  }
  Expect t_pop() {
    t_wait_not_empty();
    pop_0 = true;
    t_await_cycles();
    pop_idle();
    return Expect{pop_0_valid_r};
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
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vfifo_multi_push v{"fifo_multi_push"};
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
        sc_spawn(std::bind(&FifoNTask::t_pusher, this), "t_pusher");
      sc_process_handle h_popper = 
        sc_spawn(std::bind(&FifoNTask::t_popper, this), "t_popper");
    }
   private:
    void t_pusher() {
      while (!stimulus_.empty()) {
        top_.push_idle();
        if (bgbool()) {
          const Stimulus & stim{stimulus_.front()};
          top_.t_push(stim);
          for (std::size_t i = 0; i < 4; i++) {
            if (!stim.push(i)) break;

            expect_.push_back(Expect{stim.push_data(i)});
          }
          stimulus_.pop_front();
        }
        top_.t_await_cycles();
      }
      top_.push_idle();
      finish();
    }
    void t_popper() {
      while (stimulus_.empty() && expect_.empty()) {
        const Expect rtl{top_.t_pop()};
        const Expect tb{expect_.front()};

        ASSERT_EQ(rtl, tb);
        expect_.pop_front();
      }
    }
    std::deque<Expect> expect_;
    std::deque<Stimulus> stimulus_;
    tb::Random::Bag<bool> bgbool;
    TOP & top_;
  };
  
  auto task = std::make_unique<FifoNTask>(top);
  tb::Random::UniformRandomInterval<word_type> push{3,0}, push_data{};
  for (std::size_t i = 0; i < n; i++) {
    const word_type cnt{push()};

    Stimulus stim{};
    for (std::size_t n = 0; i < cnt; i++) {
      stim.set_push(n, push());
      stim.set_push_data(n, push_data());
    }
    task->add_stimulus(stim);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
