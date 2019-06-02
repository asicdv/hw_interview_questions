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
#include "vobj/Vfifo_sr.h"

#define PORTS(__func)                           \
  __func(push, bool)                            \
  __func(push_data, T)                          \
  __func(pop, bool)                             \
  __func(pop_data_valid, bool)                  \
  __func(pop_data, T)                           \
  __func(empty_r, bool)                         \
  __func(full_r, bool)

typedef Vfifo_sr uut_t;

template<typename T>
struct PushIntf : sc_core::sc_interface {
  virtual void idle() = 0;
  virtual void bpush(const T & t) = 0;
  virtual bool is_full() const = 0;
};

template<typename T>
struct PopIntf : sc_core::sc_interface {
  virtual void idle() = 0;
  virtual T bpop() = 0;
  virtual bool is_empty() const = 0;
};

template<typename T>
struct PushXactor : sc_core::sc_module, PushIntf<T> {
  sc_core::sc_in<bool> clk;

  sc_core::sc_in<bool> full_r;

  sc_core::sc_out<bool> push;
  sc_core::sc_out<T> push_data;
  
  SC_HAS_PROCESS(PushXactor);
  PushXactor(libtb2::Sampler & s, sc_core::sc_module_name mn = "pushxactor")
      : s_(s), full_r("full_r"), push("push"), push_data("push_data") {
  }
  void idle() {
    push = false;
    push_data = T();
  }
  void bpush(const T & t) {
    while (full_r)
      wait(clk.posedge_event());

    push = true;
    push_data = t;
    wait(clk.posedge_event());
    idle();
  }
  bool is_full() const { return full_r; }
 private:
  libtb2::Sampler & s_;
};

template<typename T>
struct PopXactor : sc_core::sc_module, PopIntf<T> {
  sc_core::sc_in<bool> clk;

  sc_core::sc_in<bool> empty_r;
  
  sc_core::sc_out<bool> pop;
  sc_core::sc_in<T> pop_data;
  
  SC_HAS_PROCESS(PopXactor);
  PopXactor(libtb2::Sampler & s, sc_core::sc_module_name mn = "popxactor")
      : s_(s), empty_r("empty_r"), pop("pop"), pop_data("pop_data") {
  }
  void idle() {
    pop = false;
  }
  T bpop() {
    while (empty_r)
      wait(empty_r.posedge_event());

    wait(s_.sample());
    const T t = pop_data;
    pop = true;
    wait(clk.posedge_event());
    idle();
    return t;
  }
  bool is_empty() const { return empty_r; }
 private:
  libtb2::Sampler & s_;
};

template<typename T>
struct FifoSrTb : libtb2::Top<FifoSrTb<T> > {
  SC_HAS_PROCESS(FifoSrTb);
  sc_core::sc_port<PushIntf<T> > push_intf_;
  sc_core::sc_port<PopIntf<T> > pop_intf_;
  FifoSrTb(sc_core::sc_module_name mn = "t")
      : uut_("uut")
      , x_push_(sampler_, "PushXactor")
      , x_pop_(sampler_, "PopXactor")
#define __declare_signals(__name, __type)       \
    , __name ## _(#__name)
    PORTS(__declare_signals)
#undef __declare_signals
  {
    //
    x_push_.clk(clk_);
    x_push_.full_r(full_r_);
    x_push_.push(push_);
    x_push_.push_data(push_data_);
    
    push_intf_.bind(x_push_);

    //
    x_pop_.clk(clk_);
    x_pop_.empty_r(empty_r_);
    x_pop_.pop(pop_);
    x_pop_.pop_data(pop_data_);
    
    pop_intf_.bind(x_pop_);
    //
    wd_.clk(clk_);
    //
    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
#define __bind_signals(__name, __type)          \
    uut_.__name(__name ## _);
    PORTS(__bind_signals)
#undef __bind_signals
        uut_.rst(rst_);
    uut_.clk(clk_);

    SC_THREAD(t_push);
    SC_THREAD(t_pop);

    st_.reset();
  }
 private:
  void t_push() {
    scv_smart_ptr<T> d;

    resetter_.wait_reset_done();
    while (true) {
      if (!push_intf_->is_full()) {
        d->next();
        push_intf_->bpush(*d);
        expected_.push_back(*d);
        st_.pushes++;
      } else {
        wait(clk_.posedge_event());
      }
    }
  }
  void t_pop() {
    const libtb2::Options & o = libtb2::Sim::get_options();

    resetter_.wait_reset_done();
    while (true) {
      if (!pop_intf_->is_empty()) {
        LIBTB2_ASSERT(!expected_.empty());
        
        const T actual = pop_intf_->bpop();
        const T expected = expected_.front(); expected_.pop_front();
        st_.pops++;
      
        LIBTB2_ERROR_ON(actual != expected);
        if (o.debug_on()) {
          LOGGER(DEBUG) << " Actual=" << actual
                        << " Expected=" << expected << "\n";
        }
      } else {
        wait(clk_.posedge_event());
      }
    }
  }
  void end_of_simulation() {
    LOGGER(INFO) << "Pushes: " << st_.pushes << "\n";
    LOGGER(INFO) << "Pops: " << st_.pops << "\n";
  }

  struct {
    void reset() {
      pushes = 0;
      pops = 0;
    }
    std::size_t pushes, pops;
  } st_;
  PushXactor<T> x_push_;
  PopXactor<T> x_pop_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signals(__name, __type)       \
  sc_core::sc_signal<__type> __name ## _;
  PORTS(__declare_signals)
#undef __declare_signals
  libtb2::Resetter resetter_;
  libtb2::Sampler sampler_;
  libtb2::SimWatchDogCycles wd_;
  uut_t uut_;
  std::deque<T> expected_;
};

int sc_main(int argc, char ** argv) {
  FifoSrTb<uint32_t> tb;
  return libtb2::Sim::start(argc, argv);
}
