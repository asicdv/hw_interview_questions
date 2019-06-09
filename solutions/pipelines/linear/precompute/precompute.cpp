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
#include "vobj/Vprecompute.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(in_pass, bool)                         \
  __func(in_inst_ra_0, word_type)               \
  __func(in_inst_ra_1, word_type)               \
  __func(in_inst_wa, word_type)                 \
  __func(in_inst_op, word_type)                 \
  __func(in_imm, word_type)                     \
  __func(in_accept, bool)                       \
  __func(out_vld_r, bool)                       \
  __func(out_wa_r, word_type)                   \
  __func(out_wdata_r, word_type)                \
  __func(replay_s4_req, bool)                   \
  __func(replay_s8_req, bool)                   \
  __func(stall_req, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "ra_0=" << stim.ra_0() << ","
              << "ra_1=" << stim.ra_1() << ","
              << "wa=" << stim.wa() << ","
              << "op=" << stim.op() << ","
              << "imm=" << stim.imm()
              << "}"
        ;
  }
  Stimulus(word_type ra_0, word_type ra_1, word_type wa, word_type op, word_type imm)
      : ra_0_(ra_0), ra_1_(ra_1), wa_(wa), op_(op), imm_(imm) {}
  word_type ra_0() const { return ra_0_; }
  word_type ra_1() const { return ra_1_; }
  word_type wa() const { return wa_; }
  word_type op() const { return op_; }
  word_type imm() const { return imm_; }
 private:
  word_type ra_0_, ra_1_, wa_, op_, imm_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & res) {
    return os << "'{wa=" << res.wa() << ",wdata=" << res.wdata() << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.wa() != b.wa())
      return false;
    if (a.wdata() != b.wdata())
      return false;

    return true;
  }
  Expect(word_type wa, word_type wdata) : wa_(wa), wdata_(wdata) {}
  word_type wa() const { return wa_; }
  word_type wdata() const { return wdata_; }
 private:
  word_type wa_, wdata_;
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
     rnd_replay_s4_req.clk(clk);
     rnd_replay_s4_req.out(replay_s4_req);
     rnd_replay_s4_req.set_p(0.05f);
     rnd_replay_s8_req.clk(clk);
     rnd_replay_s8_req.out(replay_s8_req);
     rnd_replay_s8_req.set_p(0.05f);
     start_tracing();
   }
   ~TOP() {
     stop_tracing();
   }
  void set_idle() {
    in_pass = false;
    in_inst_ra_0 = word_type{};
    in_inst_ra_1 = word_type{};
    in_inst_wa = word_type{};
    in_inst_op = word_type{};
    in_imm = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    in_pass = true;
    in_inst_ra_0 = stim.ra_0();
    in_inst_ra_1 = stim.ra_1();
    in_inst_wa = stim.wa();
    in_inst_op = stim.op();
    in_imm = stim.imm();
    t_await_advance();
    set_idle();
  }
  bool out_is_valid() const { return out_vld_r; }
  Expect get_expect() const { return Expect{out_wa_r, out_wdata_r}; }
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
  Vprecompute v{"precompute"};
  ::tb::MultiRandomBool<word_type, 10> bc{"multi_random_bool"};
  ::tb::RandomBool rnd_replay_s4_req{"rnd_replay_s4_req"};
  ::tb::RandomBool rnd_replay_s8_req{"rnd_replay_s8_req"};
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

enum OP : word_type {
  NOP = 0,
  AND = 1,
  NOT = 2,
  OR = 3,
  XOR = 4,
  ADD = 5,
  SUB = 6,
  MOV0 = 7,
  MOV1 = 8,
  MOVI = 9
};

struct MachineModel {
  Expect apply(const Stimulus & i) {
    word_type wdata;
    switch (i.op()) {
      case OP::AND: {
        wdata = (reg[i.ra_0()] & reg[i.ra_1()]);
      } break;
      case OP::NOT: {
        wdata = ~reg[i.ra_0()];
      } break;
      case OP::OR: {
        wdata = (reg[i.ra_0()] | reg[i.ra_1()]);
      } break;
      case OP::XOR: {
        wdata = (reg[i.ra_0()] ^ reg[i.ra_1()]);
      } break;
      case OP::ADD: {
        wdata = (reg[i.ra_0()] + reg[i.ra_1()]);
      } break;
      case OP::SUB: {
        wdata = (reg[i.ra_0()] - reg[i.ra_1()]);
      } break;
      case OP::MOV0: {
        wdata = reg[i.ra_0()];
      } break;
      case OP::MOV1: {
        wdata = reg[i.ra_1()];
      } break;
      case OP::MOVI: {
        wdata = i.imm();
      } break;
    }

    reg[i.wa()] = wdata;
    return Expect{i.wa(), wdata};
  }
  uint32_t reg[32];
};

} // namespace

TEST(DoubleBufferTest, Basic) {
  const std::size_t n{1024 << 5};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  MachineModel mdl{};
  tb::Random::UniformRandomInterval<uint32_t> rnd{};

  // Initialize machine state.
  for (word_type i = 0; i < 32; i++) {
    const Stimulus stim{0, 0, i, OP::MOVI, rnd()};
    const Expect expect{mdl.apply(stim)};
    
    task->add_stimulus(stim);
    task->add_expected(expect);
  }

  // Initialize machine state.
  tb::Random::UniformRandomInterval<uint32_t> rnd_reg{31, 0};

  tb::Random::Bag<OP> opbg;
  opbg.add(OP::NOP);
  opbg.add(OP::AND);
  opbg.add(OP::NOT);
  opbg.add(OP::OR);
  opbg.add(OP::XOR);
  opbg.add(OP::ADD);
  opbg.add(OP::SUB);
  opbg.add(OP::MOV0);
  opbg.add(OP::MOV1);
  opbg.add(OP::MOVI);
  opbg.finalize();

  for (word_type i = 0; i < n; i++) {
    const Stimulus stim{rnd_reg(), rnd_reg(), rnd_reg(), opbg(), rnd()};
    const Expect expect{mdl.apply(stim)};
    
    task->add_stimulus(stim);
    task->add_expected(expect);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
