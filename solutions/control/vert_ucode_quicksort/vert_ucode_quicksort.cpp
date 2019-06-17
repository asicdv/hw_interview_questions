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

/*
#include "vobj/Vvert_ucode_quicksort.h"
#include "vobj/Vvert_ucode_quicksort_vert_ucode_quicksort.h"

typedef Vvert_ucode_quicksort uut_t;
typedef uint32_t dat_t;
typedef std::vector<dat_t> queue_t;

#define ASSERT(__cond) do {                     \
  if (!(__cond)) {                              \
    sc_core::sc_stop();                         \
  }                                             \
  } while (false)
  

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
  return (word >> lo) & ((1 << (hi - lo + 1)) - 1);
}
bool bit (uint32_t word, uint32_t b) {
  return (word >> b) & 0x1;
}

} // namespace detail

struct Tracer : sc_core::sc_module {
  enum {
    BLINK = 7
  };
  
  SC_HAS_PROCESS(Tracer);
  sc_core::sc_in<bool> clk;
  sc_core::sc_in<bool> rst;
  Tracer (Vvert_ucode_quicksort_vert_ucode_quicksort* qs,
          sc_core::sc_module_name mn = "tracer")
      : qs_(qs), sc_module(mn), clk("clk") {
    SC_METHOD(t_trace);
    sensitive << clk.neg();
    dont_initialize();

    m_.reset();
  }
 private:
  void t_trace() {
    if (rst)
      return;
    
    if (qs_->da_adv) {
      std::stringstream ss;
      ss << "[" << sc_core::sc_time_stamp() << "] ";
      const uint32_t addr = qs_->da_pc_r;
      ss << "0x" << std::hex << addr << ": ";
      disassemble(ss, (qs_->da_inst_r & 0xFFFF));
      ASSERT(check_writeback(ss));
      std::cout << ss.str() << "\n";
    }
  }
  bool check_writeback(std::ostream & os) {
    bool ret = true;

    if (qs_->da_rf___05Fwen_w) {
      const uint32_t waddr = qs_->da_rf___05Fwa_w;
      const uint32_t wdata = qs_->da_rf___05Fwdata_w;

      os << "\t(R" << waddr << " <= " << std::hex << wdata << ")";

      if (expected_.check_en) {

        if (expected_.wr_addr != waddr) {
          os << "  **** expected wr_addr = " << expected_.wr_addr << " != "
             << "actual wr_addr = " << waddr;
          ret = false;
        }

        if (expected_.wr_data != wdata) {
          os << "  **** expected wr_data = " << expected_.wr_data << " != "
             << "actual wr_data = " << wdata;
          ret = false;
        }
      }

      // Apply model update
      m_.regs[waddr] = wdata;
    }

    const bool sort__en = qs_->sort___05Fen;
    const bool sort__wen = qs_->sort___05Fwen;
    const uint32_t sort__addr = qs_->sort___05Faddr;
    const uint32_t sort__din = qs_->sort___05Fdin;
    const uint32_t sort__dout = qs_->sort___05Fdout;

    if (sort__en) {
      os << "   ";
      if (sort__wen) {
        os << "(mem[" << sort__addr << "] <= " << sort__din << ")";
      } else {
        //        os << "(mem[" << sort__addr << "] => " << sort__din << ")";
        os << "(mem[" << sort__addr << "] => X)";
      }
    }

    if (expected_.sets_flags) {
      os << " (ZNC <= "
         << (qs_->ar_flag_z_r == 0 ? '0' : '1')
         << (qs_->ar_flag_n_r == 0 ? '0' : '1')
         << (qs_->ar_flag_c_r == 0 ? '0' : '1')
         << ")"
        ;
    }

    const uint32_t actual_pc = qs_->da_pc_r;
    if (actual_pc != m_.pc) {
      os << "  **** expected pc = " << m_.pc << " != "
         << "actual pc = " << actual_pc;
      ret = false;
    }
    // Apply model update
    m_.pc = expected_.next_pc;
    
    return ret;
  }
  void disassemble(std::ostream & os, const uint32_t inst) {
    const bool sel0 = detail::bit(inst, 11);
    const bool sel1 = detail::bit(inst, 3);
    const bool sel2 = detail::bit(inst, 7);
    const uint32_t r = detail::range(inst, 10, 8);
    const uint32_t s = detail::range(inst, 6, 4);
    const uint32_t u = detail::range(inst, 2, 0);
    const uint32_t a = detail::range(inst, 7, 0);
    const uint32_t opcode = detail::range(inst, 15, 12);

    expected_.reset();
    switch (opcode) {
      case 1: {
        const uint32_t cc = detail::range(inst, 9, 8);
        os << "J";

        const bool z = (qs_->ar_flag_z_r != 0);
        const bool n = (qs_->ar_flag_n_r != 0);
        const bool c = (qs_->ar_flag_c_r != 0);
        
        bool taken = true;
        switch (cc) {
        case 1: {
          os << "EQ";
          
          taken = z;
        } break;
        case 2: {
          os << "GT";

          taken = !z && !n;
        } break;
        case 3: {
          os << "LE";

          taken = z || n;
        } break;
        }
        os << " " << a;

        if (taken)
          expected_.next_pc = a;
        else
          expected_.next_pc++;
      } break;
      case 2: {
        if (sel0) {
          ASSERT(m_.stack.size() != 0);

          os << "POP R" << r;

          expected_.check_en = true;
          expected_.wr_en = true;
          expected_.wr_addr = r;
          expected_.wr_data = m_.stack.back();
          m_.stack.pop_back();
        } else {
          os << "PUSH R" << u;

          m_.stack.push_back(m_.regs[u]);
        }
        expected_.next_pc++;
      } break;
      case 4: {
        expected_.check_en = false;
        if (sel0) {
          // ST
          os << "ST [R" << s << "], R" << u;

          m_.mem[m_.regs[u]] = s;
        } else {
          // LD
          os << "LD R" << r << ", [R" << u << "]";

          if (m_.mem.count(u) != 0) {
            expected_.check_en = m_.mem.count(m_.regs[u]);
            expected_.wr_en = true;
            expected_.wr_addr = r;
            if (expected_.check_en)
              expected_.wr_data = m_.mem[m_.regs[u]];
          }
        }
        expected_.next_pc++;
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
          os << u;
        } else if (is_spe) {
          os << "S" << u;
        } else {
          os << "R" << u;
        }

        expected_.check_en = !is_spe;
        expected_.next_pc++;
        expected_.wr_en = true;
        expected_.wr_addr = r;
        expected_.wr_data = is_imm ? u : m_.regs[u];
        expected_.is_special = is_spe;
      } break;
      case 7: {
        os << (sel0 ? "SUB" : "ADD");
        if (sel1)
          os << "I";

        if (sel2)
          os << " R" << r;
        else
          os << ".f 0";

        os << ", R" << s;
        os << ", " << (sel1 ? "" : "R") << u;

        const uint32_t a = m_.regs[s];
        const uint32_t b = sel1 ? u : m_.regs[u];
        
        expected_.check_en = true;
        expected_.next_pc++;
        expected_.wr_en = sel2;
        expected_.wr_addr = r;
        expected_.wr_data = sel0 ? (a - b) : (a + b);
        expected_.sets_flags = !sel2;
      } break;
      case 12: {
        os << (sel0 ? "RET" : "CALL");
        if (!sel0)
          os << " " << a;

        expected_.check_en = true;
        expected_.next_pc = sel0 ? m_.regs[BLINK] : a;
        if (!sel0) {
          expected_.wr_en = true;
          expected_.wr_addr = BLINK;
          expected_.wr_data = m_.pc + 1;
        }
      } break;
      case 15: {
        os << (sel0 ? "EMIT" : "WAIT");

        expected_.check_en = true;
        expected_.next_pc++;
      } break;
      default: {
        os << "INVALID OPCODE:" << (int)opcode;
      }
    }
  };
  struct {
    bool check_en;
    uint32_t next_pc;
    bool wr_en;
    uint32_t wr_addr;
    uint32_t wr_data;
    bool is_special;
    bool sets_flags;
    bool z,n,c;
    void init() {
      check_en = false;
      next_pc = 0;
      wr_en = false;
      is_special = false;
    }
    void reset() {
      check_en = true;
      wr_en = false;
      is_special = false;
      sets_flags = false;
    }
  } expected_;

  struct {
    void reset() {
      regs.clear();
      stack.clear();
      mem.clear();
      pc = 0;
    }
    std::map<uint32_t, uint32_t> regs;
    std::vector<uint32_t> stack;
    std::map<uint32_t, uint32_t> mem;
    uint32_t pc;
  } m_;
  Vvert_ucode_quicksort_vert_ucode_quicksort* qs_;
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
    tracer_.rst(rst_);
    
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
*/

