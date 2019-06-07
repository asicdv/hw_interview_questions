//========================================================================== //
// Copyright (c) 2019, Stephen Henry
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

#ifndef __SCPRIMITIVES_HPP__
#define __SCPRIMITIVES_HPP__

#include <systemc>
#include <vector>
#include <memory>
#include <string>
#include "random.hpp"

namespace tb {

class RandomBool : ::sc_core::sc_module {
 public:
  ::sc_core::sc_in<bool> clk{"clk"};
  ::sc_core::sc_out<bool> out{"out"};

  SC_HAS_PROCESS(RandomBool);
  RandomBool(::sc_core::sc_module_name mn = "RandomBool", float p = 0.5f);

  void set_p(float p) { p_ = p; }
 private:
  void m_stimulus();
  Random::UniformRandomInterval<uint32_t> rnd{1023, 0};
  float p_;
};

template<typename OUT, std::size_t NUM>
class MultiRandomBool : ::sc_core::sc_module {
 public:
  ::sc_core::sc_in<bool> clk{"clk"};
  ::sc_core::sc_out<OUT> out{"out"};

  SC_HAS_PROCESS(MultiRandomBool);
  MultiRandomBool(::sc_core::sc_module_name mn = "MultiRandomBool") : sc_module(mn) {
    const std::string prefix{"RandomBool"};
    b_.resize(NUM);
    for (std::size_t i = 0; i < NUM; i++) {
      const std::string s{prefix + std::to_string(i)};
      b_[i] = std::make_unique<RandomBool>(s.c_str());
      b_[i]->clk(clk);
      b_[i]->out(b_out_[i]);
    }
      
    SC_METHOD(m_set_out);
    for (std::size_t i = 0; i < NUM; i++)
      sensitive << b_out_[i].value_changed_event();
  }

  void set_p(std::size_t b, float p) { b_[b]->set_p(p); }
 private:
  void m_set_out() {
    OUT p{};
    for (std::size_t i = 0; i < NUM; i++) {
      if (b_[i]->out)
        set_bit(p, i);
    }
    out = p;
  }
  ::sc_core::sc_signal<bool> b_out_[NUM];
  std::vector<std::unique_ptr<RandomBool>> b_;
};

} // namespace tb

#endif
