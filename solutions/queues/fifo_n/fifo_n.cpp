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
#include <deque>
#include "vobj/Vfifo_n.h"

#define PORTS(__func)                           \
  __func(push, bool)                            \
  __func(push_vq, uint32_t)                     \
  __func(push_data, uint32_t)                   \
  __func(pop, bool)                             \
  __func(pop_vq, uint32_t)                      \
  __func(pop_data_valid_r, bool)                \
  __func(pop_data_vq_r, uint32_t)               \
  __func(pop_data_w, uint32_t)                  \
  __func(empty_r, uint32_t)                     \
  __func(full_r, uint32_t)

const std::size_t N = 8;
const std::size_t VQ_N = 8;

struct FifoNTb : libtb2::Top<FifoNTb> {
  typedef Vfifo_n uut_t;
  
  SC_HAS_PROCESS(FifoNTb);
  FifoNTb(sc_core::sc_module_name mn = "t")
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
    SC_THREAD(t_push);
    SC_THREAD(t_pop);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();
  }
private:
  void m_checker() {
    if (!pop_data_valid_r_)
      return ;

    LIBTB2_ERROR_ON(vq_.empty());
    const std::size_t vq = vq_.front(); vq_.pop_front();
    const uint32_t expected = expected_[vq].front(); expected_[vq].pop_front();
    const uint32_t actual = pop_data_w_;
    LOGGER(INFO) << "Pop vq = " << vq
                 << " actual = " << std::hex << actual
                 << " expected = " << std::hex << expected
                 << "\n";
    LIBTB2_ERROR_ON(actual != expected);
  }
  void t_pop() {
    pop_ = false;
    resetter_.wait_reset_done();
    scv_smart_ptr<bool> ppop;
    scv_bag<bool> pop_bag;
    pop_bag.add(true, 10);
    pop_bag.add(false, 90);
    ppop->set_mode(pop_bag);

    while (true) {
      wait(clk_.posedge_event());
      pop_ = false;
      
      ppop->next();

      const bool pop = *ppop;
      if (!pop)
        continue;

      const uint32_t empty_r = libtb2::mask_bits<uint32_t>(empty_r_, VQ_N);
      if (libtb2::mask_bits(empty_r, VQ_N) == libtb2::ones<uint32_t>(VQ_N))
        continue;

      uint32_t pop_vq = libtb2::pickone(
          libtb2::mask_bits<uint32_t>(~empty_r, VQ_N));
      LIBTB2_ASSERT(pop_vq < VQ_N);
      LIBTB2_ASSERT(pop_vq != 0);
      pop_vq -= 1;
      
      pop_ = true;
      pop_vq_ = pop_vq;

      // RTL reports empty, however predicted model does not.
      LIBTB2_ERROR_ON(expected_[pop_vq].empty());
      vq_.push_back(pop_vq);
    }
  }
  void t_push() {
    push_ = false;
    
    scv_smart_ptr<bool> ppush;
    scv_smart_ptr<uint32_t> ppush_data;
    scv_bag<bool> push_bag;
    push_bag.add(true, 30);
    push_bag.add(false, 70);
    ppush->set_mode(push_bag);
    
    resetter_.wait_reset_done();
    while (true) {
      wait(clk_.posedge_event());
      push_ = false;
      
      ppush->next();
      ppush_data->next();
      
      const bool push = *ppush;
      const uint32_t push_data = *ppush_data;
      wait(sampler_.sample());
      const uint32_t full = libtb2::mask_bits<uint32_t>(full_r_, VQ_N);

      if (!push)
        continue;

      if (libtb2::mask_bits(full, VQ_N) == libtb2::ones<uint32_t>(VQ_N))
        continue;

      push_ = true;
      push_data_ = push_data;
      uint32_t push_vq = libtb2::pickone(libtb2::mask_bits(~full, VQ_N));
      LIBTB2_ASSERT(push_vq != 0);
      push_vq -= 1;
      
      push_vq_ = push_vq;

      // RTL does not indicate ful however predicted model does.
      LIBTB2_ERROR_ON(expected_[push_vq].size() == N);

      LOGGER(INFO) << "Push vq = " << push_vq
                   << " data = " << std::hex << push_data << "\n";
      expected_[push_vq].push_back(push_data);
    }
  }
  std::deque<uint32_t> expected_ [N];
  std::deque<std::size_t> vq_;
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
SC_MODULE_EXPORT(FifoNTb<uint32_t>);

int sc_main(int argc, char **argv) {
  FifoNTb tb;
  return libtb2::Sim::start(argc, argv);
}
