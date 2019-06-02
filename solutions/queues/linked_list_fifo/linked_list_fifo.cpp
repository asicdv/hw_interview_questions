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
#include "vobj/Vlinked_list_fifo.h"

#define PORTS(__func)                           \
  __func(cmd_pass, bool)                        \
  __func(cmd_push, bool)                        \
  __func(cmd_id, uint32_t)                      \
  __func(cmd_push_data, uint32_t)               \
  __func(cmd_pop_data, uint32_t)                \
  __func(cmd_pop_data_vld_r, bool)              \
  __func(clear, bool)                           \
  __func(full_r, bool)                          \
  __func(empty_r, bool)                         \
  __func(nempty_r, uint32_t)                    \
  __func(busy_r, bool)

typedef Vlinked_list_fifo uut_t;

const int ID_N = 4;
const int PTR_N = 255;

struct CmdXActorIntf : sc_core::sc_interface {
  virtual void idle() = 0;
  virtual void push(uint32_t id, uint32_t data) = 0;
  virtual bool is_full() = 0;
  virtual bool is_empty() = 0;
  virtual uint32_t pop(uint32_t id) = 0;
  virtual uint32_t id()  = 0;
  virtual uint32_t non_empty_id() = 0;
};

struct CmdXActor : CmdXActorIntf, sc_core::sc_module {
  //
  sc_core::sc_in<bool> clk;
  sc_core::sc_in<bool> rst;
  //
  sc_core::sc_out<bool> cmd_pass;
  sc_core::sc_out<bool> cmd_push;
  sc_core::sc_out<uint32_t> cmd_id;
  sc_core::sc_out<uint32_t> cmd_push_data;
  sc_core::sc_in<uint32_t> cmd_pop_data;
  sc_core::sc_in<bool> cmd_pop_data_vld_r;
  sc_core::sc_out<bool> clear;
  sc_core::sc_in<bool> full_r;
  sc_core::sc_in<bool> empty_r;
  sc_core::sc_in<uint32_t> nempty_r;
  sc_core::sc_in<bool> busy_r;

  CmdXActor(sc_core::sc_module_name mn = "t")
    : clk("clk"), rst("rst")
#define __construct_ports(__name, __type)       \
    , __name(#__name)
    PORTS(__construct_ports)
#undef __construct_ports
  {
    sampler_.clk(clk);
  }
  void idle() {
    cmd_pass = false;
    cmd_push = false;
    cmd_id = 0;
    cmd_push_data = 0;
  }
  bool is_full() {
    sampler_.wait_for_sample();
    return full_r;
  }
  uint32_t non_empty_id() {
    sampler_.wait_for_sample();
    return libtb2::pickone(libtb2::mask_bits(nempty_r.read(), ID_N)) - 1;
  }
  uint32_t id() {
    struct inrange_constraint : scv_constraint_base {
      scv_smart_ptr<uint32_t> p;
      SCV_CONSTRAINT_CTOR(inrange_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < ID_N)); 
      }
    };
    static inrange_constraint c("inrange_constraint");
    c.next();
    return *c.p;
  }
  bool is_empty() {
    sampler_.wait_for_sample();
    return empty_r;
  }
  void push(uint32_t id, uint32_t data) {
    LIBTB2_ERROR_ON(full_r);
    cmd_pass = true;
    cmd_push = true;
    cmd_id = id;
    cmd_push_data = data;
    wait(clk.posedge_event());
    LOGGER(DEBUG) << "Push ID = " << id << " Push Data = " << data << "\n";
    q_[id].push_back(data);
    idle();
  }
  uint32_t pop(uint32_t id) {
    LIBTB2_ERROR_ON(q_[id].empty());
    cmd_pass = true;
    cmd_push = false;
    cmd_id = id;
    wait(clk.posedge_event());
    idle();

    do {
      sampler_.wait_for_sample();
    } while (!cmd_pop_data_vld_r);

    LOGGER(DEBUG) << "Pop ID = " << id << " Pop Data = " << cmd_pop_data << "\n";
    const uint32_t expected = q_[id].front(); q_[id].pop_front();
    const uint32_t actual = cmd_pop_data;
    LIBTB2_ERROR_ON(expected != actual);
    return actual;
  }
private:
  std::deque<uint32_t> q_[ID_N];
  libtb2::Sampler sampler_;
};

enum OP { NOP, PUSH, POP };
template<>
struct scv_extensions<OP> : public scv_enum_base<OP> {
  SCV_ENUM_CTOR(OP) {
    SCV_ENUM(NOP);
    SCV_ENUM(PUSH);
    SCV_ENUM(POP);
  }
};

struct LinkedListFifoTb : libtb2::Top<LinkedListFifoTb> {
  typedef Vlinked_list_fifo uut_t;
  sc_core::sc_port<CmdXActorIntf> xact_;
  SC_HAS_PROCESS(LinkedListFifoTb);
  LinkedListFifoTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);
    //
    xact_.bind(x_);
    //
    x_.clk(clk_);
    x_.rst(rst_);
#define __bind_ports(__name, __type)            \
    x_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    SC_THREAD(t_stimulus);
  }
private:
  
  void t_stimulus() {
    scv_smart_ptr<uint32_t> ppush_data;
    
    xact_->idle();
    resetter_.wait_reset_done();
    while (true) {
      scv_bag<OP> op;
      op.add(NOP, 10);
      if (!xact_->is_full())
        op.add(PUSH, 40);
      if (!xact_->is_empty())
        op.add(POP, 40);

      scv_smart_ptr<OP> iop;
      iop->set_mode(op);
      iop->next();
      switch (*iop) {
      case NOP: {
        wait(clk_.posedge_event());
      } break;
      case PUSH: {
        ppush_data->next();
        xact_->push(xact_->id(), *ppush_data);
      } break;
      case POP: {
        xact_->pop(xact_->non_empty_id());
      } break;

      }
    }
  }
  CmdXActor x_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(LinkedListFifoTb);

int sc_main(int argc, char **argv) {
  LinkedListFifoTb tb;
  return libtb2::Sim::start(argc, argv);
}
