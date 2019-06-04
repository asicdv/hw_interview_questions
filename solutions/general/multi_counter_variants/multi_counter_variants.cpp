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
#include <tuple>
#include <unordered_map>
#include "vobj/Vmulti_counter_variants.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(cmd_pass, bool)                       \
  __func(cmd_id, word_type)                    \
  __func(cmd_op, word_type)                    \
  __func(cmd_dat, word_type)                   \
  __func(busy_r, bool)                          \
  __func(s1_pass_r, bool)                       \
  __func(s1_dat_r, word_type)                   \
  __func(s2_pass_r, bool)                       \
  __func(s2_dat_r, word_type)                   \
  __func(s3_pass_r, bool)                       \
  __func(s3_dat_r, word_type)

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
    return os << "'{dat=" << res.dat() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.dat() != b.dat()) return false;
    return true;
  }
  Expect() : is_valid_(false) {}
  Expect(word_type dat) : dat_(dat), is_valid_(true) {}
  bool is_valid() const { return is_valid_; }
  word_type dat() const { return dat_; }
 private:
  bool is_valid_{true};
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
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void set_idle() {
    cmd_pass = false;
    cmd_id = word_type{};
    cmd_op = word_type{};
    cmd_dat = word_type{};
  }
  void t_set_stimulus(const Stimulus & stim) {
    cmd_pass = true;
    cmd_op = static_cast<word_type>(stim.op());
    cmd_id = stim.id();
    cmd_dat = stim.dat();
    t_await_cycles();
  }
  bool out_is_valid(std::size_t i = 0) const {
    switch (i) {
      case 1: return s1_pass_r; break;
      case 2: return s2_pass_r; break;
      case 3: return s3_pass_r; break;
    }
    return false;
  }
  Expect get_expect(std::size_t i = 0) const {
    switch (i) {
      case 1: return Expect{s1_dat_r}; break;
      case 2: return Expect{s2_dat_r}; break;
      case 3: return Expect{s3_dat_r}; break;
    }
    return Expect{};
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
  Vmulti_counter_variants v{"multi_counter_variants"};
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


const std::size_t OP_CNTRS_N{32};
const std::size_t OP_CNTRS_W{32};
const std::size_t N{1024 << 4};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

struct MultiCounterVariantTask : ::tb::BasicPassValidNotBusyTask<TOP> {
  MultiCounterVariantTask(TOP & top)
      : BasicPassValidNotBusyTask<TOP>(top) {}
  void add_expected(const expected_type & expect) override {
    for (std::size_t i = 0; i < 3; i++)
      expected_[i].push(expect);
  }
 private:
  void m_checker() override {
    for (std::size_t i = 0; i < 3; i++) {
      if (top_.out_is_valid(i + 1)) {
        ASSERT_FALSE(expected_[i].empty());

        const expected_type & tb_out{expected_[i].front()};
        const expected_type & rtl_out{top_.get_expect(i + 1)};
        ASSERT_EQ(tb_out, rtl_out) << "["
                                   << ::sc_core::sc_time_stamp()
                                   << "]: Stimulus mismatch!"
                                   << " (id=" << (i + 1) << ")"
                                   << " Expected: " << tb_out
                                   << " Actual: " << rtl_out;
        expected_[i].pop();
      }
    }
  }
  std::queue<expected_type> expected_[3];
};

TEST(MultiCounterTest, MultipleID) {
  const std::size_t ACTIVE_IDS{16};
  auto task = std::make_unique<MultiCounterVariantTask>(top);

  MultiCounterModel model;
  std::vector<word_type> active_ids;
  
  tb::Random::UniformRandomInterval<word_type> rnd_id{OP_CNTRS_N - 1};
  tb::Random::UniformRandomInterval<word_type> rnd_word{1024 << 1};
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
      task->add_expected(Expect{r.second});
  }
  
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted();
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
