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
#include <vector>
#include <iterator>
#include <algorithm>
#include <bitset>
#include "vobj/Vzero_indices_fast.h"
typedef Vzero_indices_fast uut_t;

#define PORTS(__func)                           \
  __func(in_vector, sc_bv<128>)                 \
  __func(in_start, bool)                        \
  __func(in_busy_r, bool)                       \
  __func(resp_valid_r, bool)                    \
  __func(resp_index_r, uint32_t)

typedef Vzero_indices_fast uut_t;

struct ZeroIndicesXActIntf : sc_core::sc_interface {
  virtual void issue(const sc_bv<128> & x) = 0;
};

struct ZeroIndicesXAct : ZeroIndicesXActIntf, sc_core::sc_module {
  sc_core::sc_in<bool> clk;
  sc_core::sc_out<sc_bv<128> > in_vector;
  sc_core::sc_out<bool> in_start;
  sc_core::sc_in<bool> in_busy_r;
  ZeroIndicesXAct(sc_core::sc_module_name mn = "xact")
    : sc_core::sc_module(mn)
    , clk("clk")
    , in_vector("in_vector")
    , in_start("in_start")
    , in_busy_r("in_busy_r")
  {}
  void issue(const sc_bv<128> & x) {
    in_start = true;
    in_vector = x;
    wait(clk.posedge_event());
    idle();
  }
private:
  void idle() {
    in_start = false;
    in_vector = 0;
  }
};

struct ZeroIndicesFastTb : libtb2::Top<ZeroIndicesFastTb> {
  sc_core::sc_port<ZeroIndicesXActIntf> p;
  SC_HAS_PROCESS(ZeroIndicesFastTb);
  ZeroIndicesFastTb(sc_core::sc_module_name mn = "t")
    : uut_("uut"), p("p"), xact_("xact")
  {
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    p.bind(xact_);
    xact_.clk(clk_);
    xact_.in_vector(in_vector_);
    xact_.in_start(in_start_);
    xact_.in_busy_r(in_busy_r_);
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
    resetter_.wait_reset_done();

    scv_smart_ptr<sc_bv<128> > in_vector;
    while (true) {

      in_vector->next();

      const sc_bv<128> actual = *in_vector;
      LOGGER(INFO) << "Issue new word = "
                   << actual
                   << " zero count = " << libtb2::popcount(~actual)
                   << "\n";
      zero_indices_.clear();
      libtb2::find_bits2(actual, std::back_inserter(zero_indices_));
      std::reverse(zero_indices_.begin(), zero_indices_.end());
      p->issue(actual);

      if (!in_busy_r_)
        wait(in_busy_r_.posedge_event());

      while (in_busy_r_)
        wait(clk_.posedge_event());
    }
  }
  void m_checker() {
    if (resp_valid_r_) {
      LIBTB2_ERROR_ON(zero_indices_.size() == 0);

      const uint32_t actual = resp_index_r_;
      const uint32_t expected = zero_indices_.back(); zero_indices_.pop_back();

      LIBTB2_ERROR_ON(actual != expected);
      LOGGER(INFO) << "\tIndex = " << actual << " (" << expected << ")\n";
    }
  }
  std::vector<std::size_t> zero_indices_;
  ZeroIndicesXAct xact_;
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
SC_MODULE_EXPORT(ZeroIndicesFastTb);

int sc_main(int argc, char ** argv) {
  ZeroIndicesFastTb tb;
  return libtb2::Sim::start(argc, argv);
}
