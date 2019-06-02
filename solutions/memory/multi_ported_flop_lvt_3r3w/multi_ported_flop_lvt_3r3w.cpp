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
#include <vector>
#include <list>
#include <algorithm>
#include <map>

#include "vobj/Vmulti_ported_flop_lvt_3r3w.h"

#define PORTS(__func)                           \
  __func(ren0, bool)                            \
  __func(raddr0, addr_t)                        \
  __func(rdata0, data_t)                        \
  __func(ren1, bool)                            \
  __func(raddr1, addr_t)                        \
  __func(rdata1, data_t)                        \
  __func(ren2, bool)                            \
  __func(raddr2, addr_t)                        \
  __func(rdata2, data_t)                        \
  __func(wen0, bool)                            \
  __func(waddr0, addr_t)                        \
  __func(wdata0, data_t)                        \
  __func(wen1, bool)                            \
  __func(waddr1, addr_t)                        \
  __func(wdata1, data_t)                        \
  __func(wen2, bool)                            \
  __func(waddr2, addr_t)                        \
  __func(wdata2, data_t)                        \
  __func(init, bool)                            \
  __func(busy_w, bool)

using uut_t = Vmulti_ported_flop_lvt_3r3w;
using addr_t = uint32_t;
using data_t = uint32_t;

namespace opts {
  static const int N = 1024;
}

struct WriterAndReader : sc_core::sc_module {
  //
  sc_core::sc_in<bool> clk;

  //
  sc_core::sc_out<bool> wen;
  sc_core::sc_out<addr_t> waddr;
  sc_core::sc_out<data_t> wdata;

  //
  sc_core::sc_out<bool> re;
  sc_core::sc_out<addr_t> raddr;
  sc_core::sc_in<data_t> rdata;

  SC_HAS_PROCESS(WriterAndReader);
  WriterAndReader(sc_core::sc_module_name mn = "war", int N = 1000)
    : N_(N), enable_(false) {
    SC_THREAD(t_main);
  }
  void start_of_simulation() {
    LOGGER(INFO) << "Xactor starts with addr count = "
                 << addrs_.size() << "\n";
  }
  void push_back(addr_t a) { addrs_.push_back(a); }
  void set_enable(bool enable = true) { enable_ = enable; }
private:
  void t_main() {
    if (!enable_)
      return ;
    
    wait(100, SC_NS);
    wait(clk.posedge_event());
    
    // Generate initial state
    scv_smart_ptr<data_t> d;
    for (std::size_t i = 0; i < addrs_.size(); i++) {
      t_write(addrs_[i], *d);
      d->next();
    }

    scv_smart_ptr<bool> rnw_c;
    struct addrs_constraint : scv_constraint_base {
      scv_smart_ptr<int> d;
      SCV_CONSTRAINT_CTOR(addrs_constraint) {
        SCV_CONSTRAINT((d()>=0) && (d() < 1024));
      }
    } addrs_constraint_c("addrs_constraint_c");

    int i;
    while (N_--) {
      rnw_c->next();

      do {
        i = *addrs_constraint_c.d;
        addrs_constraint_c.next();
      } while (i >= addrs_.size());

      const bool rnw = *rnw_c;
      const addr_t a = addrs_[i];
      if (rnw) {
        const data_t data = t_read(a);
        if (mem_[a] != data) {
          LOGGER(ERROR) << "Mismatch on read. "
                        << " Expected: 0x" << std::hex << mem_[a]
                        << " Actual: 0x" << data
                        << "\n";
        }
      } else {
          t_write(a, *d);
          d->next();
      }
    }
  }
  void t_write_idle() {
    wen = false;
    waddr = 0;
    wdata = 0;
  }
  void t_write(addr_t a, data_t d) {
    wen = true;
    waddr = a;
    wdata = d;
    wait(clk.posedge_event());
    mem_[a] = d;
    t_write_idle();
  }
  void t_read_idle() {
    re = false;
    raddr = 0;
  }
  data_t t_read(addr_t a) {
    re = true;
    raddr = a;
    wait(clk.posedge_event());
    wait(1, SC_PS);
    t_read_idle();
    return rdata;
  }
  std::vector<addr_t> addrs_;
  std::map<addr_t, data_t> mem_;
  int N_;
  bool enable_;
};

struct Tb : libtb2::Top<uut_t> {
  SC_HAS_PROCESS(Tb);
  Tb(sc_core::sc_module_name mn = "t")
    : uut_("uut")
#define __declare_ports(__name, __type)         \
      , __name ## _(#__name)
      PORTS(__declare_ports)
#undef __declare_ports
  {
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    wd_.clk(clk_);

    //
    war_[0].clk(clk_);
    //
    war_[0].wen(wen0_);
    war_[0].waddr(waddr0_);
    war_[0].wdata(wdata0_);
    //
    war_[0].re(ren0_);
    war_[0].raddr(raddr0_);
    war_[0].rdata(rdata0_);

    //
    war_[1].clk(clk_);
    //
    war_[1].wen(wen1_);
    war_[1].waddr(waddr1_);
    war_[1].wdata(wdata1_);
    //
    war_[1].re(ren1_);
    war_[1].raddr(raddr1_);
    war_[1].rdata(rdata1_);

    //
    war_[2].clk(clk_);
    //
    war_[2].wen(wen2_);
    war_[2].waddr(waddr2_);
    war_[2].wdata(wdata2_);
    //
    war_[2].re(ren2_);
    war_[2].raddr(raddr2_);
    war_[2].rdata(rdata2_);

    war_[0].set_enable();
    war_[1].set_enable();
    war_[2].set_enable();

    load_stimulus();
    LOGGER(INFO) << "Stimulus loaded!\n";
    
    register_uut(uut_);
    vcd_on();
  }
private:
  void load_stimulus() {
    struct addrs_constraint : scv_constraint_base {
      scv_smart_ptr<std::size_t> d;
      SCV_CONSTRAINT_CTOR(addrs_constraint) {
        SCV_CONSTRAINT((d()>=0) && (d() < opts::N));
      }
    } addrs_constraint_c("addrs_constraint_c");
    struct ports_constraint : scv_constraint_base {
      scv_smart_ptr<std::size_t> d;
      SCV_CONSTRAINT_CTOR(ports_constraint) {
        SCV_CONSTRAINT((d()>=0) && (d() < 3));
      }
    } ports_constraint_c("ports_constraint_c");

    std::list<addr_t> addrs;
    for (addr_t a = 0; a < opts::N; a++) {
      addrs.push_back(a);
    }
    while (addrs.size() != 0ull) {
      ports_constraint_c.next();
      
      std::size_t pos = opts::N + 1;
      while ((pos = *addrs_constraint_c.d) >= addrs.size()) {
        addrs_constraint_c.next();
      }
      std::list<addr_t>::iterator it = addrs.begin();
      std::advance(it, pos);

      war_[*ports_constraint_c.d].push_back(*it);
      addrs.erase(it);
    }
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  uut_t uut_;
  WriterAndReader war_[3];
  libtb2::SimWatchDogCycles wd_;
  libtb2::Resetter resetter_;
};
SC_MODULE_EXPORT(Tb);

int sc_main(int argc, char **argv) {
  Tb tb;
  return libtb2::Sim::start(argc, argv);
}
