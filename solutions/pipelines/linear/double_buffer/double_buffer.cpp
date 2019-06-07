//========================================================================== //
// Copyright (c) 2018, Stephen Henry
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
#include "vobj/Vdouble_buffer.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(in, uint32_t)                          \
  __func(in_vld, bool)                          \
  __func(in_accept, bool)                       \
  __func(out_r, uint32_t)                       \
  __func(out_vld_r, bool)                       \
  __func(stall_req, uint32_t)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{a=" << stim.a() << "}";
  }
  Stimulus(word_type a) : a_(a) {}
  word_type a() const { return a_; }
 private:
  word_type a_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.y() == b.y());
  }
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
     bc.clk(clk);
     bc.out(stall_req);
     for (std::size_t b = 0; b < 10; b++)
       bc.set_p(b, 0.05f);
     start_tracing();
   }
   ~TOP() {
     stop_tracing();
   }
  void set_idle() {
    in_vld = false;
    in = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    in_vld = true;
    in = stim.a();
    t_await_advance();
    set_idle();
  }
  bool out_is_valid() const { return out_vld_r; }
  Expect get_expect() const { return Expect{out_r}; }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_await_advance() {
    do { t_await_cycles(); } while (!in_accept);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vdouble_buffer v{"double_buffer"};
  ::tb::MultiRandomBool<word_type, 10> bc{"multi_random_bool"};
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

TEST(DoubleBufferTest, Basic) {
  const std::size_t n{1024 << 5};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  tb::Random::UniformRandomInterval<uint32_t> rnd{};
  for (std::size_t i = 0; i < n; i++) {
    const Stimulus stim{rnd()};
    task->add_stimulus(stim);
    task->add_expected(Expect{stim.a()});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}

