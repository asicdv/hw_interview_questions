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

#include <libtb2.hpp>
#include <vector>
#include <iostream>
#include <sstream>

#include "vobj/Vprecompute.h"

#define PORTS(__func)                           \
  __func(in_pass, bool)                         \
  __func(in_inst_ra_1, uint32_t)                \
  __func(in_inst_ra_0, uint32_t)                \
  __func(in_inst_wa, uint32_t)                  \
  __func(in_inst_op, uint32_t)                  \
  __func(in_imm, uint32_t)                      \
  __func(in_accept, bool)                       \
  __func(out_vld_r, bool)                       \
  __func(out_wa_r, uint32_t)                    \
  __func(out_wdata_r, uint32_t)                 \
  __func(replay_s4_req, bool)                   \
  __func(replay_s8_req, bool)                   \
  __func(stall_req, uint32_t)

typedef Vprecompute uut_t;

enum OP {
  OP_NOP = 0,
  OP_AND = 1,
  OP_NOT = 2,
  OP_OR = 3,
  OP_XOR = 4,
  OP_ADD = 5,
  OP_SUB = 6,
  OP_MOV0 = 7,
  OP_MOV1 = 8,
  OP_MOVI = 9
};

template<>
struct scv_extensions<OP> : public scv_enum_base<OP> {
  SCV_ENUM_CTOR(OP) {
    SCV_ENUM(OP_AND);
    SCV_ENUM(OP_NOT);
    SCV_ENUM(OP_OR);
    SCV_ENUM(OP_XOR);
    SCV_ENUM(OP_ADD);
    SCV_ENUM(OP_SUB);
    SCV_ENUM(OP_MOV0);
    SCV_ENUM(OP_MOV1);
    SCV_ENUM(OP_MOVI);
  }
};

struct Inst {
  uint32_t op;
  uint32_t ra [2];
  uint32_t wa;
  uint32_t imm;
  std::string to_string() const {
    std::stringstream ss;
    ss << op
       << ra [1]
       << ra [0]
       << wa
       << imm
      ;
    return ss.str();
  }
};



struct Result {
  uint32_t wa;
  uint32_t d;
  std::string to_string() const {
    std::stringstream ss;
    ss << wa
       << d
      ;
    return ss.str();
  }
};

struct Machine {
  Result apply(const Inst & i) {
    Result r;
    r.wa = i.wa;
    switch (i.op) {
    case OP_AND: {
      r.d = (reg[i.ra[0]] & reg[i.ra[1]]);
    } break;
    case OP_NOT: {
      r.d = ~reg[i.ra[0]];
    } break;
    case OP_OR: {
      r.d = (reg[i.ra[0]] | reg[i.ra[1]]);
    } break;
    case OP_XOR: {
      r.d = (reg[i.ra[0]] ^ reg[i.ra[1]]);
    } break;
    case OP_ADD: {
      r.d = (reg[i.ra[0]] + reg[i.ra[1]]);
    } break;
    case OP_SUB: {
      r.d = (reg[i.ra[0]] - reg[i.ra[1]]);
    } break;
    case OP_MOV0: {
      r.d = reg[i.ra[0]];
    } break;
    case OP_MOV1: {
      r.d = reg[i.ra[1]];
    } break;
    case OP_MOVI: {
      r.d = i.imm;
    } break;
    }

    reg[r.wa] = r.d;
    return r;
  }
  uint32_t reg[32];
};

