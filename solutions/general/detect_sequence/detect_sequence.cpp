//========================================================================== //
// Copyright (c) 2016-17, Stephen Henry
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
#include "vobj/Vdetect_sequence.h"

#define PORTS(__func)                           \
    __func(in, bool)                            \
    __func(fail_r, bool)

typedef Vdetect_sequence uut_t;

struct DetectSequenceTb : libtb2::Top<DetectSequenceTb> {
  SC_HAS_PROCESS(DetectSequenceTb);
  DetectSequenceTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
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
    scv_smart_ptr<bool> in;

    resetter_.wait_reset_done();
    while (true) {
      in->next();

      in_ = *in;
      LOGGER(INFO) << "in = " << in_ << "\n";
      wait(clk_.posedge_event());
    }
  }
  void m_checker() {
    LIBTB2_ERROR_ON(fail_);
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Sampler sampler_;
  libtb2::Resetter resetter_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(DetectSequenceTb);

int sc_main(int argc, char ** argv) {
  DetectSequenceTb tb;
  return libtb2::Sim::start(argc, argv);
}
