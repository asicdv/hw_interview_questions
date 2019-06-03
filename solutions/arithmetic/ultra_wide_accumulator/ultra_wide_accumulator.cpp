//========================================================================== //
// copyright (c) 2017, stephen henry
// all rights reserved.
//
// redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * redistributions of source code must retain the above copyright notice, this
//   list of conditions and the following disclaimer.
//
// * redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// this software is provided by the copyright holders and contributors "as is"
// and any express or implied warranties, including, but not limited to, the
// implied warranties of merchantability and fitness for a particular purpose
// are disclaimed. in no event shall the copyright holder or contributors be
// liable for any direct, indirect, incidental, special, exemplary, or
// consequential damages (including, but not limited to, procurement of
// substitute goods or services; loss of use, data, or profits; or business
// interruption) however caused and on any theory of liability, whether in
// contract, strict liability, or tort (including negligence or otherwise)
// arising in any way out of the use of this software, even if advised of the
// possibility of such damage.
//========================================================================== //

#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include "vobj/Vultra_wide_accumulator.h"

typedef sc_dt::sc_bv<128> rtl_word_t;

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(clear, bool)                           \
  __func(x, rtl_word_t)                         \
  __func(y_r, rtl_word_t)                       \
  __func(y_vld_r, bool)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{a=" << stim.x() << "}";
  }
  Stimulus(rtl_word_t x) : x_(x) {}
  rtl_word_t x() const { return x_; }
 private:
  rtl_word_t x_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{e=" << res.y() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    return (a.y() == b.y());
  }
  Expect(rtl_word_t y) : y_(y) {}
  rtl_word_t y() const { return y_; }
 private:
  rtl_word_t y_;
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
    clear = false;
    x = rtl_word_t{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    pass = true;
    x = stim.x();
    t_await_cycles(1);
    set_idle();
  }
  bool out_is_valid() const { return y_vld_r; }
  Expect get_expect() const { return Expect{y_r}; }
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
  Vultra_wide_accumulator v{"ultra_wide_accumulator"};
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

struct Accumulator {
  using bi_t = ::sc_dt::sc_biguint<128>;
  
  static bi_t to_bi(const rtl_word_t & w) {
    return bi_t{w};
  }
  static rtl_word_t to_rtl(const bi_t & b) {
    rtl_word_t r;
    r.range(127, 64) = b.range(127, 64).to_uint64();
    r.range(63, 0) = b.range(63, 0).to_uint64();
    return r;
  }
  Accumulator(const rtl_word_t & w) : bi_(Accumulator::to_bi(w)) {}
  void apply(const rtl_word_t & x) {
    bi_ = bi_ + Accumulator::to_bi(x);
  }
  rtl_word_t w() const { return Accumulator::to_rtl(bi_); }
 private:
  bi_t bi_;
};

TEST(UltraWideAccumulatorTest, Basic) {
  const std::size_t n{1024 << 5};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  Accumulator acc{0};
  tb::Random::UniformRandomInterval<uint32_t> rnd{1024};
  for (std::size_t i = 0; i < n; i++) {
    const uint32_t x{rnd()};
    rtl_word_t w{0};
    w.range(31, 0) = x;
    task->add_stimulus(w);
    acc.apply(w);
    task->add_expected(Expect{acc.w()});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
