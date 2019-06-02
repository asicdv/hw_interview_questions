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
#include <map>
#include <deque>
#include <vector>
#include <sstream>

#include "vobj/Vrmw_long_latency.h"
typedef Vrmw_long_latency uut_t;

#define PORTS(__func)                           \
  __func(iss_vld_r, bool)                       \
  __func(iss_id_r, uint32_t)                    \
  __func(iss_op_r, uint32_t)                    \
  __func(iss_imm_r, uint32_t)                   \
  __func(iss_rdy_w, bool)                       \
  __func(cmpl_vld_r, bool)                      \
  __func(cmpl_word_r, uint32_t)

#define PORTS_TABLE(__func)                     \
  __func(tbl_wr_r, bool)                        \
  __func(tbl_wr_id_r, uint32_t)                 \
  __func(tbl_wr_word_r, uint32_t)               \
  __func(tbl_rd_word_vld_r, bool)               \
  __func(tbl_rd_word_r, uint32_t)               \
  __func(tbl_rd_ctag_r, uint32_t)               \
  __func(tbl_rd_r, bool)                        \
  __func(tbl_rd_id_r, uint32_t)                 \
  __func(tbl_rd_itag_r, uint32_t)

template<typename Key, typename Value>
struct TblMem : sc_core::sc_module {
  //
  sc_core::sc_in<bool> clk;
  sc_core::sc_in<bool> rst;
  //
  sc_core::sc_in<bool> tbl_wr_r;
  sc_core::sc_in<uint32_t> tbl_wr_id_r;
  sc_core::sc_in<uint32_t> tbl_wr_word_r;
  
  //
  sc_core::sc_in<bool> tbl_rd_r;
  sc_core::sc_in<uint32_t> tbl_rd_id_r;
  sc_core::sc_in<uint32_t> tbl_rd_itag_r;
  //
  sc_core::sc_out<bool> tbl_rd_word_vld_r;
  sc_core::sc_out<uint32_t> tbl_rd_word_r;
  sc_core::sc_out<uint32_t> tbl_rd_ctag_r;
  
  SC_HAS_PROCESS(TblMem);
  TblMem(sc_core::sc_module_name mn = "TblMem", std::size_t latency = 16) :
    sc_core::sc_module(mn)
#define __construct_ports(__name, __type)       \
    , __name(#__name)
    PORTS_TABLE(__construct_ports)
#undef __construct_ports
    , clk("clk")
    , rst("rst")
    , cycle_(0)
    , latency_(latency)
  {
    SC_METHOD(m_on_cycle);
    sensitive << clk.pos();
    dont_initialize();
  }
  // Backdoor access
  Value get(const Key & k) const { return m_[k]; }
  void set(const Key & k, const Value & v) { m_[k] = v; }
  bool is_has_key(const Key & k) const { return (m_.count(k) != 0); }
private:
  struct Command {
    enum { WRITE, READ } cmd;
    Key address;
    Value data;
    std::size_t exe_time;
    std::size_t tag;
  };
  
  void m_on_cycle() {
    ++cycle_;

    if (tbl_wr_r) {
      Command c;
      c.cmd = Command::WRITE;
      c.address = tbl_wr_id_r;
      c.data = tbl_wr_word_r;
      c.exe_time = cycle_ + latency_;
      cmds_.push_back(c);
    }
    if (tbl_rd_r) {
      Command c;
      c.cmd = Command::READ;
      c.address = tbl_rd_id_r;
      c.exe_time = cycle_ + latency_;
      c.tag = tbl_rd_itag_r;
      cmds_.push_back(c);
    }

    rd_cmpl_idle();
    if (!cmds_.empty()) {
      const Command & head = cmds_.front();
      if (head.exe_time <= cycle_) {
        handle(head);
        cmds_.pop_front();
      }
    }
  }
  void handle(const Command & c) {
    switch (c.cmd) {
    case Command::WRITE: {
      m_[c.address] = c.data;
    } break;
    case Command::READ: {
      rd_cmpl(m_[c.address], c.tag);
    } break;
    default:
      // Unknown command
      ;
    }
  }
  void rd_cmpl_idle() {
    tbl_rd_word_vld_r = false;
  }
  void rd_cmpl(Value value, std::size_t tag) {
    tbl_rd_word_vld_r = true;
    tbl_rd_word_r = value;
    tbl_rd_ctag_r = tag;
  }
  std::map<Key, Value> m_;
  std::deque<Command> cmds_;
  std::size_t cycle_;
  std::size_t latency_;
};

struct Issue {
  enum { OP_NOP = 0, OP_ADDI = 1, OP_SUBI = 2, OP_MOVI = 3 };
    
