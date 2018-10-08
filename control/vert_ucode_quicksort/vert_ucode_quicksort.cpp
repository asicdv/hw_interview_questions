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
#include <vector>
#include <deque>
#include <algorithm>
#include <sstream>
#include <iostream>

#include "vobj/Vvert_ucode_quicksort.h"
#include "vobj/Vvert_ucode_quicksort_vert_ucode_quicksort.h"

typedef Vvert_ucode_quicksort uut_t;
typedef uint32_t dat_t;
typedef std::vector<dat_t> queue_t;

template<typename FwdIt>
std::string vector_to_string(FwdIt begin, FwdIt end) {
  std::stringstream ss;
  ss << "[";
  if (begin != end)
    ss << " " << *begin;
  while (++begin != end)
    ss << ", " << *begin;
  ss << "]";
  return ss.str();
}

#define PORTS_UNSORTED_IN(__func)               \
  __func(unsorted_vld, bool)                    \
  __func(unsorted_sop, bool)                    \
  __func(unsorted_eop, bool)                    \
  __func(unsorted_dat, dat_t)

#define PORTS_UNSORTED_OUT(__func)              \
  __func(unsorted_rdy, bool)

#define PORTS_SORTED(__func)                    \
  __func(sorted_vld_r, bool)                    \
  __func(sorted_sop_r, bool)                    \
  __func(sorted_eop_r, bool)                    \
  __func(sorted_err_r, bool)                    \
  __func(sorted_dat_r, dat_t)

#define PORTS_MISC(__func)                      \
  __func(busy_r, bool)


namespace detail {

uint32_t range(uint32_t word, uint32_t hi, uint32_t lo = 0) {
  return (word >> lo) & ((1 << (hi - lo)) - 1);
}
bool bit (uint32_t word, uint32_t b) {
  return (word >> b) & 0x1;
}

} // namespace detail

struct Tracer : sc_core::sc_module {
  SC_HAS_PROCESS(Tracer);
  sc_core::sc_in<bool> clk;
  Tracer (Vvert_ucode_quicksort_vert_ucode_quicksort* qs,
          sc_core::sc_module_name mn = "tracer")
      : qs_(qs), sc_module(mn), clk("clk") {
    SC_METHOD(t_trace);
    sensitive << clk.neg();
    dont_initialize();
  }
 private:
  void t_trace() {
    if (qs_->da_adv) {
      ss.clear();

      ss << "[" << sc_core::sc_time_stamp() << "] ";
      disassemble(ss, qs_->da_inst_r);
      std::cout << ss.str() << "\n";
    }
  }
  void disassemble(std::ostream & os, const uint32_t inst) {
    const bool sel0 = detail::bit(inst, 11);
    const bool sel1 = detail::bit(inst, 3);
    const bool sel2 = detail::bit(inst, 7);
    const uint8_t r = detail::range(inst, 10, 8);
    const uint8_t s = detail::range(inst, 6, 4);
    const uint8_t u = detail::range(inst, 2, 0);
    const uint8_t a = detail::range(inst, 7, 0);
    switch (detail::range(inst, 15, 12)) {
      case 2: {
        if (sel0) {
          os << "POP R" << r;
        } else {
          os << "PUSH R" << u;
        }
      } break;
      case 4: {
        if (sel0) {
          // ST
          os << "ST R" << s << ", [R" << u << "]";
        } else {
          // LD
          os << "LD R" << r << ", [R" << u << "]";
        }
        os << (sel0 ? "ST" : "LD");
      } break;
      case 6: {
        const bool is_imm = (!sel0) && sel1;
        const bool is_spe = ( sel0);

        os << "MOV";
        if (is_imm) {
          os << "I";
        } else if (is_spe) {
          os << "S";
        }
        os << " R" << r << ", ";
        if (is_imm) {
          os << "u";
        } else if (is_spe) {
          os << "S" << u;
        } else {
          os << "R" << u;
        }
      } break;
      case 7: {
        os << (sel0 ? "SUB" : "ADD");
        if (sel1)
          os << "I";

        if (sel2)
          os << " R" << r << ", R" << s;
        else
          os << " 0";

        os << ", " << (sel1 ? "R" : "") << u;
      } break;
      case 12: {
        os << (sel0 ? "RET" : "CALL");
        if (!sel0)
          os << " " << a;
      } break;
      case 15: {
        os << (sel0 ? "EMIT" : "WAIT");
      } break;
    }
  };
  Vvert_ucode_quicksort_vert_ucode_quicksort* qs_;
  std::stringstream ss;
};

