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
#include <deque>
#include "vobj/Vdiv_by_3.h"

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(x, uint32_t)                           \
  __func(busy_r, bool)                          \
  __func(valid_r, bool)                         \
  __func(y_r, uint32_t)

typedef Vdiv_by_3 uut_t;

struct DivBy3Tb : libtb2::Top<DivBy3Tb> {
  SC_HAS_PROCESS(DivBy3Tb);
  DivBy3Tb(sc_core::sc_module_name mn = "t")
    : uut_("uut"), clk_("clk"), rst_("rst")
  {
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    wd_.clk(clk_);
    sampler_.clk(clk_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports

    SC_THREAD(t_stimulus);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();
  }
private:
  void t_stimulus() {
    struct x_constraint : scv_constraint_base {
      scv_smart_ptr<uint32_t> p;
      SCV_CONSTRAINT_CTOR(x_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < (1 << 16) - 1));
      }
    } x_c("x_constraint");

    pass_ = false;
    x_ = 0;

    resetter_.wait_reset_done();
    while (true) {
      x_c.next();

      const uint32_t x = *x_c.p;
      q_.push_back(x / 3);

      pass_ = true;
      x_ = x;
      wait(clk_.posedge_event());
      pass_ = false;

      if (!busy_r_)
        wait(busy_r_.posedge_event());

      while (busy_r_)
        wait(clk_.posedge_event());
    }
  }
  void m_checker() {
    if (valid_r_) {
      const uint32_t actual = y_r_;
      const uint32_t expected = q_.front(); q_.pop_front();

      // Account for rounding.
      LIBTB2_ERROR_ON(std::abs(actual, expected) > 1);
      LOGGER(INFO) << " Actual = " << actual
                   << " Expected = " << expected
                   << "\n";
    }
  }
  std::deque<uint32_t> q_;
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
SC_MODULE_EXPORT(DivBy3Tb);

int sc_main(int argc, char **argv) {
  DivBy3Tb tb;
  return libtb2::Sim::start(argc, argv);
}