#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include "vobj/Vvert_ucode_quicksort.h"
#include "vobj/Vvert_ucode_quicksort_vert_ucode_quicksort.h"

using word_type = uint32_t;

#define PORTS_UNSORTED_IN(__func)               \
  __func(unsorted_vld, bool)                    \
  __func(unsorted_sop, bool)                    \
  __func(unsorted_eop, bool)                    \
  __func(unsorted_dat, word_type)

#define PORTS_UNSORTED_OUT(__func)              \
  __func(unsorted_rdy, bool)

#define PORTS_SORTED(__func)                    \
  __func(sorted_vld_r, bool)                    \
  __func(sorted_sop_r, bool)                    \
  __func(sorted_eop_r, bool)                    \
  __func(sorted_err_r, bool)                    \
  __func(sorted_dat_r, word_type)

struct Stimulus {
  friend std::ostream & operator<<(std::ostream & os, const Stimulus & stim) {
    return os << "'{"
              << "sop=" << stim.sop() << ","
              << "eop=" << stim.eop() << ","
              << "dat=" << stim.dat()
              << "}";
  }
  Stimulus() : is_valid_(false) {}
  Stimulus(bool sop, bool eop, word_type dat)
      : sop_(sop), eop_(eop), dat_(dat), is_valid_(true) {}
  bool sop() const { return sop_; }
  bool eop() const { return eop_; }
  word_type dat() const { return dat_; }
 private:
  bool sop_, eop_, is_valid_;
  word_type dat_;
};

