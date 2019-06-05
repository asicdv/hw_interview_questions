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
#include "vobj/Vzero_indices_fast.h"

using in_type  = sc_bv<128>;
using out_type = uint32_t;

#define PORTS(__func)                           \
  __func(in_vector, sc_bv<128>)                 \
  __func(in_start, bool)                        \
  __func(in_busy_r, bool)                       \
  __func(resp_valid_r, bool)                    \
  __func(resp_index_r, uint32_t)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "w=" <<stim.w()
              << "}";
  }
  Stimulus(in_type w) : w_(w) {}
  in_type w() const { return w_; }
 private:
  in_type w_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{idx=" << res.idx() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.idx() != b.idx()) return false;
    return true;
  }
  Expect() : is_valid_(false) {}
  Expect(out_type idx) : idx_(idx), is_valid_(true) {}
  bool is_valid() const { return is_valid_; }
  out_type idx() const { return idx_; }
 private:
  bool is_valid_{true};
  out_type idx_;
};

struct TOP : ::tb::Top {
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
    in_start = false;
    in_vector = in_type{};
  }
  void t_set_stimulus(const Stimulus & stim) {
    t_wait_not_busy();
    in_start = true;
    in_vector = stim.w();
    t_await_cycles();
    set_idle();
  }
  bool out_is_valid() const { return resp_valid_r; }
  Expect get_expect() const { return Expect{resp_index_r}; };
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {
    while (in_busy_r)
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vzero_indices_fast v{"zero_indices_fast"};
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

template<typename RND>
in_type random_in(RND & rnd) {
  in_type in;
  for (std::size_t n = 0; n < 4; n++)
    in.range((n + 1) * 32 - 1, n * 32) = rnd();
  return in;
}

std::vector<out_type> zeros_in_word(in_type w) {
  std::vector<out_type> v;
  for (std::size_t i = 0; i < 128; i++) {
    if (!w.get_bit(i))
      v.push_back(i);
  }
  return v;
}

} // namespace 

TEST(ZeroIndicesFastTest, Basic) {
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  tb::Random::UniformRandomInterval<uint32_t> rnd{};
  for (std::size_t i = 0; i < n; i++) {
    const Stimulus stim{random_in(rnd)};
    task->add_stimulus(stim);
    for (out_type w : zeros_in_word(stim.w()))
      task->add_expected(Expect{w});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
