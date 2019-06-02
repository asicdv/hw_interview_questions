//========================================================================== //
// Copyright (c) 2017, Stephen Henry
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
#include "vobj/Vfused_multiply_add.h"

#define PORTS(__func)                           \
  __func(cntrl_load, bool)                      \
  __func(cntrl_init, T)                         \
  __func(pass, bool)                            \
  __func(m, T)                                  \
  __func(x, T)                                  \
  __func(c, T)                                  \
  __func(y_valid_r, bool)                       \
  __func(y_w, T)

typedef Vfused_multiply_add uut_t;

namespace opts {

static int ACCUMULATION_N = 100;

} // namespace opts

template<typename T = uint32_t>
struct Machine {
  Machine() : y_(0) {}
  void init(T i) { y_ = i; }
  void apply(T m, T x, T c) {
    y_ += (m * x) + c;
  }
  T y() const { return y_; }
 private:
  T y_;
};

template<typename T>
struct FusedMultiplyAddTransactorIntf : sc_core::sc_interface {
  virtual void init(const T & t) = 0;
  virtual void issue(const T & m, const T & x, const T & c) = 0;
};

template<typename T>
struct FusedMultiplyAddTransactor
    : FusedMultiplyAddTransactorIntf<T>, sc_core::sc_module {
  //
  sc_core::sc_in<bool> clk;
  sc_core::sc_in<bool> rst;
  //
  sc_core::sc_out<bool> cntrl_load;
  sc_core::sc_out<T> cntrl_init;
  //
  sc_core::sc_out<bool> pass;
  sc_core::sc_out<T> m;
  sc_core::sc_out<T> x;
  sc_core::sc_out<T> c;
  //
  sc_core::sc_in<bool> y_valid_r;
  sc_core::sc_in<T> y_w;

  FusedMultiplyAddTransactor(sc_core::sc_module_name mn = "t")
      : clk("clk"), rst("rst")
#define __declare_ports(__name, __type)        \
      , __name(#__name)
      PORTS(__declare_ports)
#undef __declare_ports
  {}
  void end_of_elaboration() { idle(); }
  void init(const T & t) {
    cntrl_load = true;
    cntrl_init = t;
    wait_posedge_clk();
    idle();
  }
  void issue(const T & m_upt, const T & x_upt, const T & c_upt) {
    pass = true;
    m = m_upt;
    x = x_upt;
    c = c_upt;
    wait_posedge_clk();
    idle();
  }
 private:
  void wait_posedge_clk() { wait(clk.posedge_event()); }
  void idle() {
    //
    cntrl_load = false;
    cntrl_init = T();
    //
    pass = false;
    m = T();
    x = T();
    c = T();
  }
};

struct FusedMultiplyAddTb : libtb2::Top<FusedMultiplyAddTb> {
  typedef uint32_t T;
  
  sc_core::sc_port<FusedMultiplyAddTransactorIntf<T> > p;
  SC_HAS_PROCESS(FusedMultiplyAddTb);
  FusedMultiplyAddTb(sc_core::sc_module_name mn = "t")
      : uut_("uut")
      , rst_("rst")
      , clk_("clk")
      , p("p")
#define __construct_ports(__name, __type)       \
      , __name ## _(#__name)
      PORTS(__construct_ports)
#undef __construct_ports
  {
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    xact_.clk(clk_);
    xact_.rst(rst_);
    p.bind(xact_);

    //
    uut_.clk(clk_);
    uut_.rst(rst_);
    
#define __bind_ports(__name, __type)            \
    xact_.__name(__name ## _);                  \
    uut_.__name(__name ## _);
    PORTS(__bind_ports)
#undef __bind_ports
    wd_.clk(clk_);
    sampler_.clk(clk_);

    SC_THREAD(t_stimulus);
    SC_METHOD(m_model);
    sensitive << sampler_.sample();
    dont_initialize();
  }
 private:
  void t_stimulus() {
    scv_smart_ptr<bool> pass_;
    struct cntrl_init_constraint : scv_constraint_base {
      scv_smart_ptr<T> p;
      SCV_CONSTRAINT_CTOR(cntrl_init_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < 10000));
      }
    };
    cntrl_init_constraint cntrl_init_c("cntrl_init_constraint");
    struct x_constraint : scv_constraint_base {
      scv_smart_ptr<T> p;
      SCV_CONSTRAINT_CTOR(x_constraint) {
        SCV_CONSTRAINT((p() >= 0) && (p() < 1000));
      }
    } m_c("m_c"), x_c("x_c"), c_c("c_c");

    resetter_.wait_reset_done();

    while (true) {
      cntrl_init_c.next();
      const T cntrl_init = *cntrl_init_c.p;

      LOGGER(INFO) << "Loading MAC, init=" << cntrl_init << "\n";
      p->init(cntrl_init);
      for (int i = 0; i < opts::ACCUMULATION_N; i++) {
        m_c.next(); x_c.next(); c_c.next();

        const T m = *m_c.p;
        const T x = *x_c.p;
        const T c = *c_c.p;

        LOGGER(INFO) << "m = " << m << " x = " << x << " c = " << c << "\n";
        p->issue(m, x, c);
      }
    }
  }
  void m_model() {
    if (y_valid_r_) {
      // Validate output
      const T actual = y_w_;
      const T expected = mdl_.y();
      LIBTB2_ERROR_ON(actual != expected);

      LOGGER(DEBUG) << "actual = " << actual  << " expected = " << expected << "\n";
    }
    if (cntrl_load_) { mdl_.init(cntrl_init_); }
    if (pass_) { mdl_.apply(m_, x_, c_); }
  }
  Machine<T> mdl_;
  FusedMultiplyAddTransactor<T> xact_;
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
};
SC_MODULE_EXPORT(FusedMultiplyAddTb);

int sc_main(int argc, char **argv) {
  FusedMultiplyAddTb tb;
  return libtb2::Sim::start(argc, argv);
}
