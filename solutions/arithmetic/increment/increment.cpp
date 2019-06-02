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
#include "vobj/Vfast_increment.h"

#define PORTS(__func)                           \
  __func(A, T)                                  \
  __func(fail, bool)

typedef Vfast_increment uut_t;

template<typename T>
struct IncrementTb : libtb2::Top<IncrementTb<T> > {
  SC_HAS_PROCESS(IncrementTb);
  IncrementTb(sc_core::sc_module_name mn = "t")
      : uut_("uut") {
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
      wd_.clk(clk_);

    SC_METHOD(m_fail);
    this->sensitive << fail_.posedge_event();
    this->dont_initialize();
    
    SC_THREAD(t_stimulus);
  }
 private:
  void m_fail() { LIBTB2_ERROR_ON(fail_); }
  void t_stimulus() {
    scv_smart_ptr<T> p;
    while (true) {
      wait(clk_.posedge_event());
      p->next();
      A_ = *p;

      LOGGER(INFO) << "A=" << A_ << "\n";
    }
  }
  sc_core::sc_clock clk_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(IncrementTb<uint32_t>);

int sc_main(int argc, char **argv) {
  IncrementTb<uint32_t> tb;
  return libtb2::Sim::start(argc, argv);
}
