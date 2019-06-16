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


#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include <vector>
#include <sstream>
#include <utility>
#include <memory>
#include "vobj/Vmulti_ported_flop_lvt_3r3w.h"

using word_type = uint32_t;

#define PORTS(__func)                           \
  __func(ren0, bool)                            \
  __func(raddr0, word_type)                     \
  __func(rdata0, word_type)                     \
  __func(ren1, bool)                            \
  __func(raddr1, word_type)                     \
  __func(rdata1, word_type)                     \
  __func(ren2, bool)                            \
  __func(raddr2, word_type)                     \
  __func(rdata2, word_type)                     \
  __func(wen0, bool)                            \
  __func(waddr0, word_type)                     \
  __func(wdata0, word_type)                     \
  __func(wen1, bool)                            \
  __func(waddr1, word_type)                     \
  __func(wdata1, word_type)                     \
  __func(wen2, bool)                            \
  __func(waddr2, word_type)                     \
  __func(wdata2, word_type)                     \
  __func(init, bool)                            \
  __func(busy_w, bool)

struct TOP : tb::Top {
  TOP() {
    v.rst(rst);
    v.clk(clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void idle_wr(std::size_t i) {
    switch (i) {
      case 0:
        wen0 = false;
        waddr0 = word_type{};
        wdata0 = word_type{};
        break;
      case 1:
        wen1 = false;
        waddr1 = word_type{};
        wdata1 = word_type{};
        break;
      case 2:
        wen2 = false;
        waddr2 = word_type{};
        wdata2 = word_type{};
        break;
    }
  }
  void idle_rd(std::size_t i) {
    switch (i) {
      case 0:
        ren0 = false;
        raddr0 = word_type{};
        break;
      case 1:
        ren1 = false;
        raddr1 = word_type{};
        break;
      case 2:
        ren2 = false;
        raddr2 = word_type{};
        break;
    }
  }
  word_type t_set_read(std::size_t i, word_type addr) {
    word_type ret;
    switch (i) {
      case 0:
        ren0 = true;
        raddr0 = addr;
        t_await_cycles(1);
        ret = rdata0;
        break;
      case 1:
        ren1 = true;
        raddr1 = addr;
        t_await_cycles(1);
        ret = rdata1;
        break;
      case 2:
        ren2 = true;
        raddr2 = addr;
        t_await_cycles(1);
        ret = rdata2;
        break;
    }
    return ret;
  }
  void t_set_write(std::size_t i, word_type addr, word_type data) {
    switch (i) {
      case 0:
        wen0 = true;
        waddr0 = addr;
        wdata0 = data;
        break;
      case 1:
        wen1 = true;
        waddr1 = addr;
        wdata1 = data;
        break;
      case 2:
        wen2 = true;
        waddr2 = addr;
        wdata2 = data;
        break;
    }
  }
  void t_apply_init() {
    init = true;
    t_await_cycles(1);
    init = false;
    while (busy_w)
      t_await_cycles(1);
      
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vmulti_ported_flop_lvt_3r3w v{"multi_ported_flop_lvt_3r3w"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
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

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(MemTest, Basic) {
  const std::size_t n{1024 << 6};

  struct WriteCommand {
    word_type addr, data;
  };
  struct MemTask : tb::Task {
    MemTask(TOP & top, std::size_t rd_n = 3, std::size_t wr_n = 3)
        : top_(top), rd_n_(rd_n), wr_n_(wr_n) {}
    void add_write_stimulus(std::size_t i, const WriteCommand & cmd) {
      stimulus_[i].push_back(cmd);
    }
    void execute() override {
      using namespace sc_core;

      top_.t_apply_reset();
      top_.t_apply_init();

      std::vector<::sc_core::sc_process_handle> h;
      for (std::size_t i = 0; i < rd_n_; i++) {
        std::stringstream ss{"t_read"}; ss << i;
        const std::string s{ss.str()};
        h.push_back(sc_spawn(std::bind(&MemTask::t_read, this, i), s.c_str()));
      }

      for (std::size_t i = 0; i < wr_n_; i++) {
        std::stringstream ss{"t_write"}; ss << i;
        const std::string s{ss.str()};
        h.push_back(sc_spawn(std::bind(&MemTask::t_write, this, i), s.c_str()));
      }
      t_join(h);
      finish();
    }
   private:
    void t_write(std::size_t i) {
      tb::Random::UniformRandomInterval<word_type> dly{4};
      for (const WriteCommand & cmd : stimulus_[i]) {
        top_.t_set_write(i, cmd.addr, cmd.data);
        top_.t_await_cycles(1);
        top_.idle_wr(i);
        top_.t_await_cycles(dly());
        mem_[cmd.addr] = cmd.data;
      }
      writes_done_[i].notify();
    }
    void t_read(std::size_t i) {
      wait(writes_done_[i]);
      tb::Random::UniformRandomInterval<word_type> dly{4};

      const std::size_t lo = 1024 * i;
      const std::size_t hi = 1024 * (i + 1);
      for (std::size_t word = lo; word < hi; word++) {
        const word_type rdata_rtl{top_.t_set_read(i, word)};
        const word_type rdata_tb{mem_[word]};
        ASSERT_EQ(rdata_rtl, rdata_tb);
      }
    }
    void t_join(std::vector<::sc_core::sc_process_handle> & v) {
      ::sc_core::sc_event_and_list e;
      for (::sc_core::sc_process_handle h : v)
        e &= h.terminated_event();
      wait(e);
    }
    std::map<word_type, word_type> mem_;
    ::sc_core::sc_event writes_done_[3];
    std::vector<WriteCommand> stimulus_[3];
    std::size_t rd_n_, wr_n_;
    TOP & top_;
  };
  
p  auto task = std::make_unique<MemTask>(top);

  tb::Random::UniformRandomInterval<word_type> data{};
  for (std::size_t wr = 0; wr < 3; wr++) {
    tb::Random::UniformRandomInterval<word_type> addr(1024 * (wr + 1), 1024 * wr);
    
    for (std::size_t i = 0; i < n; i++)
      task->add_write_stimulus(wr, WriteCommand{addr(), data()});
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}
