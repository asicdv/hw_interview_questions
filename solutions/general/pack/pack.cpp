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
#include "vobj/Vpack.h"

using word_type = ::sc_dt::sc_bv<256>;
using valid_type = uint32_t;

#define PORTS(__func)                           \
  __func(in_pass, bool)                         \
  __func(in_w, word_type)                       \
  __func(in_vld_w, valid_type)                  \
  __func(out_pass_r, bool)                      \
  __func(out_r, word_type)                      \
  __func(out_vld_r, valid_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "w=" << stim.w().to_string(SC_HEX) << " "
              << "vld=" << stim.vld()
              << "}";
  }
  Stimulus(word_type w, valid_type vld) : w_(w), vld_(vld) {}
  word_type w() const { return w_; }
  valid_type vld() const { return vld_; }
 private:
  word_type w_;
  valid_type vld_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{w=" << res.w().to_string(SC_HEX) << ", vld=" << res.vld() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.vld() != b.vld()) return false;
    valid_type vld{a.vld()};
    for (std::size_t i = 0; vld != 0; i++, vld >>= 1) {
      const word_type & a_w{a.w()};
      const word_type & b_w{b.w()};

      if (a_w.get_word(i) != b_w.get_word(i))
        return false;
    }
    return true;
  }
  Expect() : is_valid_(false) {}
  Expect(word_type w, valid_type vld) : w_(w), vld_(vld), is_valid_(true) {}
  bool is_valid() const { return is_valid_; }
  word_type w() const { return w_; }
  valid_type vld() const { return vld_; }
 private:
  bool is_valid_{true};
  word_type w_;
  valid_type vld_;
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
    in_pass = false;
    in_w = word_type{};
    in_vld_w = valid_type{};
  }
  void t_set_stimulus(const Stimulus & stim) {
    in_pass = true;
    in_w = stim.w();
    in_vld_w = stim.vld();
    t_await_cycles();
    set_idle();
  }
  bool out_is_valid() const { return out_pass_r; }
  Expect get_expect() const { return Expect{out_r, out_vld_r}; };
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vpack v{"pack"};
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

template<typename RND>
Stimulus generate_stimulus(RND && rnd) {
  word_type w{0};
  for (std::size_t n = 0; n < 8; n++)
    w.set_word(n, rnd());
  const valid_type vld{rnd() & 0xFF};
  return Stimulus{w, vld};
}

Expect generate_expect(const Stimulus & stim) {
  word_type w{0};
  valid_type vld{0};
  for (std::size_t w_in = 0, w_out = 0; w_in < 8; w_in++) {
    if (tb::bit(stim.vld(), w_in)) {
      w.set_word(w_out, stim.w().get_word(w_in));
      tb::set_bit(vld, w_out);
      ++w_out;
    }
  }
  return Expect{w, vld};
}

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(PackTest, Basic) {
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  tb::Random::UniformRandomInterval<uint32_t> rnd{};
  for (std::size_t i = 0; i < n; i++) {
    const Stimulus stim{generate_stimulus(rnd)};
    task->add_stimulus(stim);
    task->add_expected(generate_expect(stim));
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