struct UnsortedIntf : sc_core::sc_interface {
  virtual void t_idle () = 0;
  virtual void t_push (const queue_t & d) = 0;
  virtual bool t_is_ready () const = 0;
  virtual void t_wait_until_ready () = 0;
};

struct UnsortedXactor : sc_core::sc_module, UnsortedIntf {
  sc_core::sc_in<bool> clk;
#define __func(__name, __type)                  \
  sc_core::sc_out< __type > __name;
  PORTS_UNSORTED_IN(__func)
#undef __func
#define __func(__name, __type)                  \
  sc_core::sc_in< __type > __name;
  PORTS_UNSORTED_OUT(__func)
#undef __func
  UnsortedXactor(sc_core::sc_module_name mn = "UnsortedXactor")
    : sc_core::sc_module(mn)
#define __func(__name, __type)                 \
    , __name ( #__name )
    PORTS_UNSORTED_OUT(__func)
    PORTS_UNSORTED_IN(__func)
#undef __func
  {}

  void t_idle() {
    unsorted_vld = false;
    unsorted_sop = false;
    unsorted_eop = false;
    unsorted_dat = 0;
  }

  void t_push(const queue_t & d) {
    for (std::size_t i = 0; i < d.size(); i++) {
      unsorted_vld = true;
      unsorted_sop = (i == 0);
      unsorted_eop = (i == (d.size() - 1));
      unsorted_dat = d[i];
      wait(clk.posedge_event());
    }
    t_idle();
  }

  bool t_is_ready() const {
    return unsorted_rdy;
  }

  void t_wait_until_ready() {
    while (!t_is_ready()) {
      wait(unsorted_rdy.posedge_event());
      if (!clk.posedge())
        wait(clk.posedge_event());
    }
  }
};

struct SortedMonitor : sc_core::sc_module {
  sc_core::sc_in<bool> clk;
#define __declare_ports(__name, __type)         \
  sc_core::sc_in<__type> __name;
  PORTS_SORTED(__declare_ports)
#undef __declare_ports
  
  SC_HAS_PROCESS(SortedMonitor);
  SortedMonitor(sc_core::sc_module_name mn = "SortedMonitor")
      : in_packet_(false) {
    
    SC_METHOD(m_capture_output);
    sensitive << clk.pos();
    dont_initialize();
  }

  void push_back(const queue_t & q) {
    expected_.push_back(q);
  }

private:
  void m_capture_output() {
    if (sorted_vld_r) {
      if (sorted_sop_r) {
        if (in_packet_)
          LOGGER(ERROR) << "Protocol violation: duplicate SOP.\n";
        
        in_packet_ = true;
        current_.clear();
      }
      current_.push_back(sorted_dat_r);
      if (sorted_eop_r) {
        if (!in_packet_)
          LOGGER(ERROR) << "Protocol violation: No preceding SOP for EOP.\n";
        
        in_packet_ = false;
        validate_current();
      }
    }
  }

  void validate_current() {
    if (expected_.size() == 0) {
      LOGGER(ERROR) << "Protocol violation: did not expect completion.\n";
      return ;
    }
    
    if (current_ == expected_.front()) 
      LOGGER(INFO) << "PASS\n";
    else {
      queue_t const & x = expected_.front();
      LOGGER(ERROR) << "FAIL\n";
      LOGGER(ERROR) << "Expected: "
                    << vector_to_string(x.begin(), x.end()) << "\n";
      LOGGER(ERROR) << "Actual: "
                    << vector_to_string(current_.begin(), current_.end()) << "\n";
      
    }
    expected_.pop_front();
  }

