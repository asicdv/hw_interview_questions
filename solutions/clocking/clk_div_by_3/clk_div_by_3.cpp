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
#include "vobj/Vclk_div_by_3.h"

#define PORTS(__func)                           \
  __func(clk_div_3, bool)
typedef Vclk_div_by_3 uut_t;

struct ClkDivBy3Tb : libtb2::Top<ClkDivBy3Tb> {
  SC_HAS_PROCESS(ClkDivBy3Tb);
  ClkDivBy3Tb(sc_core::sc_module_name mn = "t")
      : uut_("uut")
#define __construct_signals(__name, __type)     \
      , __name##_(#__name)
      PORTS(__construct_signals)
#undef __construct_signals
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
#define __bind_signals(__name, __type)          \
    uut_.__name(__name##_);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_METHOD(m_clk1);
    sensitive << clk_.posedge_event()
              << rst_.posedge_event();
    dont_initialize();
    
    SC_METHOD(m_clk2);
    sensitive << clk_div_3_.posedge_event()
              << rst_.posedge_event();
    dont_initialize();

    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();

    count_1_ = 0;
    count_2_ = 0;
  }
 private:
  void m_clk1() {
    if (rst_)
      count_1_ = 0;
    else
      count_1_++;
  }
  void m_clk2() {
    if (rst_)
      count_2_ = 0;
    else
      count_2_++;
  }
  void m_checker() {
    if (count_1_ == 0)
      return ;

    if (count_1_ % 33 == 0) {
      LIBTB2_ERROR_ON(count_2_ % 11 != 0);

      LOGGER(INFO) << " Fast = " << count_1_ << " Slow = " << count_2_
                   << " Ratio = " << (count_1_ / count_2_)
                   << "\n";
    }
  }
  
  std::size_t count_1_, count_2_;

  libtb2::Resetter resetter_;
  libtb2::SimWatchDogCycles wd_;
  libtb2::Sampler sampler_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  uut_t uut_;
};
SC_MODULE_EXPORT(ClkDivBy3Tb);

int sc_main(int argc, char **argv) {
  ClkDivBy3Tb tb;
  return libtb2::Sim::start(argc, argv);
}
