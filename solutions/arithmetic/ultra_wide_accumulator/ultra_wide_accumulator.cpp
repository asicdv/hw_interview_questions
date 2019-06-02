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
#include "vobj/Vultra_wide_accumulator.h"

typedef Vultra_wide_accumulator uut_t;
typedef sc_dt::sc_bv<128> rtl_word_t;
typedef sc_dt::sc_biguint<128> beh_word_t;

beh_word_t convert(const rtl_word_t & r) {
  beh_word_t b;
  b.range(127, 64) = r.range(127, 64).to_uint64();
  b.range(63, 0) = r.range(63, 0).to_uint64();
  return b;
}

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(clear, bool)                           \
  __func(x, rtl_word_t)                             \
  __func(y_r, rtl_word_t)                           \
  __func(y_vld_r, bool)

struct AccumulatorModel {
  AccumulatorModel() : word_(0) {}
  void clear() { word_ = 0; }
  void add(const beh_word_t & w) { word_ = word_ + w; }
  beh_word_t word() const { return word_; }
private:
  beh_word_t word_;
};

struct UltraWideAccumulatorTb : libtb2::Top<UltraWideAccumulatorTb> {
  SC_HAS_PROCESS(UltraWideAccumulatorTb);
  UltraWideAccumulatorTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {

    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);

    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports

    SC_THREAD(t_stimulus);
    SC_METHOD(m_sample);
    sensitive << sampler_.sample();
    dont_initialize();
  }
private:
  void m_sample() {
    if (!y_vld_r_)
      return ;

    const rtl_word_t r = applied_.front(); applied_.pop_front();
    mdl_.add(convert(r));

    LOGGER(DEBUG) << "EXPECTED=" << mdl_.word().to_string(SC_HEX)
                  << " ACTUAL=" << y_r_.read().to_string(SC_HEX) << "\n";
    LIBTB2_ERROR_ON(mdl_.word() != convert(y_r_));
  }
  void t_stimulus() {
    const libtb2::Options & o = libtb2::Sim::get_options();

    resetter_.wait_reset_done();

    struct x_constraint : scv_constraint_base {
      scv_smart_ptr<rtl_word_t> w;
      SCV_CONSTRAINT_CTOR(x_constraint) {
        SCV_CONSTRAINT(w() < (1ll << 32));
      }
    } x("x_constraint");
    
    while (true) {
      x.next();

      pass_ = true;
      x_ = *x.w;

      applied_.push_back(*x.w);
      wait(clk_.posedge_event());
    }
  }
  std::deque<rtl_word_t> applied_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  AccumulatorModel mdl_;
  uut_t uut_;
};

int sc_main(int argc, char **argv) {
  UltraWideAccumulatorTb tb;
  return libtb2::Sim::start(argc, argv);
}

