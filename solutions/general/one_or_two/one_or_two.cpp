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

//#include <libtb2.hpp>
//#include <sstream>
//#include <map>

#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include "vobj/Vone_or_two.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(x, word_type)                          \
  __func(inv, bool)                             \
  __func(has_vld_r, bool)                       \
  __func(has_set_0_r, bool)                     \
  __func(has_set_1_r, bool)                     \
  __func(has_set_more_than_1_r, bool)


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
    return os << "'{"
              << "has_set_0=" << res.has_set_0()
              << "has_set_1=" << res.has_set_1()
              << "has_set_more_than_1=" << res.has_set_more_than_1()
              << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    bool ne{false};
    ne |= (a.has_set_0() != b.has_set_0());
    ne |= (a.has_set_1() != b.has_set_1());
    ne |= (a.has_set_more_than_1() != b.has_set_more_than_1());
    return !ne;
  }
  Expect(bool has_set_0, bool has_set_1, bool has_set_more_than_1)
      : has_set_0_(has_set_0), has_set_1_(has_set_1),
        has_set_more_than_1_(has_set_more_than_1) {}
  bool has_set_0() const { return has_set_0_; }
  bool has_set_1() const { return has_set_1_; }
  bool has_set_more_than_1() const { return has_set_more_than_1_; }
 private:
  bool has_set_0_, has_set_1_, has_set_more_than_1_;
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
    x = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    pass = true;
    x = stim.x();
    t_await_cycles(1);
    set_idle();
  }
  bool out_is_valid() const { return has_vld_r; }
  Expect get_expect() const {
    return Expect{has_set_0_r, has_set_1_r, has_set_more_than_1_r};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {}
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vone_or_two v{"one_or_two"};
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
enum class Mode { Zero, One, MoreThanOne };

TEST(OneOrTwoTest, Basic) {
  
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  tb::Random::Bag<Mode> mbag{};
  mbag.add(Mode::Zero);
  mbag.add(Mode::One);
  mbag.add(Mode::MoreThanOne);
  tb::Random::UniformRandomInterval<word_type> rnd{};
  for (std::size_t i = 0; i < n; i++) {
    word_type w;
    bool has_set_0{false};
    bool has_set_1{false};
    bool has_set_more_than_1{false};
    switch (mbag()) {
      case Mode::Zero:
        has_set_0 = true;
        w = 0;
        break;
      case Mode::One:
        has_set_1 = true;
        w = (1 << (rnd() & 0x1F));
        break;
      case Mode::MoreThanOne:
        has_set_more_than_1 = true;
        w = rnd();
        break;
    }
    task->add_stimulus(Stimulus{w});
    const Expect expect{has_set_0, has_set_1, has_set_more_than_1};
    task->add_expected(expect);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