struct Expect {
  friend std::ostream & operator<<(std::ostream & os, const Expect & x) {
    return os << "'{"
              << "sop=" << x.sop() << ","
              << "eop=" << x.eop() << ","
              << "dat=" << x.dat()
              << "}";
  }
  friend bool operator==(const Expect & a, const Expect & b) {
    if (a.sop() != b.sop()) return false;
    if (a.eop() != b.eop()) return false;
    if (a.dat() != b.dat()) return false;

    return true;
  }
  Expect() : is_valid_(false) {}
  Expect(bool sop, bool eop, word_type dat)
      : sop_(sop), eop_(eop), dat_(dat), is_valid_(true) {}
  bool sop() const { return sop_; }
  bool eop() const { return eop_; }
  word_type dat() const { return dat_; }
 private:
  bool sop_, eop_, is_valid_;
  word_type dat_;
};

struct TOP : tb::Top {
  using stimulus_type = Stimulus;
  using expected_type = Expect;

  TOP() {
    v.rst(rst);
    v.clk(clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS_UNSORTED_IN(__bind_signals)
    PORTS_UNSORTED_OUT(__bind_signals)
    PORTS_SORTED(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void set_idle() {
    unsorted_vld = false;
    unsorted_sop = word_type{};
    unsorted_eop = word_type{};
    unsorted_dat = word_type{};
  }
  void t_set_stimulus(const stimulus_type & stim) {
    //    t_wait_not_busy();
    unsorted_vld = true;
    unsorted_sop = stim.sop();
    unsorted_eop = stim.eop();
    unsorted_dat = stim.dat();
    t_await_cycles();
    set_idle();
  }
  bool out_is_valid() const { return sorted_vld_r; }
  Expect get_expect() const {
    return Expect{sorted_sop_r, sorted_eop_r, sorted_dat_r};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_wait_not_busy() {
    while (!unsorted_rdy)
      t_await_cycles();
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vvert_ucode_quicksort v{"vert_ucode_quicksort"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
    PORTS_UNSORTED_IN(__declare_signal)
    PORTS_UNSORTED_OUT(__declare_signal)
    PORTS_SORTED(__declare_signal)
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

struct StimulusGenerator {
  StimulusGenerator(std::size_t n = 16) {
    while (n--)
      unsorted_.push_back(rnd());

    sorted_ = unsorted_;
    std::sort(sorted_.begin(), sorted_.end());
  }
  bool get_stimulus(Stimulus & s) {
    if (stim_i_ >= unsorted_.size())
      return false;
    const bool sop{stim_i_ == 0};
    const bool eop{stim_i_ == (unsorted_.size() - 1)};
    s = Stimulus{sop, eop, unsorted_[stim_i_]};
    ++stim_i_;
    return true;
  }
  bool get_expected(Expect & x) {
    if (expect_i_ >= sorted_.size())
      return false;
    const bool sop{expect_i_ == 0};
    const bool eop{expect_i_ == (sorted_.size() - 1)};
    x = Expect{sop, eop, sorted_[expect_i_]};
    ++expect_i_;
    return true;
  }
 private:
  std::size_t stim_i_{0}, expect_i_{0};
  std::vector<word_type> unsorted_, sorted_;
  tb::Random::UniformRandomInterval<word_type> rnd{127, 0};
};

TOP top;
tb::TaskRunner TaskRunner;

} // namespace

TEST(VertUcodeQuicksortTest, Basic) {
  const std::size_t n{1024 << 3};
  auto task = std::make_unique<
    tb::BasicPassValidNotBusyTask<TOP> >(top);

  for (std::size_t i = 0; i < 2; i++) {
    StimulusGenerator gen{16};

    Stimulus stim;
    while (gen.get_stimulus(stim))
      task->add_stimulus(stim);

    Expect x;
    while (gen.get_expected(x))
      task->add_expected(x);
  }
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}

