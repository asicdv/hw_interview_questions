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
#include "vobj/Vfused_multiply_add.h"

using in_type = uint32_t;
using out_type = uint32_t;

#define PORTS(__func)                           \
  __func(cntrl_load, bool)                      \
  __func(cntrl_init, out_type)                  \
  __func(pass, bool)                            \
  __func(m, in_type)                            \
  __func(x, in_type)                            \
  __func(c, in_type)                            \
  __func(y_valid_r, bool)                       \
  __func(y_w, out_type)


struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{m=" << stim.m() << ",x=" << stim.x() << ",c=" << stim.c() << "}";
  }
  Stimulus(in_type m, in_type x, in_type c) : m_(m), x_(x), c_(c) {}
  in_type m() const { return m_; }
  in_type x() const { return x_; }
  in_type c() const { return c_; }
 private:
  in_type m_, x_, c_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return a.y() == b.y();
  }
  Expect(out_type y) : y_(y) {}
  out_type y() const { return y_; }
 private:
  out_type y_;
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
    m = in_type{};
    x = in_type{};
    c = in_type{};
  }
  bool out_is_valid() const { return y_valid_r; }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    pass = true;
    m = stim.m();
    x = stim.x();
    c = stim.c();
    t_await_cycles(1);
    set_idle();
  }
  Expect get_expect() const { return Expect{y_w}; }
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
  Vfused_multiply_add v{"fused_multiply_add"};
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

TEST(FusedMultiplyAddTest, Basic) {
  const std::size_t n{1024 << 5};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  struct x_constraint : scv_constraint_base {
    scv_smart_ptr<in_type> p;
    SCV_CONSTRAINT_CTOR(x_constraint) {
      SCV_CONSTRAINT((p() >= 0) && (p() < (1 << 8) - 1));
    }
  } m_c("m_constrained"), x_c("x_constrained"), c_c("c_constrained");

  out_type sum{0};
  for (std::size_t i = 0; i < n; i++) {
    const in_type m{*m_c.p};
    const in_type x{*x_c.p};
    const in_type c{*c_c.p};

    task->add_stimulus(Stimulus{m, x, c});

    sum += (m * x) + c;
    task->add_expected(Expect{sum});
    x_c.next();
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
