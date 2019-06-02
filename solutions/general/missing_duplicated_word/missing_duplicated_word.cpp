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
#include <algorithm>
#include "vobj/Vmissing_duplicated_word.h"

#define PORTS(__func)                           \
  __func(state_upt, bool)                       \
  __func(state_id, uint32_t)                    \
  __func(state_dat, uint32_t)                   \
  __func(cntrl_start, bool)                     \
  __func(cntrl_busy_r, bool)                    \
  __func(cntrl_dat_r, uint32_t)

typedef Vmissing_duplicated_word uut_t;

const int N = 8;

struct MissingDuplicateWordTb : libtb2::Top<MissingDuplicateWordTb> {
  SC_HAS_PROCESS(MissingDuplicateWordTb);
  MissingDuplicateWordTb(sc_core::sc_module_name mn = "t")
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
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    SC_THREAD(t_stimulus);
  }
private:
  void t_stimulus() {
    resetter_.wait_reset_done();

    cntrl_start_ = false;
    while (true) {
      const uint32_t missing = initialize();

      cntrl_start_ = true;
      wait(clk_.posedge_event());
      cntrl_start_ = false;
      do {
        wait(sampler_.sample());
      } while (cntrl_busy_r_);
      const uint32_t detected = cntrl_dat_r_;

      LOGGER(INFO) << " Missing = " << missing
                   << " Detected = " << detected << "\n";
      LIBTB2_ERROR_ON(missing != detected);
    }
  }
  uint32_t initialize() {
    struct n_constraint : scv_constraint_base {
      scv_smart_ptr<uint32_t> p;
      SCV_CONSTRAINT_CTOR(n_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < 32));
      }
    } n_constraint("n_constraint");

    std::vector<uint32_t> ns;
    while (ns.size() != (N + 1)) {
      n_constraint.next();
      const uint32_t n = *n_constraint.p;
      if (std::find(ns.begin(), ns.end(), n) == ns.end())
        ns.push_back(n);
    }

    for (int i = 0; i < N + 1; i++) {
      if (i < N) {
        state_upt_ = true;
        state_id_ = i;
        state_dat_ = ns [i];
        LOGGER(DEBUG) << "Update id = " << i << " dat = " << ns [i] << "\n";
        wait(clk_.posedge_event());
      }
      state_dat_ = ns [i];
      state_id_ = i + N;
      LOGGER(DEBUG) << "Update id = " << (i + N) << " dat = " << ns [i] << "\n";
      wait(clk_.posedge_event());
    }
    state_upt_ = false;
    return ns.back();
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Sampler sampler_;
  libtb2::Resetter resetter_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(MissingDuplicateWordTb);

int sc_main(int argc, char ** argv) {
  MissingDuplicateWordTb tb;
  return libtb2::Sim::start(argc, argv);
}
