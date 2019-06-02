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
#include <deque>
#include <vector>
#include <algorithm>
#include <iomanip>
#include "vobj/Vfifo_multi_push.h"

#define PORTS(__func)                           \
  __func(push_0, bool)                          \
  __func(push_0_data, uint32_t)                 \
  __func(push_1, bool)                          \
  __func(push_1_data, uint32_t)                 \
  __func(push_2, bool)                          \
  __func(push_2_data, uint32_t)                 \
  __func(push_3, bool)                          \
  __func(push_3_data, uint32_t)                 \
  __func(pop_0, bool)                           \
  __func(pop_0_data_r, uint32_t)                \
  __func(pop_0_valid_r, bool)                   \
  __func(empty_r, bool)                         \
  __func(full_r, uint32_t)

struct FifoMultiPushCmd {
  //
  uint32_t push;
  uint32_t push_0_data;
  uint32_t push_1_data;
  uint32_t push_2_data;
  uint32_t push_3_data;
};

template<>
struct scv_extensions<FifoMultiPushCmd> : public scv_extensions_base<FifoMultiPushCmd> {
  //
  scv_extensions<uint32_t> push;
  scv_extensions<uint32_t> push_0_data;
  scv_extensions<uint32_t> push_1_data;
  scv_extensions<uint32_t> push_2_data;
  scv_extensions<uint32_t> push_3_data;

  SCV_EXTENSIONS_CTOR(FifoMultiPushCmd) {
    SCV_FIELD(push);
    SCV_FIELD(push_0_data);
    SCV_FIELD(push_1_data);
    SCV_FIELD(push_2_data);
    SCV_FIELD(push_3_data);
  }
};

struct FifoMultiPushCmdConstraint : scv_constraint_base {
  scv_smart_ptr<FifoMultiPushCmd> cmd;
  SCV_CONSTRAINT_CTOR(FifoMultiPushCmdConstraint) {
    SCV_CONSTRAINT((cmd->push() == 0x0) ||
                   (cmd->push() == 0x1) ||
                   (cmd->push() == 0x3) ||
                   (cmd->push() == 0x7) ||
                   (cmd->push() == 0xF));
  }
};

struct FifoMultiPushTb : libtb2::Top<FifoMultiPushTb> {
  typedef Vfifo_multi_push uut_t;
  SC_HAS_PROCESS(FifoMultiPushTb);
  FifoMultiPushTb(sc_core::sc_module_name mn = "t")
    : uut_("uut") {
    //
    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    wd_.clk(clk_);
    //
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __declare_signals(__name, __type)       \
    uut_.__name(__name ## _);
    PORTS(__declare_signals)
#undef __declare_signals
    SC_THREAD(t_stimulus);
    SC_THREAD(t_pop);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();

    st_.reset();
  }
  void end_of_simulation() {
    st_.report();
  }
private:
  void t_pop() {
    resetter_.wait_reset_done();

    scv_smart_ptr<bool> pop;
    while (true) {
      pop->next();

      wait(sampler_.sample());
      pop_0_ = empty_r_ ? false : *pop;
      wait(clk_.posedge_event());
    }
  }
  void m_checker() {
    if (pop_0_valid_r_) {
      const uint32_t actual = pop_0_data_r_;
      const uint32_t expected = expected_.front(); expected_.pop_front();

      LOGGER(INFO) << std::hex << std::setw(8) << std::setfill('0')
                   << " Expected = 0x" << expected
                   << " Actual = 0x" << actual
                   << "\n";
      LIBTB2_ERROR_ON(actual != expected);
    }
  }
  void t_stimulus() {
    resetter_.wait_reset_done();

    FifoMultiPushCmdConstraint c("FifoMultiPushCmdConstraint");
    while (true) {
      c.next();

      push_0_ = false;
      push_1_ = false;
      push_2_ = false;
      push_3_ = false;

      const uint32_t push = c.cmd->push;
#define BIT_IS_SET(__w, __b) ((((__w) >> (__b)) & 0x1) == true)

      if (BIT_IS_SET(full_r_, 0))
        goto __done;
      
      if (BIT_IS_SET(push, 0)) {
        push_0_ = true;
        push_0_data_ = c.cmd->push_0_data;

        expected_.push_back(c.cmd->push_0_data);

        st_.n++;
        st_.cnt[0]++;
      }

      if (BIT_IS_SET(full_r_, 1))
        goto __done;
      
      if (BIT_IS_SET(push, 1)) {
        push_1_ = true;
        push_1_data_ = c.cmd->push_1_data;
        
        expected_.push_back(c.cmd->push_1_data);

        st_.n++;
        st_.cnt[1]++;
      }
      
      if (BIT_IS_SET(full_r_, 2))
        goto __done;
      
      if (BIT_IS_SET(push, 2)) {
        push_2_ = true;
        push_2_data_ = c.cmd->push_2_data;
        
        expected_.push_back(c.cmd->push_2_data);

        st_.n++;
        st_.cnt[2]++;
      }
      
      if (BIT_IS_SET(full_r_, 3))
        goto __done;
      
      if (BIT_IS_SET(push, 3)) {
        push_3_ = true;
        push_3_data_ = c.cmd->push_3_data;

        expected_.push_back(c.cmd->push_3_data);

        st_.n++;
        st_.cnt[3]++;
      }
#undef BIT_IS_SET
      
    __done:
      wait(clk_.posedge_event());
    }
  }
  struct {
    void reset() {
      n = 0;
      cnt.resize(4);
      std::fill(cnt.begin(), cnt.end(), 0);
    }
    void report() {
      LOGGER(INFO) << std::dec << "n = " << n << " cnt = ";
      for (std::size_t i = 0; i < cnt.size(); i++)
        LOGGER(INFO) << cnt[i] << " ";
      LOGGER(INFO) << "\n";
    }
    std::size_t n;
    std::vector<std::size_t> cnt;
  } st_;
  std::deque<uint32_t> expected_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Sampler sampler_;
  libtb2::Resetter resetter_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
};
SC_MODULE_EXPORT(FifoMultiPushTb);

int sc_main(int argc, char **argv) {
  FifoMultiPushTb tb;
  return libtb2::Sim::start(argc, argv);
}
