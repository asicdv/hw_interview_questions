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
#include "vobj/Vmcp_formulation.h"

#define PORTS(__func)                           \
    __func(l_in_pass_r, bool)                   \
    __func(l_in_r, uint32_t)                    \
    __func(l_busy_r, bool)                      \
    __func(c_out_pass_r, bool)                  \
    __func(c_out_r, uint32_t)

typedef Vmcp_formulation uut_t;

struct McpFormulationTb : libtb2::Top<McpFormulationTb> {

  SC_HAS_PROCESS(McpFormulationTb);
  McpFormulationTb(sc_core::sc_module_name = "t")
      : uut_("uut")
#define __construct_signals(__name, __type)     \
      , __name##_(#__name)
      PORTS(__construct_signals)
#undef __construct_signals
  {
    //
    resetter_0_.clk(clk_0_);
    resetter_0_.rst(rst_0_);

    //
    resetter_1_.clk(clk_1_);
    resetter_1_.rst(rst_1_);
    //
    wd_.clk(clk_0_);
    //
    sampler_0_.clk(clk_0_);
    sampler_1_.clk(clk_1_);
    
    SC_METHOD(m_checker);
    sensitive << sampler_1_.sample();
    dont_initialize();

    uut_.l_clk(clk_0_);
    uut_.l_rst(rst_0_);
    uut_.c_clk(clk_1_);
    uut_.c_rst(rst_1_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name##_);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_THREAD(t_stimulus);
  }
 private:
  void t_stimulus() {
    b_idle();

    resetter_0_.wait_reset_done();
    resetter_1_.wait_reset_done();
    
    LOGGER(INFO) << "Starting stimulus...\n";

    scv_smart_ptr<uint32_t> p;
    while (true) {
      p->next();

      const uint32_t actual = *p;
       b_issue(actual);
    }
    LOGGER(INFO) << "Stimulus ends.\n";

    wait();
  }

  void b_idle() {
    l_in_pass_r_ = false;
    l_in_r_ = 0;;
  }

  void b_issue(const uint32_t & w) {
    while (l_busy_r_)
      wait(sampler_0_.sample());

    l_in_pass_r_ = true;
    l_in_r_ = w;
    wait_cycles();
    LOGGER(INFO) << "Launching: " << w << "\n";
    queue_.push_back(w);
    b_idle();
  }

  void m_checker() {
    if (c_out_pass_r_) {
      LIBTB2_ERROR_ON(queue_.size() == 0);

      const uint32_t expected = queue_.front();
      queue_.pop_front();
      const uint32_t actual = c_out_r_;
      LIBTB2_ERROR_ON(actual != expected);
      LOGGER(INFO) << "Capturing: " << actual << "\n";
    }
  }

  void wait_cycles(std::size_t cycles = 1) {
    while (cycles--)
      wait(clk_1_.posedge_event());
  }

  std::deque<uint32_t> queue_;
  libtb2::Resetter resetter_0_, resetter_1_;
  libtb2::SimWatchDogCycles wd_;
  libtb2::Sampler sampler_0_, sampler_1_;
  sc_core::sc_clock clk_0_, clk_1_;
  sc_core::sc_signal<bool> rst_0_, rst_1_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  uut_t uut_;
};
SC_MODULE_EXPORT(McpFormulationTb);

int sc_main(int argc, char **argv) {
  McpFormulationTb tb;
  return libtb2::Sim::start(argc, argv);
}
