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
#include "vobj/Vmissing_duplicated_word.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(state_upt, bool)                       \
  __func(state_id, word_type)                   \
  __func(state_dat, word_type)                  \
  __func(cntrl_start, bool)                     \
  __func(cntrl_busy_r, bool)                    \
  __func(cntrl_dat_r, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{x=" << stim.x() << "}";
  }
  Stimulus(word_type x) : x_(x) {}
  word_type x() const { return x_; }
 private:
  word_type x_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.y() == b.y());
  }
  Expect() {}
  Expect(word_type y) : y_(y) {}
  word_type y() const { return y_; }
 private:
  word_type y_;
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
  void t_state_idle() {
    state_upt = false;
  }
  void t_state_set(word_type id, word_type dat) {
    state_upt = true;
    state_id = id;
    state_dat = dat;
    t_await_cycles(1);
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_cntrl_idle() {
    cntrl_start = false;
  }
  void t_cntrl_start() {
    cntrl_start = true;
    t_await_cycles(1);
    t_cntrl_idle();
  }
  Expect get_expect() const { return Expect{cntrl_dat_r}; }
  void t_cntrl_await_not_busy() {
    while (cntrl_busy_r)
      t_await_cycles(1);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vmissing_duplicated_word v{"missing_duplicated_word"};
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

struct TestCase {
  friend std::ostream & operator<<(std::ostream & os, const TestCase & t) {
    os << "'{expect=" << t.expect() << ", ";
    const std::vector<word_type> & words{t.words()};
    for (std::size_t i = 0; i < words.size(); i++)
      os << (i == 0 ? "[" : " ") << words[i];
    os << "]";
    return os << "}";
  }
  
  static TestCase generate(word_type w, word_type n) {
    tb::Random::UniformRandomInterval<word_type> rnd((1 << w) - 1);
    tb::Random::NotPriorPredicate<word_type> pred;
    tb::Random::Filter<tb::Random::UniformRandomInterval<word_type>,
                       tb::Random::NotPriorPredicate<word_type> > filter{rnd, pred};
    
    TestCase t;
    for (std::size_t n = 0; n < 8; n++) {
      const word_type duplicated{filter()};

      t.words_.push_back(duplicated);
      t.words_.push_back(duplicated);
    }
    const word_type not_duplicated{filter()};
    t.words_.push_back(not_duplicated);
    t.expect_ = Expect{not_duplicated};
    tb::Random::shuffle(t.words_.begin(), t.words_.end());
    return t;
  }

  Expect expect() const { return expect_; }
  const std::vector<word_type> & words() const { return words_; }
 private:
  Expect expect_;
  std::vector<word_type> words_;
};

TEST(MissingDuplicateWordTest, Basic) {
  const std::size_t OPT_W{5}, OPT_N{17};
  struct DriverScenario : ::tb::Task {
    void add(const TestCase & c) { tests_.push_back(c); }
    bool is_completed() const override { return is_completed_; }
    void execute() override {
      top.t_apply_reset();
      for (const TestCase & c : tests_) {
        for (std::size_t id = 0; id < c.words().size(); id++)
          top.t_state_set(id, c.words()[id]);
        top.t_state_idle();

        top.t_cntrl_start();
        top.t_cntrl_await_not_busy();

        const Expect actual{top.get_expect()};
        const Expect expect{c.expect()};
        ASSERT_EQ(actual, expect);
      }
      is_completed_ = true;
    }
   private:
    bool is_completed_{false};
    std::vector<TestCase> tests_;
  };
  std::unique_ptr<DriverScenario> task =
      std::make_unique<DriverScenario>();
  const std::size_t N{1024 << 3};
  for (std::size_t i = 0; i < N; i++)
    task->add(TestCase::generate(OPT_W, OPT_N));
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
