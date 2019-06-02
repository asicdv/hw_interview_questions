//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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

#include "vobj/Vusing_full_adders.h"

#define PORTS(__func)                           \
    __func(x, uint32_t)                         \
    __func(fail, bool)

typedef Vusing_full_adders uut_t;

template<typename T>
struct UsingFullAddersTb : libtb2::Top<UsingFullAddersTb<T> > {
  SC_HAS_PROCESS(UsingFullAddersTb);
  UsingFullAddersTb(sc_core::sc_module_name mn = "t") : uut_("uut") {
    //
    wd_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    SC_THREAD(t_stimulus);
    SC_METHOD(m_fail);
    this->sensitive << sampler_.sample();
    this->dont_initialize();
  }
private:
  void m_fail() {
    LIBTB2_ERROR_ON(fail_);
  }
  void t_stimulus() {
    const libtb2::Options & o = libtb2::Sim::get_options();
    resetter_.wait_reset_done();
    scv_smart_ptr<T> x;
    while (true) {
      x->next();
      x_ = *x;
      if (o.debug_on()) {
        LOGGER(DEBUG) << "X=" << x_ << "\n";
      }
      wait(clk_.posedge_event());
    }
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};

int sc_main(int argc, char **argv) {
  UsingFullAddersTb<uint32_t> tb;
  return libtb2::Sim::start(argc, argv);
}
