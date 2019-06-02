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
#include "vobj/Vrob.h"
#include <deque>
#include <map>
#include <set>

typedef Vrob uut_t;

#define PORTS(__func)                           \
  __func(alloc_vld, bool)                       \
  __func(alloc_data, uint32_t)                  \
  __func(alloc_rdy, bool)                       \
  __func(alloc_id, uint32_t)                    \
  __func(cmpl_vld, bool)                        \
  __func(cmpl_id, uint32_t)                     \
  __func(cmpl_data, uint32_t)                   \
  __func(retire_rdy, bool)                      \
  __func(retire_cmpl_data, uint32_t)            \
  __func(retire_alloc_data, uint32_t)           \
  __func(retire_vld, bool)                      \
  __func(clear, bool)                           \
  __func(idle_r, bool)                          \
  __func(full_r, bool)

class MachineModel {
  
  struct State {
    uint32_t alloc_data;
    uint32_t cmpl_data;
    uint32_t id;
  };
  typedef std::map<uint32_t, State *>::iterator s_it_t;

public:
  MachineModel()
  {}

  void apply_allocation(uint32_t id, uint32_t alloc_data) {
    if (s_.find(id) != s_.end())
      LOGGER(ERROR) << "Attempt to allocate in flight id = " << id << "\n";

    State * s = fp_.alloc();
    s->id = id;
    s->alloc_data = alloc_data;
    d_.push_back(s);
    s_.insert(std::make_pair(id, s));
  }

  void apply_completion(uint32_t id, uint32_t cmpl_data) {
    s_it_t it = s_.find(id);
    if (it == s_.end())
      LOGGER(ERROR) << "Completing unknown id = " << id << "\n";

    LIBTB2_ASSERT(it->id == id);
    State * s = it->second;
    s->cmpl_data = cmpl_data;
  };

  void check_retirement(uint32_t alloc_data, uint32_t cmpl_data) {
    State * s = d_.front();

    bool error = false;
    if (s->alloc_data != alloc_data) {
      error = true;
      LOGGER(ERROR) << "Allocation data mismatch:"
                    << " expected = " << s->alloc_data
                    << " actual = " << alloc_data
                    << "\n";
    }
    if (s->cmpl_data != cmpl_data) {
      error = true;
      LOGGER(ERROR) << "Completion data mismatch:"
                    << " expected = " << s->cmpl_data
                    << " actual = " << cmpl_data
                    << "\n";
    }

    if (!error)
      LOGGER(INFO) << "ID validated on retirement. ID = " << s->id << "\n";

    d_.pop_front();
    s_.erase(s_.find(s->id));
    fp_.free(s);
  }
private:
  libtb2::FreePool<State> fp_;
  std::map<uint32_t, State *> s_;
  std::deque<State *> d_;
};

struct RobTb : libtb2::Top<RobTb> {
  SC_HAS_PROCESS(RobTb);
  RobTb(sc_core::sc_module_name mn = "t")
    : uut_("uut")
  {
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

    SC_THREAD(t_allocation);
    SC_THREAD(t_completion);
    SC_METHOD(m_retirement);
    sensitive << sampler_.sample();
  }
private:
  void t_allocation() {
    scv_smart_ptr<uint32_t> palloc_data;
    alloc_vld_ = false;
    resetter_.wait_reset_done();
    while (true) {
      alloc_vld_ = false;
      wait_cycles(libtb2::random(2));
      sampler_.wait_for_sample();
      if (alloc_rdy_) {
        palloc_data->next();
        const uint32_t alloc_id = alloc_id_;
        const uint32_t alloc_data = *palloc_data;
        
        alloc_vld_ = true;
        alloc_data_ = alloc_data;

        mdl_.apply_allocation(alloc_id, alloc_data);
        active_ids_.insert(alloc_id);
        wait_cycles(1);
        LOGGER(INFO) << "Allocation of ID = "
                     << alloc_id << " DATA = " << alloc_data << "\n";
      }
    }
  }
  void t_completion() {
    scv_smart_ptr<uint32_t> pcmpl_data;
    cmpl_vld_ = false;
    resetter_.wait_reset_done();
    while (true) {
      cmpl_vld_ = false;
      wait_cycles(libtb2::random(3));
      if (active_ids_.size() != 0) {
        pcmpl_data->next();
        const uint32_t cmpl_data = *pcmpl_data;
        const uint32_t id =
          *libtb2::choose(active_ids_.begin(), active_ids_.end());

        cmpl_vld_ = true;
        cmpl_id_ = id;
        cmpl_data_ = cmpl_data;
        wait_cycles(1);
        LOGGER(INFO) << "Completion of ID = "
                     << id << " DATA = " << cmpl_data << "\n";
        mdl_.apply_completion(id, cmpl_data);
        std::set<uint32_t>::iterator it = active_ids_.find(id);
        LIBTB2_ASSERT(it != active_ids_.end());
        active_ids_.erase(it);
      }
    }
  }
  void m_retirement() {
    retire_rdy_ = true;
    if (retire_vld_) {
      mdl_.check_retirement(retire_alloc_data_, retire_cmpl_data_);
      LOGGER(INFO) << "Retiring ALLOC_DATA = "
                   << retire_alloc_data_ << " CMPL_DATA = " << retire_cmpl_data_
                   << "\n";
    }
  }
  void wait_cycles(std::size_t cycles = 1) {
    while (cycles--)
      wait(clk_.posedge_event());
  }
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  std::set<uint32_t> active_ids_;
  MachineModel mdl_;
  uut_t uut_;
};
SC_MODULE_EXPORT(RobTb);

int sc_main(int argc, char **argv) {
  RobTb tb;
  return libtb2::Sim::start(argc, argv);
}
