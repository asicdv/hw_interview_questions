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
#include "vobj/Vcount_zeros_32.h"

#define PORTS(__func)                           \
  __func(pass, bool)                            \
  __func(x, T)                                  \
  __func(valid_r, bool)                         \
  __func(y, T)

typedef Vcount_zeros_32 uut_t;

template<typename T>
struct CountZeros32Tb : libtb2::Top<T> {
  SC_HAS_PROCESS(CountZeros32Tb);
  CountZeros32Tb(sc_module_name mn = "t")
    : uut_("uut") {
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    sampler_.clk(clk_);
    //
    wd_.clk(clk_);
    //
    SC_THREAD(t_stimulus);
    //
    SC_METHOD(m_checker);
    this->sensitive << sampler_.sample();
    this->dont_initialize();
    //
    st_.reset();
  }
  void end_of_simulation() {
    LOGGER(INFO) << "Samples processed=" << st_.n << "\n";
  }
private:
  void m_checker() {
    const libtb2::Options & o = libtb2::Sim::get_options();

    if (!valid_r_)
      return ;

    const T d = d_.front(); d_.pop_front();
    const std::size_t zeros = libtb2::popcount(~d);
    LIBTB2_ERROR_ON(zeros != y_);

    if (o.debug_on()) {
      LOGGER(DEBUG) << "x = " << x_
                    << " actual = " << y_
                    << " expected = " << zeros
                    << "\n";
    }
  }
  void t_stimulus() {
    resetter_.wait_reset_done();

    scv_smart_ptr<T> p;
    while (true) {
      p->next();

      pass_ = true;
      x_ = *p;
      d_.push_back(*p);
      wait(clk_.posedge_event());
    }
  }
  struct {
    void reset() { n = 0; }
    std::size_t n;
  } st_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  std::deque<T> d_;
  uut_t uut_;
};

int sc_main(int argc, char **argv) {
  CountZeros32Tb<uint32_t> tb;
  return libtb2::Sim::start(argc, argv);
}
