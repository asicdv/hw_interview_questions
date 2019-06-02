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
#include <map>
#include <deque>
#include <utility>
#include "vobj/Vmulti_counter.h"
typedef Vmulti_counter uut_t;

#define PORTS(__func)                           \
  __func(cntr_pass, bool)                       \
  __func(cntr_id, uint32_t)                     \
  __func(cntr_op, uint32_t)                     \
  __func(cntr_dat, uint32_t)                    \
  __func(status_pass_r, bool)                   \
  __func(status_qry_r, bool)                    \
  __func(status_id_r, uint32_t)                 \
  __func(status_dat_r, uint32_t)

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

const std::size_t CNTRS_N = 256;

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

struct Cmd {
  OP op;
  uint32_t id;
  uint32_t dat;
};

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
  sc_core::sc_out<bool> cntr_pass;
  sc_core::sc_out<uint32_t> cntr_id;
  sc_core::sc_out<uint32_t> cntr_op;
  sc_core::sc_out<uint32_t> cntr_dat;

  CmdTransactor(sc_core::sc_module_name mn = "CmdXActor")
      : sc_core::sc_module(mn) {}
  
  void issue(const Cmd & c) {
    cntr_pass = true;
    cntr_id = c.id;
    cntr_op = c.op;
    cntr_dat = c.dat;
    wait(clk.posedge_event());
    cntr_pass = false;
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

struct MultiCounterTb : libtb2::Top<MultiCounterTb> {
  sc_core::sc_port<CmdTransactorIntf> cmdintf;

  SC_HAS_PROCESS(MultiCounterTb);
  MultiCounterTb(sc_core::sc_module_name mn = "t")
      : uut_("uut"), cmdxactor_("CmdXActor") {
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
    cmdxactor_.cntr_pass(cntr_pass_);
    cmdxactor_.cntr_id(cntr_id_);
    cmdxactor_.cntr_op(cntr_op_);
    cmdxactor_.cntr_dat(cntr_dat_);
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
private:
  void m_checker() {
    if (status_qry_r_) {
      const std::pair<uint32_t, uint32_t> e = expected_.front();
      expected_.pop_front();
      
      const uint32_t actual_id = status_id_r_;
      const uint32_t expected_id = e.first;
      LIBTB2_ERROR_ON(actual_id != expected_id);

      const uint32_t actual = status_dat_r_;
      const uint32_t expected = e.second;

      LOGGER(INFO) << " Expected = " << expected
                   << " Actual = " << actual
                   << "\n";
      LIBTB2_ERROR_ON(actual != expected);
    }
  }
  void t_stimulus() {
    struct cmd_constraints : scv_constraint_base {
      scv_smart_ptr<Cmd> c;
      SCV_CONSTRAINT_CTOR(cmd_constraints) {
        SCV_CONSTRAINT((c->id() >= 0) && (c->id() < 256));
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
        expected_.push_back(std::make_pair(cmd.id, mdl_.get(cmd.id)));
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
  std::deque<std::pair<uint32_t, uint32_t> > expected_;
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
SC_MODULE_EXPORT(MultiCounterTb);

int sc_main(int argc, char ** argv) {
  MultiCounterTb tb;
  return libtb2::Sim::start(argc, argv);
}
