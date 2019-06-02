//========================================================================== //
// Copyright (c) 2016-17, Stephen Henry
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
#include "vobj/Vvending_machine_dp.h"

#define PORTS(__func)                           \
  __func(client_nickel, bool)                   \
  __func(client_dime, bool)                     \
  __func(client_quarter, bool)                  \
  __func(client_dispense, bool)                 \
  __func(client_enough_r, bool)                 \
  __func(serve_done, bool)                      \
  __func(serve_emit_irn_bru_r, bool)            \
  __func(change_done, bool)                     \
  __func(change_emit_dime_r, bool)

typedef Vvending_machine_dp uut_t;

struct VendingMachineDpTb : libtb2::Top<VendingMachineDpTb> {
  enum CoinType { NICKEL, DIME, QUARTER };

  SC_HAS_PROCESS(VendingMachineDpTb);
  VendingMachineDpTb(sc_core::sc_module_name mn = "t")
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

    SC_THREAD(t_stimulus);
    SC_THREAD(t_emit_serve_done);
    SC_THREAD(t_emit_change_done);
    SC_THREAD(t_dispense);

    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name##_);
    PORTS(__bind_signals)
#undef __bind_signals
  }
 private:
  void t_stimulus() {
    LOGGER(INFO) << "Stimulus starts...\n";
    test_0();
    LOGGER(INFO) << "Stimulus ends\n";

    wait();
  }

  void test_0() {
    LOGGER(INFO) << "Test 0";
    issue_coin(QUARTER);
    issue_coin(QUARTER);
    LOGGER(INFO) << "\tPass\n";

    wait_cycles(100);
  }

  void issue_idle() {
    client_nickel_ = false;
    client_dime_ = false;
    client_quarter_ = false;
  }

  void issue_coin(CoinType c) {
    issue_idle();
    client_nickel_ = (c == NICKEL);
    client_dime_ = (c == DIME);
    client_quarter_ = (c == QUARTER);
    wait_cycles();
    issue_idle();
  }

  void reset_change_count() { change_count_ = 0; }

  void t_dispense() {

    const int DLY = 2;
    while (1) {
      client_dispense_ = false;
      wait(client_enough_r_.posedge_event());
      wait_cycles(DLY);
      client_dispense_ = true;
      wait_cycles();
      client_dispense_ = false;
    }
  }

  void t_emit_change_done() {
    const int DLY = 4;
    while (1) {
      change_done_ = false;
      wait(change_emit_dime_r_.posedge_event());
      wait_cycles(DLY);
      change_count_++;
      change_done_ = true;
      wait_cycles();
    }
  }

  void t_emit_serve_done() {
    const int DLY = 3;
    while (1) {
      serve_done_ = false;
      wait(serve_emit_irn_bru_r_.posedge_event());
      wait_cycles(DLY);
      serve_done_ = true;
      wait_cycles();
    }
  }

  void wait_cycles(std::size_t cycles = 1) {
    while (cycles-- > 0)
      wait(clk_.posedge_event());
  }

  std::size_t change_count_;
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
SC_MODULE_EXPORT(VendingMachineDpTb);

int sc_main (int argc, char **argv) {
  VendingMachineDpTb tb;
  return libtb2::Sim::start(argc, argv);
}