  uint32_t op;
  uint32_t imm;
  uint32_t id;
  std::string to_string() const {
    std::stringstream ss;
    ss << "OP: " << op
       << " IMM: 0x" << std::hex << imm
       << " ID:" << std::dec << id;
    return ss.str();
  }
};
std::ostream & operator<<(std::ostream & os, const Issue & issue) {
  return os << issue.to_string();
}

template<typename Addr, typename Data>
struct MachineModel {
  void set(const Addr & addr, const Data & data) { mem_[addr] = data; }
  Data apply(const Issue & issue) {
    switch (issue.op) {
    case Issue::OP_ADDI: {
      mem_[issue.id] += issue.imm;
    } break;
    case Issue::OP_SUBI: {
      mem_[issue.id] -= issue.imm;
    } break;
    case Issue::OP_MOVI: {
      mem_[issue.id] = issue.imm;
    }
    }
    return mem_[issue.id];
  }
private:
  std::map<Addr, Data> mem_;
};

struct TbOptions {
  TbOptions()
    : id_n(1), n(1)
  {}
  std::size_t id_n;
  std::size_t n;
};

struct RmwLongLatencyTb : libtb2::Top<uut_t> {
  SC_HAS_PROCESS(RmwLongLatencyTb);
  RmwLongLatencyTb(const TbOptions opts = TbOptions(),
                   sc_core::sc_module_name mn = "t")
    : uut_("uut")
#define __construct_ports(__name, __type)       \
      , __name ## _(#__name)
      PORTS(__construct_ports)
      PORTS_TABLE(__construct_ports)
#undef __construct_ports
    , clk_("clk")
    , rst_("rst")
    , opts_(opts)
  {
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
    PORTS_TABLE(__bind_signals)
#undef __bind_signals

    tbl_.clk(clk_);
    tbl_.rst(rst_);
#define __bind_signals(__name, __type)          \
    tbl_.__name(__name ## _);
    PORTS_TABLE(__bind_signals)
#undef __bind_signals

    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);

    initialize_stimulus(opts_);

    SC_THREAD(t_run);

    SC_METHOD(m_check_completion);
    sensitive << clk_.posedge_event();
    dont_initialize();

    register_uut(uut_);
    vcd_on();
  }
private:
  void initialize_stimulus(TbOptions opts) {
    scv_smart_ptr<uint32_t> u32;
    while (opts.id_n--) {
      valid_id_.push_back(*u32);
      u32->next();
    }

    scv_smart_ptr<uint32_t> cmd;
    scv_bag<uint32_t> cmd_bag;
    cmd_bag.add(Issue::OP_NOP, 10);
    cmd_bag.add(Issue::OP_ADDI, 10);
    cmd_bag.add(Issue::OP_SUBI, 10);
    cmd_bag.add(Issue::OP_MOVI, 10);
    cmd->set_mode(cmd_bag);

    struct imm_constraint : scv_constraint_base {
      scv_smart_ptr<uint32_t> p;
      SCV_CONSTRAINT_CTOR(imm_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < 128));
      }
    } imm_c("imm_constraint_c");

    scv_smart_ptr<uint32_t> idptr;
    idptr->keep_only(0, valid_id_.size() - 1);

    while (opts.n--) {
      Issue iss;
      iss.op = *cmd;
      iss.imm = *imm_c.p;
      iss.id = *idptr;
      stimulus_.push_back(iss);

      // Advance
      cmd->next();
      idptr->next();
      imm_c.next();
    }
  }
  void t_run() {
    scv_smart_ptr<uint32_t> initptr;

    resetter_.wait_reset_done();

    wait_cycles(10);
    
    // Initialize machine state
    for (std::size_t i = 0; i < valid_id_.size(); i++, initptr->next()) {
      Issue issue;
      issue.op = Issue::OP_MOVI;
      issue.imm = *initptr;
      issue.id = i;

      rtl_issue(issue);
    }

    wait_cycles(100);

    // Run random stimulus
    for (std::size_t i = 0; i < stimulus_.size(); i++)
      rtl_issue(stimulus_[i]);

    wait_cycles(10);
    
    // Done
    sc_core::sc_stop();
  }
  void m_check_completion() {
    if (cmpl_vld_r_) {
      const uint32_t expected = expected_.front();
      const uint32_t actual = cmpl_word_r_;

      if (actual != expected) {
        LOGGER(ERROR) << "Mismatch on: actual = 0x" << std::hex << actual
                      << " expected = 0x" << std::hex << expected
                      << "\n";
      }
      expected_.pop_front();
    }
  }
  void rtl_issue(const Issue & issue) {
    iss_vld_r_ = true;
    iss_op_r_ = issue.op;
    iss_id_r_ = issue.id;
    iss_imm_r_ = issue.imm;
    do { wait_cycles(); } while (!iss_rdy_w_);
    LOGGER(DEBUG) << "Issue: " << issue << "\n";
    expected_.push_back(mm_.apply(issue));
    iss_vld_r_ = false;

    wait_cycles(100);
  }
  void wait_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk_.negedge_event());
  }
  std::vector<uint32_t> valid_id_;
  std::vector<Issue> stimulus_;
  std::deque<uint32_t> expected_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
  PORTS_TABLE(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  MachineModel<uint32_t, uint32_t> mm_;
  TblMem<uint32_t, uint32_t> tbl_;
  TbOptions opts_;
  uut_t uut_;
};
SC_MODULE_EXPORT(RmwLongLatencyTb);

int sc_main(int argc, char **argv) {
  TbOptions opts;
  opts.id_n = 1;
  opts.n = 100;
  RmwLongLatencyTb tb(opts);
  return libtb2::Sim::start(argc, argv);
}

