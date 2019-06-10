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
#include "vobj/Vgates_from_MUX2X1.h"

#define PORTS(__func)                           \
    __func(a, bool)                             \
    __func(b, bool)                             \
    __func(fail, bool)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{a=" << stim.a() << ", b=" << stim.b() << "}";
  }
  Stimulus(bool a, bool b) : a_(a), b_(b) {}
  bool a() const { return a_; }
  bool b() const { return b_; }
 private:
  bool a_, b_;
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;

  TOP() {
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
    a = false;
    b = false;
  }
  void t_set_stimulus(const stimulus_type & stim) {
    a = stim.a();
    b = stim.b();
    t_await_cycles(1);
    set_idle();
  }
  bool is_fail() const { return fail; }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vgates_from_MUX2X1 v{"gates_from_MUX2X1"};
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

TEST(GatesFromMux2X1Test, Basic) {
  const std::size_t n{1024 << 10};
  auto task = std::make_unique<tb::BasicNotFailTask<TOP> >(top);

  tb::Random::Bag<bool> rnd{};
  rnd.add(false);
  rnd.add(true);
  rnd.finalize();

  for (std::size_t i = 0; i < n; i++)
    task->add_stimulus(Stimulus{rnd(), rnd()});

  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
