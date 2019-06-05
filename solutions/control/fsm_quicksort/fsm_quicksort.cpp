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
#include <algorithm>
#include "vobj/Vfsm_quicksort.h"

using word_type = uint32_t;

#define PORTS_UNSORTED_IN(__func)               \
  __func(unsorted_vld, bool)                    \
  __func(unsorted_sop, bool)                    \
  __func(unsorted_eop, bool)                    \
  __func(unsorted_dat, word_type)

#define PORTS_UNSORTED_OUT(__func)              \
  __func(unsorted_rdy, bool)

#define PORTS_SORTED(__func)                    \
  __func(sorted_vld_r, bool)                    \
  __func(sorted_sop_r, bool)                    \
  __func(sorted_eop_r, bool)                    \
  __func(sorted_err_r, bool)                    \
  __func(sorted_dat_r, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "sop=" << stim.sop() << ","
              << "eop=" << stim.eop() << ","
              << "dat=" << stim.dat()
              << "}";
  }
  Stimulus() : is_valid_(false) {}
  Stimulus(bool sop, bool eop, word_type dat)
      : sop_(sop), eop_(eop), dat_(dat), is_valid_(true) {}
  bool sop() const { return sop_; }
  bool eop() const { return eop_; }
  word_type dat() const { return dat_; }
 private:
  bool sop_, eop_, is_valid_;
  word_type dat_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & x) {
    return os << "'{"
              << "sop=" << x.sop() << ","
              << "eop=" << x.eop() << ","
              << "dat=" << x.dat()
              << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.sop() != b.sop()) return false;
    if (a.eop() != b.eop()) return false;
    if (a.dat() != b.dat()) return false;

    return true;
  }
  Expect() : is_valid_(false) {}
  Expect(bool sop, bool eop, word_type dat)
      : sop_(sop), eop_(eop), dat_(dat), is_valid_(true) {}
  bool sop() const { return sop_; }
  bool eop() const { return eop_; }
  word_type dat() const { return dat_; }
 private:
  bool sop_, eop_, is_valid_;
  word_type dat_;
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;
  using expected_type = Expect;

  TOP() {
    v.rst(rst);
    v.clk(clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS_UNSORTED_IN(__bind_signals)
    PORTS_UNSORTED_OUT(__bind_signals)
    PORTS_SORTED(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void set_idle() {
    unsorted_vld = false;
    unsorted_sop = word_type{};
    unsorted_eop = word_type{};
    unsorted_dat = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    t_wait_not_busy();
    unsorted_vld = true;
    unsorted_sop = stim.sop();
    unsorted_eop = stim.eop();
    unsorted_dat = stim.dat();
    t_await_cycles();
    set_idle();
  }
  bool out_is_valid() const { return sorted_vld_r; }
  Expect get_expect() const {
    return Expect{sorted_sop_r, sorted_eop_r, sorted_dat_r};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {
    while (!unsorted_rdy)
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vfsm_quicksort v{"fsm_quicksort"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
    PORTS_UNSORTED_IN(__declare_signal)
    PORTS_UNSORTED_OUT(__declare_signal)
    PORTS_SORTED(__declare_signal)
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

struct StimulusGenerator {
  StimulusGenerator(std::size_t n = 16) {
    while (n--)
      unsorted_.push_back(rnd());

    sorted_ = unsorted_;
    std::sort(sorted_.begin(), sorted_.end());
  }
  bool get_stimulus(Stimulus & s) {
    if (stim_i_ >= unsorted_.size())
      return false;
    const bool sop{stim_i_ == 0};
    const bool eop{stim_i_ == (unsorted_.size() - 1)};
    s = Stimulus{sop, eop, unsorted_[stim_i_]};
    ++stim_i_;
    return true;
  }
  bool get_expected(Expect & x) {
    if (expect_i_ >= sorted_.size())
      return false;
    const bool sop{expect_i_ == 0};
    const bool eop{expect_i_ == (sorted_.size() - 1)};
    x = Expect{sop, eop, sorted_[expect_i_]};
    ++expect_i_;
    return true;
  }
 private:
  std::size_t stim_i_{0}, expect_i_{0};
  std::vector<word_type> unsorted_, sorted_;
  tb::Random::UniformRandomInterval<word_type> rnd{};
};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(FSMQuicksortTest, Basic) {
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  for (std::size_t i = 0; i < 20; i++) {
    StimulusGenerator gen{16};

    Stimulus stim;
    while (gen.get_stimulus(stim))
      task->add_stimulus(stim);

    Expect x;
    while (gen.get_expected(x))
      task->add_expected(x);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}

