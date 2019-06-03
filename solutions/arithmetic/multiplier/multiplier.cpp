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
#include <gtest/gtest.h>
#include <queue>
#include <sstream>
#include <functional>
#include "vobj/Vmultiplier.h"

using word_type = uint32_t;
using result_type = vluint64_t;

#define PORTS(__func)                           \
  __func(a, word_type)                          \
  __func(b, word_type)                          \
  __func(pass, bool)                            \
  __func(y, result_type)                        \
  __func(y_vld_r, bool)                         \
  __func(busy_r, bool)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{a=" << stim.a() << "}";
  }
  Stimulus(word_type a, word_type b) : a_(a), b_(b) {}
  word_type a() const { return a_; }
  word_type b() const { return b_; }
 private:
  word_type a_, b_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.y() == b.y());
  }
  Expect(result_type y) : y_(y) {}
  result_type y() const { return y_; }
 private:
  result_type y_;
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
  void set_idle() {
    pass = false;
    a = word_type{};
    b = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    pass = true;
    a = stim.a();
    b = stim.b();
    t_await_cycles(1);
    set_idle();
  }
  bool out_is_valid() const { return y_vld_r; }
  Expect get_expect() const { return Expect{y}; }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {
    while (busy_r)
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vmultiplier v{"multiplier"};
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
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  tb::Random::UniformRandomInterval<uint32_t> rnd{1024};
  for (std::size_t i = 0; i < n; i++) {
    const Stimulus stim{rnd(), rnd()};
    task->add_stimulus(stim);
    task->add_expected(Expect{stim.a() * stim.b()});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
