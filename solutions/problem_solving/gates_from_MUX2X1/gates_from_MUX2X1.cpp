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
#include "vobj/Vgates_from_MUX2X1.h"

#define PORTS(__func)                           \
    __func(a, bool)                             \
    __func(b, bool)                             \
    __func(fail, bool)

typedef Vgates_from_MUX2X1 uut_t;

struct GatesFromMUX2X1Tb : libtb2::Top<GatesFromMUX2X1Tb> {
  SC_HAS_PROCESS(GatesFromMUX2X1Tb);
  GatesFromMUX2X1Tb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    sampler_.clk(clk_);
    wd_.clk(clk_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();
    SC_THREAD(t_stimulus);
  }
private:
  void t_stimulus() {
    scv_smart_ptr<bool> a, b;
    while (true) {
      a->next();
      b->next();
      
      a_ = *a; b_ = *b;
      LOGGER(INFO) << "a = " << a_ << " b = " << b_ << "\n";
      wait(clk_.posedge_event());
    }
  }
  void m_checker() {
    LIBTB2_ERROR_ON(fail_);
  }
  sc_core::sc_clock clk_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(GatesFromMUX2X1Tb);

int sc_main(int argc, char ** argv) {
  GatesFromMUX2X1Tb tb;
  return libtb2::Sim::start(argc, argv);
}