struct CombStallTb : libtb2::Top<uut_t> {
  SC_HAS_PROCESS(CombStallTb);
  CombStallTb(sc_core::sc_module_name mn = "t")
    : uut_("uut")
#define __construct_ports(__name, __type)       \
      , __name ## _(#__name)
    PORTS(__construct_ports)
#undef __construct_ports
    , clk_("clk")
    , rst_("rst")
  {
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    resetter_.clk(clk_);
    resetter_.rst(rst_);
    
    sampler_.clk(clk_);

    generate_stimulus(1024);

    SC_THREAD(t_in);
    
    SC_METHOD(m_out);
    sensitive << sampler_.sample();
    dont_initialize();

    stall_n_ = 0;
    SC_METHOD(m_stall);
    sensitive << clk_.posedge_event();

    SC_THREAD(t_replay);

    register_uut(uut_);
    vcd_on();

    i_ = 0;
    error_ = false;
  }
private:
  void t_replay() {
    scv_smart_ptr<bool> replay;
    scv_bag<bool> replay_bag;
    replay_bag.add(true, 1);
    replay_bag.add(false, 99);
    replay->set_mode(replay_bag);
    
    replay_s8_req_ = false;
    replay_s4_req_ = false;
    while (true) {
      replay->next();
      replay_s8_req_ = *replay;

      replay->next();
      replay_s4_req_ = *replay;
      
      wait(clk_.posedge_event());
      replay_s8_req_ = false;
      replay_s4_req_ = false;
    }
  }
  void m_stall() {
    if ((++stall_n_ % 10) == 0) {
      stall_req_ = (1 << (*stall_ptr_ % 10));

      stall_ptr_->next();
    } else {
      stall_req_ = 0;
    }
  }
  void generate_stimulus(std::size_t n = 10) {

    stimulus_.clear();

    // Initialize machine
    scv_smart_ptr<uint32_t> imm_ptr;
    for (uint32_t i = 0; i < 32; i++, imm_ptr->next()) {
      Inst inst;
      inst.op = OP_MOVI;
      inst.wa = i;
      inst.imm = *imm_ptr;

      expected_.push_back(m_.apply(inst));
      stimulus_.push_back(inst);
    }
    
    struct reg_constraint : scv_constraint_base {
      scv_smart_ptr<uint32_t> p;
      SCV_CONSTRAINT_CTOR(reg_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < 32)); 
      }
    } reg_c("reg_constraint_c");
    
    scv_smart_ptr<OP> op;
    scv_smart_ptr<uint32_t> imm;

    while (n-- != 0) {
      op->next();
      imm->next();

      Inst inst;
      inst.op =  *op; 
      inst.imm =  *imm; 
      inst.ra [0] = *reg_c.p; reg_c.next();
      inst.ra [1] = *reg_c.p; reg_c.next();
      inst.wa = *reg_c.p; reg_c.next();

      expected_.push_back(m_.apply(inst));
      stimulus_.push_back(inst);
    }    
  }
  void t_in() {
    in_pass_ = false;
    resetter_.wait_reset_done();

    wait(100, SC_NS);

    for (std::size_t i = 0; i < stimulus_.size(); i++) {

      do { sampler_.wait_for_sample(); } while (!in_accept_);

      const Inst & inst = stimulus_[i];

      in_pass_ = true;
      in_inst_ra_1_ = inst.ra[1];
      in_inst_ra_0_ = inst.ra[0];
      in_inst_wa_ = inst.wa;
      in_inst_op_ = inst.op;
      in_imm_ = inst.imm;

      wait(clk_.posedge_event());

      if (error_)
        break;
    }
    in_pass_ = false;

    wait(10, SC_NS);
    sc_core::sc_stop();
  }
  void m_out() {
    if (out_vld_r_) {

      const uint32_t out_wa = out_wa_r_;
      const uint32_t out_wdata = out_wdata_r_;

      const Result & r = expected_[i_++];
      if (out_wa != r.wa) {
        error_ = true;

        std::cout << sc_core::sc_time_stamp()
                  << "**** ERROR:"
                  << " expected WA = " << r.wa
                  << " actual WA = " << out_wa
                  << "\n";
      }

      if (out_wdata != r.d) {
        error_ = true;
        
        std::cout << sc_core::sc_time_stamp()
                  << "**** ERROR:"
                  << " expected WDATA = " << r.d
                  << " actual WDATA = " << out_wdata
                  << "\n";
      } 
    }
  }
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  Machine m_;
  std::vector<Inst> stimulus_;
  std::vector<Result> expected_;
  std::size_t i_;
  bool error_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  std::size_t stall_n_;
  scv_smart_ptr<uint32_t> stall_ptr_;
  uut_t uut_;
};
SC_MODULE_EXPORT(CombStallTb);

int sc_main(int argc, char **argv) {
  CombStallTb tb("tb");
  return libtb2::Sim::start(argc, argv);
}
