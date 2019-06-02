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
#include <sstream>

#include "vobj/Vpack.h"

#define PORTS(__func)                           \
  __func(in_pass, bool)                         \
  __func(in_w, sc_bv<256>)                      \
  __func(in_vld_w, uint32_t)                    \
  __func(out_pass_r, bool)                      \
  __func(out_r, sc_bv<256>)                     \
  __func(out_vld_r, uint32_t)

struct PackTb : libtb2::Top<Vpack> {
  struct Expectation {
    Expectation() {
      n = 0;
      for (int i = 0; i < 8; i++) {
        x [i] = 0;
        valid [i] = false;
      }
    }
    std::string to_string() const {
      std::stringstream ss;
      ss << "{";
      for (int i = 0; i < 8; i++) {
        if (i != 0)
          ss << ",";
        
        ss << i << ":(v:" << valid [i]
           << " d:" << std::hex << x [i] << ")";
      }
      ss << ", expect: " << bv.to_string(SC_HEX)
         << "}";
      return ss.str();
    }
    void finalize() {
      bv = 0;
      for (int i = 0, j = 0; i < 8; i++) {
        if (valid [i])
          bv.set_word(j++, x [i]);
      }
    }
    uint32_t x [8];
    bool valid [8];
    sc_bv<256> bv;
    std::size_t n;
  };
  
  SC_HAS_PROCESS(PackTb);
  PackTb(sc_core::sc_module_name mn = "t")
      : uut_("uut")
#define __construct_ports(__name, __type)       \
        , __name ## _(#__name)
        PORTS(__construct_ports)
#undef __construct_ports
      , clk_("clk")
      , rst_("rst") {
    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals

    resetter_.clk(clk_);
    resetter_.rst(rst_);

    sampler_.clk(clk_);
    
    SC_METHOD(m_check_output);
    sensitive << sampler_.sample();
    dont_initialize();

    SC_THREAD(t_stimulus);

    generate_stimulus(1024 << 1);
    register_uut(uut_);
    vcd_on();

    error_ = false;
    j_ = 0;
  };
 private:
  void generate_stimulus(std::size_t cnt = 16) {
    scv_smart_ptr<bool> do_value;
    scv_smart_ptr<uint32_t> x_ptr;

    while (cnt-- != 0) {
      Expectation e;

      for (uint32_t i = 0; i < 8; i++, do_value->next()) {
        if (*do_value) {
          e.x [i] = *x_ptr;
          e.valid [i] = true;
          e.n++;

          x_ptr->next();
        }
      }
      e.finalize();

      expect_.push_back(e);
    }
  }
  void t_stimulus() {
    in_pass_ = false;
    in_w_ = 0;
    in_vld_w_ = 0;

    resetter_.wait_reset_done();
    wait(20, SC_NS);

    for (std::size_t i = 0; i < expect_.size(); i++) {
      const Expectation & e = expect_[i];

      uint32_t val_in_vld_w = 0;
      sc_bv<256> val_in_w;
      for (int j = 0; j < 8; j++) {
        if (e.valid [j])
          val_in_vld_w |= (1 << j);
        
        val_in_w.set_word(j, e.x[j]);
      }
      in_pass_ = true;
      in_w_ = val_in_w;
      in_vld_w_ = val_in_vld_w;
      wait(clk_.negedge_event());
      wait(clk_.posedge_event());
      in_pass_ = false;
      in_vld_w_ = 0;

      if (error_)
        break;
    }

    wait(100, SC_NS);
    sc_core::sc_stop();
  }
  void m_check_output() {
    if (out_pass_r_) {
      if (j_ >= expect_.size()) {
        error_ = true;

        std::cout << "***** ERROR: unexpected output.\n";
      }

      uint32_t out_vld_r = out_vld_r_.read();
      const std::size_t n = libtb2::popcount(out_vld_r);

      const Expectation & e = expect_[j_++];
      if (n != e.n) {
        error_ = true;
        std::cout << "**** ERROR: n = " << n << " != e.n = " << e.n << "\n";
      }
      sc_bv<256> out_r = out_r_.read();
      for (int i = 0; i < 8; i++) {
        if ((out_vld_r & (1 << i)) == 0) 
          out_r.set_word(i, 0);
      }
      
      if (out_r != e.bv) {
        error_ = true;
        
        std::cout << sc_core::sc_time_stamp()
                  << "**** ERROR: out mismatch"
                  << "\nACTUAL = " << out_r.to_string(SC_HEX)
                  << "\nEXPECT = " << e.to_string()
                  << "\n";
      }
    }
  }
  bool error_;
  std::size_t i_, j_;
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  std::deque<Expectation> expect_;
  Vpack uut_;
};
SC_MODULE_EXPORT(PackTb);

int sc_main(int argc, char **argv) {
  PackTb tb("tb");
  return libtb2::Sim::start(argc, argv);
}
