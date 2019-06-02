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

#include "libtb2.hpp"
#include "verilated_vcd_sc.h"
#include <deque>
#include "vobj/Vfifo_async.h"

#define PORTS(__func)                           \
  __func(push, bool)                            \
  __func(push_data, uint32_t)                   \
  __func(pop, bool)                             \
  __func(pop_data, uint32_t)                    \
  __func(pop_data_vld_r, bool)                  \
  __func(empty_r, bool)                         \
  __func(full_r, bool)

typedef Vfifo_async uut_t;

struct FifoAsyncTb : libtb2::Top<FifoAsyncTb> {
  SC_HAS_PROCESS(FifoAsyncTb);
  FifoAsyncTb(sc_core::sc_module_name mn = "t")
    : uut_("uut"), wresetter_("wresetter"),
      rresetter_("rresetter"),
      rclk_("rclk"), wclk_("wclk"),
      rsampler_("rsampler"), wsampler_("wsampler") {
    //
    wresetter_.clk(wclk_);
    wresetter_.rst(wrst_);
    //
    rresetter_.clk(rclk_);
    rresetter_.rst(rrst_);
    //
    rsampler_.clk(rclk_);
    wsampler_.clk(wclk_);
    //
    wd_.clk(rclk_);
    //
    uut_.rclk(rclk_);
    uut_.rrst(rrst_);
    //
    uut_.wclk(wclk_);
    uut_.wrst(wrst_);
    //
#define __bind_signals(__name, __type)       \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_THREAD(t_push);
    SC_THREAD(t_pop);
    SC_METHOD(m_checker);
    sensitive << rsampler_.sample();
    dont_initialize();
  }
private:
  void m_checker() {
    if (pop_data_vld_r_) {
      LIBTB2_ERROR_ON(expected_.empty());
      const uint32_t expected = expected_.front(); expected_.pop_front();
      const uint32_t actual = pop_data_;
      LOGGER(INFO) << " Expected = " << std::hex << expected
                   << " Actual = " << actual
                   << "\n";
      LIBTB2_ERROR_ON(actual != expected);
    }
  }
  void t_push() {
    wresetter_.wait_reset_done();

    scv_smart_ptr<bool> push;
    scv_smart_ptr<uint32_t> push_data;
    while (true) {
      push_ = false;

      wait(wsampler_.sample());
      if (!full_r_) {
        push->next();
        push_data->next();

        const bool rtl_push = *push;
        const uint32_t rtl_push_data = *push_data;
        
        push_ = rtl_push;
        push_data_ = rtl_push_data;
        if (rtl_push) {
          expected_.push_back(rtl_push_data);
          LOGGER(INFO) << "Push data = " << std::hex << rtl_push_data << "\n";
        }
      }
      wait(wclk_.posedge_event());
    }
  }
  void t_pop() {
    rresetter_.wait_reset_done();

    scv_smart_ptr<bool> pop;
    while (true) {
      pop_ = false;

      wait(rsampler_.sample());
      if (!empty_r_) {
        pop->next();
        pop_ = *pop;
      }
      wait(rclk_.posedge_event());
    }
  }
  std::deque<uint32_t> expected_;
  sc_core::sc_clock wclk_, rclk_;
  sc_core::sc_signal<bool> wrst_, rrst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Sampler rsampler_, wsampler_;
  libtb2::Resetter wresetter_, rresetter_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};

int sc_main(int argc, char ** argv) {
  FifoAsyncTb tb;
  return libtb2::Sim::start(argc, argv);
}
