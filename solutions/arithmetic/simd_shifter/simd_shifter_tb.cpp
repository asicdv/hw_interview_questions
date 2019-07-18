//========================================================================== //
// Copyright (c) 2019, Stephen Henry
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
#include "vobj/Vsimd_shifter_tb.h"

#define PORTS(__func)                           \
  __func(in, vluint64_t)                        \
  __func(mode, uint32_t)                        \
  __func(op, uint32_t)                          \
  __func(shift, vluint64_t)                     \
  __func(fail, bool)

enum Op { Nop = 0, Sll = 1, Srl = 2, Sla = 4, Sra = 5 };
enum Mode { M1B = 0, M2B = 1, M4B = 2, M8B = 3 };

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
#define __declare_out(__name, __type) << #__name << "=" << stim.__name() << ","
    return os
      PORTS(__declare_out)
      ;
#undef __declare_out
  }
  Stimulus() {}
#define __declare_getters(__name, __type)               \
  __type __name() const { return __name ## _; }         \
  void set_ ## __name(__type t) { __name ## _ = t; }
  PORTS(__declare_getters)
#undef __declare_getters

 private:
#define __declare_fields(__name, __type)        \
  __type __name ## _;
  PORTS(__declare_fields)
#undef __declare_fields
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;

  TOP() {
    v.clk(clk);
    v.rst(rst);
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
    op = Op::Nop;
  }
  void t_set_stimulus(const stimulus_type & stim) {
    in = stim.in();
    mode = stim.mode();
    op = stim.op();
    shift = stim.shift();
    t_await_cycles(1);
    set_idle();
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  bool is_fail() const { return fail; }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vsimd_shifter_tb v{"simd_shifter_tb"};
  ::sc_core::sc_clock clk{"clk"};
  ::sc_core::sc_signal<bool> rst{"rst"};
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

TEST(SimdShifterTb, Basic) {
  const std::size_t n{1024 << 0};
  auto task = std::make_unique<tb::BasicNotFailTask<TOP> >(top);

  tb::Random::Bag<Mode> rnd_mode{};
  rnd_mode.add(Mode::M8B);
  //  rnd_mode.add(Mode::M4B);
  //  rnd_mode.add(Mode::M2B);
  //  rnd_mode.add(Mode::M1B);
  rnd_mode.finalize();

  tb::Random::Bag<Op> rnd_op{};
  rnd_op.add(Op::Nop);
  rnd_op.add(Op::Sll);
  rnd_op.add(Op::Srl);
  rnd_op.add(Op::Sla);
  rnd_op.add(Op::Sra);
  rnd_op.finalize();

  tb::Random::UniformRandomInterval<vluint64_t> rnd_in;
  
  for (std::size_t i = 0; i < n; i++) {
    Stimulus s{};
    s.set_in(rnd_in());
    s.set_mode(rnd_mode());
    s.set_op(rnd_op());
    switch (s.mode()) {
    case M1B: {
      tb::Random::UniformRandomInterval<uint32_t> rnd_shift(7, 0);
      uint32_t shift = 0;
      for (std::size_t i = 0; i < 8; i++)
        tb::set_range_bits(shift, rnd_shift(), i * 6, 6);
    } break;
    case M2B: {
      tb::Random::UniformRandomInterval<uint32_t> rnd_shift(15, 0);
      uint32_t shift = 0;
      for (std::size_t i = 0; i < 4; i++)
        tb::set_range_bits(shift, rnd_shift(), i * 6, 6);
    } break;
    case M4B: {
      tb::Random::UniformRandomInterval<uint32_t> rnd_shift(31, 0);
      uint32_t shift = 0;
      for (std::size_t i = 0; i < 2; i++)
        tb::set_range_bits(shift, rnd_shift(), i * 6, 6);
    } break;
    case M8B: {
      tb::Random::UniformRandomInterval<uint32_t> rnd_shift(63, 0);
      s.set_shift(rnd_shift());
    } break;
    }
    task->add_stimulus(s);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

