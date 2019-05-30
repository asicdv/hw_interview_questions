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

#include "vobj/Vdelay_pipe_static.h"

#define PORTS(__func)                           \
  __func(in, uint32_t)                          \
  __func(in_vld, bool)                          \
  __func(out_r, uint32_t)                       \
  __func(out_vld_r, bool)

typedef Vdelay_pipe_static uut_t;

struct DelayPipeStaticTb : libtb2::Top<uut_t> {
  SC_HAS_PROCESS(DelayPipeStaticTb);
  DelayPipeStaticTb(sc_core::sc_module_name mn = "t")
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

    register_uut(uut_);
    vcd_on();
  }
private:
  void generate_stimulus(std::size_t n = 10) {
    scv_smart_ptr<uint32_t> rnd;
    stimulus_.clear();
    i_ = 0;
    j_ = 0;
    while (n-- != 0) {
      rnd->next();
      stimulus_.push_back(*rnd);
    }
  }
  void t_in() {
    in_vld_ = false;
    in_ = 0;
    
    resetter_.wait_reset_done();

    wait(10, SC_NS);

    while (i_ < stimulus_.size()) {
      wait(clk_.posedge_event());

      in_vld_ = true;
      in_ = stimulus_[i_++];
    }
    in_vld_ = false;

    wait(100, SC_NS);
    sc_core::sc_stop();
  }
  void m_out() {
    if (out_vld_r_) {
      const uint32_t actual = out_r_;
      const uint32_t expected = stimulus_[j_++];

      std::cout << "[" << sc_core::sc_time() << "] ";
      if (actual != expected)
        std::cout << "*** Mismatch expected = " << expected
                  << " actual = " << actual;
      else
        std::cout << "Match!";

      std::cout << "\n";
    }
  }
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  std::vector<uint32_t> stimulus_;
  std::size_t i_, j_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  uut_t uut_;
};
SC_MODULE_EXPORT(DelayPipeStaticTb);

int sc_main(int argc, char **argv) {
  DelayPipeStaticTb tb("tb");
  return libtb2::Sim::start(argc, argv);
}
