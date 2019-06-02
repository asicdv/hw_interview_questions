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

#include <libtb2.hpp>
#include <deque>
//
#include "vobj/Vmulti_counter_variants.h"

#define PORTS(__func)                           \
    __func(cmd_pass, bool)                      \
    __func(cmd_id, uint32_t)                    \
    __func(cmd_op, uint32_t)                    \
    __func(cmd_dat, uint32_t)                   \
    __func(busy_r, bool)                        \
    __func(s1_pass_r, bool)                     \
    __func(s1_dat_r, uint32_t)                  \
    __func(s2_pass_r, bool)                     \
    __func(s2_dat_r, uint32_t)                  \
    __func(s3_pass_r, bool)                     \
    __func(s3_dat_r, uint32_t)

typedef Vmulti_counter_variants uut_t;

enum OP { OP_NOP = 0x0,
          OP_INIT = 0x04,
          OP_INC = 0x0C,
          OP_DEC = 0x0D,
          OP_QRY = 0x18 };

const char * to_string(OP op) {
  switch (op) {
    case OP_INIT: return "OP_INIT";
    case OP_INC: return "OP_INC";
    case OP_DEC: return "OP_DEC";
    case OP_QRY: return "OP_QRY";
    case OP_NOP:
    default: return "OP_NOP";
  }
}

namespace {

const int CNTRS_N = 32;

} // namespace 

template<>
struct scv_extensions<OP> : scv_enum_base<OP> {
  SCV_ENUM_CTOR(OP) {
    SCV_ENUM(OP_NOP);
    SCV_ENUM(OP_INIT);
    SCV_ENUM(OP_INC);
    SCV_ENUM(OP_DEC);
    SCV_ENUM(OP_QRY);
  }
};

struct Cmd { OP op; uint32_t id; uint32_t dat; };

template<>
struct scv_extensions<Cmd> : scv_extensions_base<Cmd> {
  scv_extensions<OP> op;
  scv_extensions<uint32_t> id, dat;
  SCV_EXTENSIONS_CTOR(Cmd) {
    SCV_FIELD(op);
    SCV_FIELD(id);
    SCV_FIELD(dat);
  }
};

struct CmdTransactorIntf : sc_core::sc_interface {
  virtual void issue(const Cmd & c) = 0;
};

struct CmdTransactor : sc_core::sc_module, CmdTransactorIntf {
  //
  sc_core::sc_in<bool> clk;
  //
  sc_core::sc_out<bool> cmd_pass;
  sc_core::sc_out<uint32_t> cmd_id;
  sc_core::sc_out<uint32_t> cmd_op;
  sc_core::sc_out<uint32_t> cmd_dat;

  CmdTransactor(sc_core::sc_module_name mn = "CmdXActor")
      : sc_core::sc_module(mn) {}
  
  void issue(const Cmd & c) {
    cmd_pass = true;
    cmd_id = c.id;
    cmd_op = c.op;
    cmd_dat = c.dat;
    wait(clk.posedge_event());
    cmd_pass = false;
  }
};

struct MultiCounterMdl {
  void apply(const Cmd & c) {
    switch (c.op) {
    case OP_INIT: {
      ctxts_[c.id] = c.dat;
    } break;
    case OP_INC: {
      ctxts_[c.id]++;
    } break;
    case OP_DEC: {
      ctxts_[c.id]--;
    };
    case OP_NOP:
    case OP_QRY: {
    } break;
    }
  }
  uint32_t get(uint32_t id) {
    return ctxts_[id];
  }
private:
  std::map<uint32_t, uint32_t> ctxts_;
};

struct MultiCounterVariantsTb : libtb2::Top<MultiCounterVariantsTb> {
  sc_core::sc_port<CmdTransactorIntf> cmdintf;

  SC_HAS_PROCESS(MultiCounterVariantsTb);
  MultiCounterVariantsTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    //
    wd_.clk(clk_);
    //
    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    cmdintf.bind(cmdxactor_);
    cmdxactor_.clk(clk_);
    cmdxactor_.cmd_pass(cmd_pass_);
    cmdxactor_.cmd_id(cmd_id_);
    cmdxactor_.cmd_op(cmd_op_);
    cmdxactor_.cmd_dat(cmd_dat_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_THREAD(t_stimulus);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();

    st_.reset();
  }
  void end_of_simulation() {
    LOGGER(INFO) << "Transaction count = " << st_.n << "\n";
  }
private:
  void m_checker() {
    if (s1_pass_r_) {
      LIBTB2_ERROR_ON(expected_[0].empty());
      const uint32_t actual = s1_dat_r_;
      const uint32_t expected = expected_[0].front().second;
      expected_[0].pop_front();
      
      compare_output(1, actual, expected);
    }
    if (s2_pass_r_) {
      LIBTB2_ERROR_ON(expected_[1].empty());
      const uint32_t actual = s2_dat_r_;
      const uint32_t expected = expected_[1].front().second;
      expected_[1].pop_front();
      
      compare_output(2, actual, expected);
    }
    if (s3_pass_r_) {
      LIBTB2_ERROR_ON(expected_[2].empty());
      const uint32_t actual = s3_dat_r_;
      const uint32_t expected = expected_[2].front().second;
      expected_[2].pop_front();
      
      compare_output(3, actual, expected);
    }
  }
  void compare_output(int i, uint32_t actual, uint32_t expected) {
    LOGGER(INFO) << "i = " << i
                 << " actual = " << actual
                 << " expected = " << expected
                 << "\n";
    LIBTB2_ERROR_ON(actual != expected);
  }
  void t_stimulus() {
    struct cmd_constraints : scv_constraint_base {
      scv_smart_ptr<Cmd> c;
      SCV_CONSTRAINT_CTOR(cmd_constraints) {
        SCV_CONSTRAINT((c->id() >= 0) && (c->id() < CNTRS_N));
      }
    } cmd_c("cmd_constraint");

    scv_smart_ptr<uint32_t> dat;
    for (int i = 0; i < CNTRS_N; i++) {
      dat->next();
      
      Cmd cmd;
      cmd.op = OP_INIT;
      cmd.id = i;
      cmd.dat = *dat;
      t_issue(cmd);
    }
    wd_.pat();
    
    while (true) {
      cmd_c.next();
      const Cmd cmd = *cmd_c.c;
      
      t_issue(cmd);
      if (cmd.op == OP_QRY) {
        for (int i = 0; i < 3; i++) {
          expected_[i].push_back(std::make_pair(cmd.id, mdl_.get(cmd.id)));
        }
      }
      st_.n++;
      wait(clk_.posedge_event());
    }
  }
  void t_issue(const Cmd & cmd) {
    LOGGER(INFO) << "Issue cmd id = " << cmd.id
                 << " op = " << to_string(cmd.op)
                 << " dat = " << std::hex << cmd.dat << "\n";

    cmdintf->issue(cmd);
    mdl_.apply(cmd);
  }
  struct {
    void reset() {
      n = 0;
    }
    std::size_t n;
  } st_;
  MultiCounterMdl mdl_;
  std::deque<std::pair<uint32_t, uint32_t> > expected_ [3];
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  CmdTransactor cmdxactor_;
  uut_t uut_;
};
SC_MODULE_EXPORT(MultiCounterVariantsTb);

int sc_main(int argc, char ** argv) {
  MultiCounterVariantsTb tb;
  return libtb2::Sim::start(argc, argv);
}
