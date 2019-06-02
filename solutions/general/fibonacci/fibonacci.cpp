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
#include "vobj/Vfibonacci.h"

#define PORTS(__func)                           \
    __func(y, uint32_t)

template<typename T>
struct Fibonacci {
  Fibonacci() : a_(0), b_(1), ret_a_(true) {}
  T operator()() {
    T ret;
    if (ret_a_) {
      ret = a_;
      a_ += b_;
    } else {
      ret = b_;
      b_ += a_;
    }
    ret_a_ = !ret_a_;
    return ret;
  }
 private:
  bool ret_a_;
  T a_, b_;
};

struct FibonacciTb : libtb2::Top<FibonacciTb> {
  typedef Vfibonacci uut_t;
  SC_HAS_PROCESS(FibonacciTb);
  FibonacciTb(sc_core::sc_module_name mn = "t")
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
#define __bind_ports(__name, __type)            \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    SC_THREAD(t_checker);
  }
 private:
  void t_checker() {
    resetter_.wait_reset_done();

    while (true) {
      wait(sampler_.sample());
    
      const uint32_t actual = y_;
      const uint32_t expected = f_();

      LOGGER(INFO) << " Actual = " << actual
                   << " Expected = " << expected
                   << "\n";
      LIBTB2_ERROR_ON(actual != expected);
    }
  }
  Fibonacci<uint32_t> f_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(FibonacciTb);

int sc_main(int argc, char **argv) {
  FibonacciTb tb;
  return libtb2::Sim::start(argc, argv);
}