  bool in_packet_;
  queue_t current_;
  std::deque<queue_t> expected_;
};

struct VertUcodeQuicksortTb : libtb2::Top<uut_t> {
  SC_HAS_PROCESS(VertUcodeQuicksortTb);
  VertUcodeQuicksortTb(sc_core::sc_module_name mn = "t")
    : uut_("uut")
    , tracer_(uut_.vert_ucode_quicksort)
#define __construct_ports(__name, __type)       \
      , __name ## _(#__name)
    PORTS_UNSORTED_OUT(__construct_ports)
    PORTS_UNSORTED_IN(__construct_ports)
    PORTS_SORTED(__construct_ports)
    PORTS_MISC(__construct_ports)
#undef __construct_ports
    , clk_("clk")
    , rst_("rst")
  {
    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);

    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signal(__name, __type)           \
    uut_.__name(__name##_);
    PORTS_UNSORTED_OUT(__bind_signal)
    PORTS_UNSORTED_IN(__bind_signal)
    PORTS_SORTED(__bind_signal)
    PORTS_MISC(__bind_signal)
#undef __bind_signals

    unsortedxactor_.clk(clk_);
#define __bind(__name, __type)                  \
    unsortedxactor_.__name(__name ## _);
    PORTS_UNSORTED_IN(__bind)
    PORTS_UNSORTED_OUT(__bind)
#undef __bind
     monitor_.clk(clk_);
#define __bind(__name, __type)                  \
    monitor_.__name(__name ## _);
    PORTS_SORTED(__bind)
#undef __bind

    tracer_.clk(clk_);
    
    SC_THREAD(t_stimulus);
    register_uut(uut_);
    vcd_on();
  }

private:
  void t_stimulus() {
    wait_cycles(10);
    
    struct dat_constraint : scv_constraint_base {
      scv_smart_ptr<dat_t> d;
      SCV_CONSTRAINT_CTOR(dat_constraint) {
        SCV_CONSTRAINT((d() >= 0) && (d() < 1000));
      }
    } dat_constraint_c("dat_constraint_c");

    struct len_constraint : scv_constraint_base {
      scv_smart_ptr<int> i;
      SCV_CONSTRAINT_CTOR(len_constraint) {
        SCV_CONSTRAINT((i() > 0) && (i() < 16));
      }
    } len_constraint_c("len_constraint_c");

    std::vector<uint32_t> q;
    for (int i = 0; i < 10; i++) {
      unsortedxactor_.t_wait_until_ready();

      q.clear();
      len_constraint_c.next();
      for (int i = 0; i < *len_constraint_c.i; i++) {
        q.push_back(*dat_constraint_c.d);
        dat_constraint_c.next();
      }
      unsortedxactor_.t_push(q);
      std::sort(q.begin(), q.end());
      monitor_.push_back(q);
    }
    wait(100, SC_NS);
    sc_core::sc_stop();
  }

  void wait_cycles(std::size_t cycles = 1) {
    while (cycles-- > 0)
      wait(clk_.posedge_event());
  }

  libtb2::Resetter resetter_;
  //  libtb2::SimWatchDogCycles wd_;
  libtb2::Sampler sampler_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS_UNSORTED_OUT(__declare_signal)
  PORTS_UNSORTED_IN(__declare_signal)
  PORTS_SORTED(__declare_signal)
  PORTS_MISC(__declare_signal)
#undef __declare_signal
  UnsortedXactor unsortedxactor_;
  SortedMonitor monitor_;
  uut_t uut_;
  Tracer tracer_;
};
SC_MODULE_EXPORT(VertUcodeQuicksortTb);

int sc_main(int argc, char **argv) {
  VertUcodeQuicksortTb tb("tb");
  return libtb2::Sim::start(argc, argv);
}

