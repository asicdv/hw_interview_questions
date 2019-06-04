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
#include <tuple>
#include <unordered_map>
#include "vobj/Vmulti_counter.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(cntr_pass, bool)                       \
  __func(cntr_id, word_type)                    \
  __func(cntr_op, word_type)                    \
  __func(cntr_dat, word_type)                   \
  __func(status_pass_r, bool)                   \
  __func(status_qry_r, bool)                    \
  __func(status_id_r, word_type)                \
  __func(status_dat_r, word_type)

enum class OP : word_type {
  NOP  = 0x00,
  INIT = 0x04,
  INCR = 0x0C,
  DECR = 0x0D,
  QRY  = 0x18,
};

std::ostream & operator<<(std::ostream & os, const OP & op) {
  switch (op) {
    case OP::NOP: os << "NOP"; break;
    case OP::INIT: os << "INIT"; break;
    case OP::INCR: os << "INCR"; break;
    case OP::DECR: os << "DECR"; break;
    case OP::QRY: os << "QRY"; break;
  }
  return os;
}

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "op=" <<stim.op()
              << " , id=" << stim.id()
              << " , dat=" << stim.dat()
              << "}"
        ;
  }
  Stimulus(OP op, word_type id, word_type dat = 0)
      : op_(op), id_(id), dat_(dat) {}
  OP op() const { return op_; }
  word_type id() const { return id_; }
  word_type dat() const { return dat_; }
 private:
  OP op_{OP::NOP};
  word_type id_, dat_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{"
              << "id=" << res.id()
              << " , dat=" << res.dat()
              << "}"
        ;
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.id() != b.id()) return false;
    if (a.dat() != b.dat()) return false;
    return true;
  }
  Expect(word_type id, word_type dat) : id_(id), dat_(dat) {}
  word_type id() const { return id_; }
  word_type dat() const { return dat_; }
 private:
  word_type dat_, id_;
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
    cntr_pass = false;
    cntr_id = word_type{};
    cntr_op = word_type{};
    cntr_dat = word_type{};
  }
  void t_set_stimulus(const Stimulus & stim) {
    cntr_pass = true;
    cntr_op = static_cast<word_type>(stim.op());
    cntr_id = stim.id();
    cntr_dat = stim.dat();
    t_await_cycles();
  }
  bool out_is_valid() const {
    return status_pass_r && status_qry_r;
  }
  Expect get_expect() const {
    return Expect{status_id_r, status_dat_r};
  }
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
  Vmulti_counter v{"multi_counter"};
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

// Behavioral model of the RTL.
//
struct MultiCounterModel {
  using id_type = word_type;
  using value_type = word_type;

  std::pair<bool, word_type> apply(const Stimulus & stim) {
    switch (stim.op()) {
      case OP::INIT:
        mem_[stim.id()] = stim.dat();
        break;
      case OP::INCR:
        ++mem_[stim.id()];
        break;
      case OP::DECR:
        --mem_[stim.id()];
        break;
      case OP::QRY:
        return std::make_pair(true, mem_[stim.id()]);
        break;
      case OP::NOP:
      default:
        return std::make_pair(false, 0);
    }
    return {false, 0};
  }
 private:
  std::unordered_map<id_type, value_type> mem_;
};


const std::size_t OP_CNTRS_N{256};
const std::size_t OP_CNTRS_W{32};
const std::size_t N{1024 << 4};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(MultiCounterTest, MultipleID) {
  const std::size_t ACTIVE_IDS{16};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  MultiCounterModel model;
  std::vector<word_type> active_ids;
  
  tb::Random::UniformRandomInterval<word_type> rnd_id{OP_CNTRS_N - 1};
  tb::Random::UniformRandomInterval<word_type> rnd_word{};
  for (std::size_t i = 0; i < ACTIVE_IDS; i++) {
    const word_type id{rnd_id()};
    const word_type init{rnd_word()};

    active_ids.push_back(id);
    const Stimulus stimulus{OP::INIT, id, init};
    task->add_stimulus(stimulus);
    auto r{model.apply(stimulus)};
    ASSERT_TRUE(!r.first);
  }
  tb::Random::Bag<OP> opbag;
  opbag.add(OP::INCR);
  opbag.add(OP::DECR);
  tb::Random::Bag<word_type> idbag;
  for (word_type id : active_ids)
    idbag.add(id);
  for (std::size_t i = 0; i < N; i++) {
    const OP op{opbag()};
    const word_type id{idbag()};

    const Stimulus stimulus{op, id};
    task->add_stimulus(stimulus);
    auto r{model.apply(stimulus)};
    ASSERT_TRUE(!r.first);
  }
  for (word_type id : active_ids) {
    const Stimulus stimulus{OP::QRY, id};
    task->add_stimulus(stimulus);
    auto r{model.apply(stimulus)};
    ASSERT_TRUE(r.first);
    if (r.first)
      task->add_expected(Expect{id, r.second});
  }
  
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted();
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
