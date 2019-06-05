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

#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include "vobj/Vvending_machine_fsm.h"

#define PORTS(__func)                           \
  __func(nickel, bool)                          \
  __func(dime, bool)                            \
  __func(vend, bool)                            \
  __func(change, bool)

struct TOP : tb::Top {
  TOP() {
    v.clk(clk);
    v.rst(rst);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void set_idle() {
    nickel = false;
    dime = false;
  }
  void set_nickel(bool x = true) { nickel = x; }
  void set_dime(bool x = true) { dime = x; }
  bool get_vend() const { return vend; }
  bool get_change() const { return change; }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  void t_apply_reset() {
    set_idle();
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  Vvending_machine_fsm v{"vending_machine_fsm"};
  ::sc_core::sc_clock clk{"clk"};
  ::sc_core::sc_signal<bool> rst{"rst"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
  PORTS(__declare_signal)
#undef __declare_signal
 private:
  void start_tracing() {
#ifdef OPT_ENABLE_TRACE
    Verilated::traceEverOn(true);
    vcd_ = std::make_unique<VerilatedVcdSc>();
    v.trace(vcd_.get(), 99);
    vcd_->open("sim.vcd");
#endif
  }
  void stop_tracing() {
#ifdef OPT_ENABLE_TRACE
    vcd_->close();
#endif
  }
#ifdef OPT_ENABLE_TRACE
  std::unique_ptr<VerilatedVcdSc> vcd_;
#endif
};

namespace {

struct VendingMachineModel {
  VendingMachineModel() { reset(); }
  
  void set_nickel(bool x = true) { if (x) sum_ += 5; }
  void set_dime(bool x = true) { if (x) sum_ += 10; }

  std::size_t sum() const { return sum_; }
  bool vend() const { return sum_ >= 40; }
  bool change() const { return sum_ > 40; }

  void reset() { sum_ = 0; }
  void step() {}
 private:
  std::size_t sum_{0};
};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(VendingMachineFsmTest, Basic) {
  struct VendingMachineFsmTask : ::tb::Task {
    VendingMachineFsmTask(TOP & top, std::size_t n = 1024)
        : top_(top), n_(n) {}
    void execute() override {
      scenario_1();
      scenario_2();
      finish();
    }
   private:
    void scenario_1() {
      // Add dimes until machine vends
      
      tb::Random::Bag<bool> bg_do_coin;
      bg_do_coin.add(true, 80);
      bg_do_coin.add(false, 20);
      bg_do_coin.finalize();

      VendingMachineModel mdl{};
      top_.t_apply_reset();

      while (!mdl.vend()) {
        top_.set_idle();
        if (bg_do_coin()) {
          top_.set_nickel();
          mdl.set_nickel();
        }
        top_.t_await_cycles();
      }
      ASSERT_TRUE(top_.get_vend());
      ASSERT_FALSE(top_.get_change());
    }
    void scenario_2() {
      // Add dimes and nickle to cause machine to vend with change.

      VendingMachineModel mdl{};
      top_.t_apply_reset();

      for (std::size_t i = 0; i < 7; i++) {
        top_.set_nickel();
        mdl.set_nickel();
        top_.t_await_cycles();
      }

      ASSERT_FALSE(top_.get_vend());
      ASSERT_FALSE(top_.get_change());

      top_.set_idle();
      top_.set_dime();
      mdl.set_dime();
      top_.t_await_cycles();
      top_.set_idle();

      ASSERT_TRUE(mdl.vend());
      ASSERT_TRUE(mdl.change());
      ASSERT_TRUE(top_.get_vend());
      ASSERT_TRUE(top_.get_change());
    }
    TOP & top_;
    std::size_t n_;
  };
  auto task = std::make_unique<VendingMachineFsmTask>(top);
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
