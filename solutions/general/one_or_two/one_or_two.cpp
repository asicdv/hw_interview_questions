//========================================================================== //
// Copyright (c) 2016-18, Stephen Henry
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
#include <sstream>
#include <map>
#include "vobj/Vone_or_two.h"

#define PORTS(__func)                           \
  __func(x, uint32_t)                           \
  __func(inv, bool)                             \
  __func(has_set_0, bool)                       \
  __func(has_set_1, bool)                       \
  __func(has_set_more_than_1, bool)

enum InState { ZERO, ONE, MULTI };

template<>
struct scv_extensions<InState> : public scv_enum_base<InState> {
  SCV_ENUM_CTOR(InState) {
    SCV_ENUM(ZERO);
    SCV_ENUM(ONE);
    SCV_ENUM(MULTI);
  }
};

template<typename T>
struct Stimulus : libtb2::RandomSrc<T> {
  SC_HAS_PROCESS(Stimulus);
  Stimulus(sc_core::sc_module_name mn = "Stimulus")
      : libtb2::RandomSrc<T>(mn) {

    scv_bag<InState> in_dist_;
    in_dist_.add(ZERO, 10);
    in_dist_.add(ONE, 50);
    in_dist_.add(MULTI, 70);
    in_state_->set_mode(in_dist_);

    rnd_bit_->keep_only(0, sizeof(T) * 4 - 1);
  }
 protected:
  void m_random() {
    in_state_->next();
    switch  (*in_state_) {
      case ZERO: {
        this->out = 0;
      } break;
      case ONE: {
        rnd_bit_->next();
        this->out = (1 << *rnd_bit_);
      } break;
      case MULTI: {
        this->rnd_->next();
        this->out = *this->rnd_;
      };
    }
  }
  scv_smart_ptr<InState> in_state_;
  scv_smart_ptr<std::size_t> rnd_bit_;
};

class OneOrTwoTb : libtb2::Top<OneOrTwoTb> {
  typedef Vone_or_two uut;
 public:
  SC_HAS_PROCESS(OneOrTwoTb);
  OneOrTwoTb(sc_core::sc_module_name = "t")
      : uut_("uut"), clk_("clk"), wd_(1000, "wd") {

    //
    rnd_x_.clk(clk_);
    rnd_x_.out(x_);
    //
    rnd_inv_.clk(clk_);
    rnd_inv_.out(inv_);
    //
    wd_.clk(clk_);
    
#define __bind(__name, __type)                  \
    uut_.__name(__name##_);
    PORTS(__bind)
#undef __bind

    SC_METHOD(m_checker);
    sensitive << clk_.negedge_event();
    dont_initialize();
  }
 private:

  void m_checker() {
    switch (libtb2::popcount(inv_ ? ~x_ : x_)) {
      case 0: {
        LIBTB2_ERROR_ON(!has_set_0_);
        LIBTB2_ERROR_ON( has_set_1_);
        LIBTB2_ERROR_ON( has_set_more_than_1_);
        
        dist_[ZERO]++;
      } break;
      case 1: {
        LIBTB2_ERROR_ON( has_set_0_);
        LIBTB2_ERROR_ON(!has_set_1_);
        LIBTB2_ERROR_ON( has_set_more_than_1_);
        
        dist_[ONE]++;
      } break;
      default: {
        LIBTB2_ERROR_ON( has_set_0_);
        LIBTB2_ERROR_ON( has_set_1_);
        LIBTB2_ERROR_ON(!has_set_more_than_1_);
        
        dist_[MULTI]++;
      } break;
    }
    trace();
  }

  void end_of_simulation() {
    typedef std::map<int, std::size_t>::const_iterator it_t;
    for (it_t it = dist_.begin(); it != dist_.end(); ++it) {
      std::cout << it->first << " " << it->second << "\n";
    }
  }
 private:
  void trace() {
    libtb2::Render r;
    r.add("x", x_);
    r.add("inv", inv_);
    r.add("has_set_0", has_set_0_);
    r.add("has_set_1", has_set_1_);
    r.add("has_set_more_than_1", has_set_more_than_1_);
    r.render(std::cout);
  }
  Stimulus<uint32_t> rnd_x_;
  libtb2::RandomSrc<bool>  rnd_inv_;
  libtb2::SimWatchDogCycles wd_;
  sc_core::sc_clock clk_;
  scv_smart_ptr<InState> in_state_;
  
#define __signals(__name, __type)               \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__signals)
#undef __signals
  uut uut_;
  std::map<int, std::size_t> dist_;
};
 SC_MODULE_EXPORT(OneOrTwoTb);

int sc_main(int argc, char **argv)
{
  OneOrTwoTb t;
  return libtb2::Sim::start(argc, argv);
}
