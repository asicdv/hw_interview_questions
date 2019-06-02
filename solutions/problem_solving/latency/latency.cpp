//========================================================================== //
// Copyright (c) 2016-18, Stephen Henry
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
#include "vobj/Vlatency.h"

#define PORTS(__func)                           \
    __func(issue, bool)                         \
    __func(clear, bool)                         \
    __func(retire, bool)                        \
    __func(issue_cnt_r, T)                      \
    __func(aggregate_cnt_r, T)

typedef Vlatency uut_t;

template<typename T>
struct LatencyTb : libtb2::Top<LatencyTb<T> > {
  SC_HAS_PROCESS(LatencyTb);
  LatencyTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);
    //
    dp_.clk(clk_);
    dp_.in(issue_);
    dp_.out(retire_);
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
    resetter_.wait_reset_done();

    struct delay_constraint : scv_constraint_base {
      scv_smart_ptr<std::size_t> p;
      SCV_CONSTRAINT_CTOR(delay_constraint) {
        SCV_CONSTRAINT((p() > 10) && (p() < 100));
      }
    } dconst("delay_constraint");

    std::size_t N = 16;
    LOGGER(INFO) << "Stimulus starts:\n";
    while (N--) {
      dconst.next();
      const std::size_t delay = *dconst.p;
      LOGGER(INFO) << "Running round latency = " << delay << "\n";
      t_round(delay);
    }
    LOGGER(INFO) << "Stimulus complete!\n";
    libtb2::Sim::end();
  }
  void t_round(std::size_t delay) {
    scv_bag<bool> ibag;
    ibag.add(true, 20);
    ibag.add(false, 80);
    scv_smart_ptr<bool> i;
    i->set_mode(ibag);

    dp_.set_delay(delay);
    dp_.clear();
    t_clear();

    std::size_t n = 0;
    while (n < 16) {
      i->next();
      issue_ = *i;
      if (issue_)
        n++;
      wait(clk_.posedge_event());
      issue_ = false;

      wd_.pat();
    }
    WAIT_FOR_CYCLES(clk_, 100);

    const std::size_t latency = (aggregate_cnt_r_ / issue_cnt_r_);
    LIBTB2_ERROR_ON(latency != delay);

    LOGGER(INFO) << " Expected = " << delay
                 << " Actual = " << latency
                 << " aggregate_cnt = " << aggregate_cnt_r_
                 << " issue_cnt = " << issue_cnt_r_
                 << "\n";
  }
  void t_clear() {
    wait(clk_.posedge_event());
    clear_ = true;
    wait(clk_.posedge_event());
    clear_ = false;
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  libtb2::ConstantDelayPipe<bool> dp_;
  uut_t uut_;
};
SC_MODULE_EXPORT(LatencyTb<uint32_t>);

int sc_main(int argc, char **argv) {
  LatencyTb<uint32_t> tb;
  return libtb2::Sim::start(argc, argv);
}
