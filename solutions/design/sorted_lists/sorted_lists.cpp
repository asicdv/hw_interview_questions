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
#include <list>
#include <deque>
#include <algorithm>
#include <iterator>
#include <set>
#include <limits>
#include <sstream>
//
#include "vobj/Vsorted_lists.h"
typedef Vsorted_lists uut_t;

#define PORTS(__func)                           \
  __func(upt_vld, bool)                         \
  __func(upt_id, sl::id_t)                      \
  __func(upt_op, sl::op_t)                      \
  __func(upt_key, sl::key_t)                    \
  __func(upt_size, sl::size_t)                  \
  __func(upt_error_vld_r, bool)                 \
  __func(upt_error_id_r, sl::id_t)              \
  __func(qry_vld, bool)                         \
  __func(qry_id, sl::id_t)                      \
  __func(qry_level, sl::level_t)                \
  __func(qry_resp_vld_r, bool)                  \
  __func(qry_key_r, sl::key_t)                  \
  __func(qry_size_r, sl::size_t)                \
  __func(qry_error_r, bool)                     \
  __func(qry_listsize_r, sl::listsize_t)        \
  __func(qry_id_r, sl::id_t)                    \
  __func(ntf_vld_r, bool)                       \
  __func(ntf_id_r, sl::id_t)                    \
  __func(ntf_key_r, sl::key_t)                  \
  __func(ntf_size_r, sl::size_t)

enum Op {
  OP_CLEAR = 0, OP_ADD = 1, OP_DELETE = 2, OP_REPLACE = 3
};

template<>
struct scv_extensions<Op> : scv_enum_base<Op> {
  SCV_ENUM_CTOR(Op) {
    SCV_ENUM(OP_CLEAR);
    SCV_ENUM(OP_ADD);
    SCV_ENUM(OP_DELETE);
    SCV_ENUM(OP_REPLACE);
  }
};

const char * op_to_string(int op) {
  switch (op) {
    case OP_CLEAR: return "OP_CLEAR";
    case OP_ADD: return "OP_ADD";
    case OP_DELETE: return "OP_DELETE";
    case OP_REPLACE: return "OP_REPLACE";
    default: return "<UNKNOWN OP>";
  }
}

namespace opts {

const int UPDATES = 100000;
const int N = 4;
const int M = 64;

} // namespace opts

namespace sl {

typedef uint32_t id_t;
typedef uint32_t op_t;
typedef vluint64_t key_t;
typedef uint32_t size_t;
typedef uint32_t listsize_t;
typedef uint32_t level_t;

struct Query {
  id_t id;
  level_t l;
  bool error_expected;
  std::string to_string() const {
    libtb2::Render r;
    r.add("id", id);
    r.add("l", l);
    r.add("error_expected", error_expected);
    return r.to_string();
  }
};

struct Update {
  Update()
  {}
  Update(id_t id, op_t op, key_t key, size_t size)
      : id(id), op(op), k(key), s(size)
  {}
  id_t id;
  op_t op;
  key_t k;
  size_t s;
  std::string to_string() const {
    libtb2::Render r;
    r.add("id", id);
    r.add("op", op_to_string(op));
    r.add("k", k);
    r.add("s", s);
    return r.to_string();
  }
};

struct Notify {
  Notify()
  {}
  Notify(id_t id, key_t key, size_t size)
      : id(id), key(key), size(size)
  {}
  id_t id;
  key_t key;
  size_t size;
  std::string to_string() const {
    libtb2::Render r;
    r.add("id", id);
    r.add("key", key);
    r.add("size", size);
    return r.to_string();
  }
};

struct Entry {
  friend bool operator<(const Entry & a, const Entry & b);
  
  Entry(key_t k, size_t s) : key(k), size(s) {}
  key_t key;
  size_t size;
  std::string to_string() const {
    libtb2::Render r;
    r.add("key", key);
    r.add("size", size);
    return r.to_string();
  }
};
bool operator<(const Entry & a, const Entry & b) {
  return (a.key > b.key);
}

struct QueryResult {
  friend bool operator==(const QueryResult &l, const QueryResult &r);

  QueryResult()
  {}
  
  QueryResult(key_t k, size_t s, listsize_t l, bool e, id_t id)
      : key(k), size(s), listsize(l), error(e), id(id)
  {}
  
  key_t key;
  size_t size;
  listsize_t listsize;
  bool error;
  id_t id;

  std::vector<Entry> dbg_;
  Query query;

  std::string to_string() const {
    libtb2::Render r;
    r.add("key", key);
    r.add("size", size);
    r.add("listsize", listsize);
    r.add("error", error);
    r.add("id", id);
    //    r.add("query", query.to_string());
    return r.to_string();
  }
};

bool operator==(const QueryResult &l, const QueryResult &r) {
  bool eq = true;

  // If query has errored, all bets are off.
  if ((r.error == l.error) && r.error)
    return true;

  eq &= (r.key == l.key);
  eq &= (r.size == l.size);
  eq &= (r.listsize == l.listsize);
  return eq;
}

typedef std::vector<Entry> table_entry_t;
typedef table_entry_t list_table_t [::opts::M];
    
struct KeyFinder {
  KeyFinder(sl::key_t k) : k_(k) {}
  bool operator()(const Entry & e) { return (k_ == e.key); }
 private:
  key_t k_;
};

} // namespace sl

class MachineModel {
  
 public:
  void update_actives() {
    std::stringstream ss;
    active_updates_.clear();
    for (int i = 0; i < 20; i++) {
      const int active = libtb2::random(opts::M - 1);
      active_updates_.insert(active);
      ss << active << " ";
    }
    LOGGER(DEBUG) << "New actives: " << ss.str() << "\n";
  }

  // Construct a random query based upon the known state of the machine.
  //
  sl::Update random_update() {
    sl::Update u;

    // If the ACTIVE UPDATE set is non-empty, ensure that ID is a random
    // entry in the SET. We cannot touch ID outside of this range.
    if (active_updates_.size() != 0)
      u.id = *libtb2::choose(active_updates_.begin(), active_updates_.end());
    else
      u.id = libtb2::random(opts::M - 1);

    u.op = random_update_op(u.id, u.k);
    u.s = libtb2::random<uint32_t>();
    return u;
  }

  sl::Query random_query() {
    sl::Query q;

    const bool allow_error = libtb2::random_bool(0.1f);
    for (int i = 0; i < 100; i++) {
      int id;

      // Specifically choose an ID outside of the ACTIVE UPDATE set.
      //
      while (true) {
        id = libtb2::random(opts::M - 1);
        if (active_updates_.find(id) == active_updates_.end())
          break;
      }
      const std::size_t sz = t_[id].size();
      if ((sz != 0) || allow_error) {
        const sl::level_t l = libtb2::random(opts::N - 1);
        if ((l < sz) || allow_error) {
          q.id = id;
          q.l = l;
          q.error_expected = allow_error && ((sz == 0) || (l <= sz));
          return q;
        }
      }
    }
    return q;
  }

  void apply_query(const sl::Query & q, sl::QueryResult & qr) {
    typedef std::vector<sl::Entry>::const_iterator table_entry_it_t;
    
    LOGGER(DEBUG) << "Applying query: " << q.to_string() << "\n";

    std::vector<sl::Entry> & es = t_[q.id];
    qr.key = 0;
    qr.size = 0;
    qr.listsize = 0;
    qr.error = 0;

    if (q.l >= es.size()) {
      qr.error = true;
      return;
    }

    // Sort by key
    std::sort(es.begin(), es.end());
    table_entry_it_t it = es.begin();
    std::advance(it, q.l);
    qr.key = it->key;
    qr.size = it->size;
    qr.listsize = es.size();
    qr.dbg_ = es;
    qr.id = q.id;
  }

  bool update(const sl::Update & u) {
    typedef std::vector<sl::Entry>::iterator table_entry_it_t;

    bool error = false;
    switch (u.op) {
      case OP_CLEAR: {
        t_[u.id].clear();
      } break;

      case OP_ADD: {
        if (t_[u.id].size() < opts::N)
          t_[u.id].push_back(sl::Entry(u.k,u.s));
        else
          error = true;
      } break;

      case OP_DELETE: {
        table_entry_it_t it =
            std::find_if(t_[u.id].begin(), t_[u.id].end(), sl::KeyFinder(u.k));

        if (it != t_[u.id].end())
          t_[u.id].erase(it);
        else {
          LOGGER(DEBUG) << "Key not found for id = " << u.id << "\n";
          error = true;
        }
      } break;

      case OP_REPLACE: {
        table_entry_it_t it =
            std::find_if(t_[u.id].begin(), t_[u.id].end(), sl::KeyFinder(u.k));
        if (it != t_[u.id].end())
          it->size = u.s;
        else
          error = true;
      } break;
    }
    
    LOGGER(DEBUG) << "Apply update id = " << u.id << "\n";
    for (std::size_t i = 0; i < t_[u.id].size(); i++)
      LOGGER(DEBUG) << i << " " << t_[u.id][i].to_string() << "\n";

    return error;
  }

 private:

  // Intelligently construct a opcode based upon the current machine
  // with appropriate weights where required.
  //
  sl::op_t random_update_op(sl::id_t id, sl::key_t & k) {
    sl::table_entry_t & es = t_[id];

    sl::op_t r = OP_CLEAR;
    k = 0;

    const bool allow_error = libtb2::random_bool(0.1f);

    scv_smart_ptr<Op> opp;
    scv_bag<Op> opbg;
    opbg.add(OP_ADD, 10);
    opbg.add(OP_DELETE, 10);
    opbg.add(OP_REPLACE, 10);
    opp->set_mode(opbg);

    const int ROUNDS = 10;
    for (int i = 0; i < ROUNDS; i++) {
      const std::size_t sz = es.size();

      opp->next();
      const Op op = *opp;

      switch (op) {
        case OP_ADD: {
          if (sz < opts::N || allow_error) {
            k = libtb2::random<uint32_t>();
            r = OP_ADD;

            goto __end;
          }
        } break;
        case OP_DELETE: {
          if (sz != 0 || allow_error) {
            if (!allow_error)
              k = libtb2::choose(es.begin(), es.end())->key;
            else
              k = 0;
            r = OP_DELETE;

            goto __end;
          }
        } break;
        case OP_REPLACE: {
          if (sz != 0 || allow_error) {
            if (!allow_error)
              k = libtb2::choose(es.begin(), es.end())->key;
            r = OP_REPLACE;

            goto __end;
          }
        } break;
        case OP_CLEAR: {
          // NOP
        } break;
      }
    }
 __end:
    return r;
  }

  sl::list_table_t t_;
  std::set<sl::id_t> active_updates_;
};

struct QryIntf : sc_core::sc_interface {
  virtual void issue(const sl::Query & q) = 0;
};

struct QryXactor : sc_core::sc_module, QryIntf {
#define QRY_PORTS(__func)                       \
  __func(qry_vld, bool)                         \
  __func(qry_id, uint32_t)                      \
  __func(qry_level, uint32_t)

  sc_core::sc_in<bool> clk;
#define __declare_ports(__name, __type)         \
  sc_core::sc_out<__type> __name;
  QRY_PORTS(__declare_ports)
#undef __declare_ports
  QryXactor(sc_core::sc_module_name mn = "QryXactor")
    : sc_core::sc_module(mn)
    , clk("clk")
#define __construct_ports(__name, __type)       \
    , __name(#__name)
      QRY_PORTS(__construct_ports)
#undef __construct_ports
  {}
  void issue(const sl::Query & q) {
    qry_vld = true;
    qry_id = q.id;
    qry_level = q.l;
    wait_cycle();
    LOGGER(INFO) << "Query issue: " << q.to_string() << "\n";
    idle();
  }
 private:
  void idle() {
    qry_vld = 0;
    qry_id = 0;
    qry_level = 0;
  }
  void wait_cycle() { wait(clk.posedge_event()); }
#undef QRY_PORTS
};

struct NotifyChecker : sc_core::sc_module {
#define NOTIFY_PORTS(__func)                    \
  __func(ntf_vld_r, bool)                       \
  __func(ntf_id_r, uint32_t)                    \
  __func(ntf_key_r, sl::key_t)                  \
  __func(ntf_size_r, uint32_t)
  
  sc_core::sc_in<bool> clk;
#define __declare_ports(__name, __type)         \
  sc_core::sc_in<__type> __name;
  NOTIFY_PORTS(__declare_ports)
#undef __declare_ports
  SC_HAS_PROCESS(NotifyChecker);
  NotifyChecker(MachineModel & m, sc_core::sc_module_name mn = "NotifyChecker")
      : m_(m), sc_core::sc_module(mn), clk("clk")
#define __construct_ports(__name, __type)       \
      , __name(#__name)
      NOTIFY_PORTS(__construct_ports)
#undef __construct_ports
  {
    sampler_.clk(clk);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();
  }
 private:
  void m_checker() {
    if (ntf_vld_r) {
      const sl::Notify n(ntf_id_r, ntf_key_r, ntf_size_r);
      LOGGER(INFO) << "Notify raised: " << n.to_string() << "\n";
    }
  }
  MachineModel & m_;
  libtb2::Sampler sampler_;
};

struct QueryChecker : sc_core::sc_module {
#define QUERY_PORTS(__func)                     \
  __func(qry_resp_vld_r, bool)                  \
  __func(qry_key_r, sl::key_t)                  \
  __func(qry_size_r, uint32_t)                  \
  __func(qry_error_r, bool)                     \
  __func(qry_listsize_r, uint32_t)              \
  __func(qry_id_r, uint32_t)
  
  sc_core::sc_in<bool> clk;
#define __declare_ports(__name, __type)         \
  sc_core::sc_in<__type> __name;
  QUERY_PORTS(__declare_ports)
#undef __declare_ports
  SC_HAS_PROCESS(QueryChecker);
  QueryChecker(MachineModel & m, sc_core::sc_module_name mn = "QueryChecker")
      : m_(m), sc_core::sc_module(mn), clk("clk")
#define __construct_ports(__name, __type)       \
      , __name(#__name)
      QUERY_PORTS(__construct_ports)
#undef __construct_ports
  {
    sampler_.clk(clk);
    SC_METHOD(m_checker);
    sensitive << sampler_.sample();
    dont_initialize();
  }
  void add_pending_result(const sl::QueryResult & q) {
    r_list_.push_back(q);
  }
 private:
  void m_checker() {
    if (qry_resp_vld_r) {
      LIBTB2_ERROR_ON(r_list_.size() == 0);

      const sl::QueryResult expected = r_list_.front();
      r_list_.pop_front();
      const sl::QueryResult actual(
          qry_key_r, qry_size_r, qry_listsize_r, qry_error_r, qry_id_r);

      if (!(expected == actual)) {
        LOGGER(ERROR) << "Mismatch detected: "
                      << " Actual:" << actual.to_string()
                      << " Expected:" << expected.to_string()
                      << "\n";

        // Report LIST state.
        for (std::size_t i = 0; i < expected.dbg_.size(); i++)
          LOGGER(DEBUG) << i << " " << expected.dbg_[i].to_string() << "\n";
          
      } else {
        LOGGER(INFO)
            << "Query response validated:" << actual.to_string() << "\n";
      }
    }
  }
  MachineModel & m_;
  libtb2::Sampler sampler_;
  std::deque<sl::QueryResult> r_list_;
#undef QUERY_PORTS
};

struct UptIntf : sc_core::sc_interface {
  virtual void issue(const sl::Update & u) = 0;
};

struct UptXactor : sc_core::sc_module, UptIntf {
#define UPT_PORTS(__func)                       \
  __func(upt_vld, bool)                         \
  __func(upt_id, uint32_t)                      \
  __func(upt_op, uint32_t)                      \
  __func(upt_key, sl::key_t)                    \
  __func(upt_size, uint32_t)

  sc_core::sc_in<bool> clk;
#define __declare_ports(__name, __type)         \
  sc_core::sc_out<__type> __name;
  UPT_PORTS(__declare_ports)
#undef __declare_ports

  UptXactor(sc_core::sc_module_name mn = "UptXactor")
    : sc_core::sc_module(mn)
    , clk("clk")
#define __construct_ports(__name, __type)       \
      , __name(#__name)
      UPT_PORTS(__construct_ports)
#undef __construct_ports
  {}
  void issue(const sl::Update & u) {
    upt_vld = true;
    upt_id = u.id;
    upt_op = u.op;
    upt_key = u.k;
    upt_size = u.s;
    wait_cycles();
    LOGGER(INFO) << "Update issue: " << u.to_string() << "\n";
    idle();
    // 
    wait_cycles();
  }
 private:
  void idle() {
    upt_vld = false;
    upt_id = 0;
    upt_op = 0;
    upt_key = 0;
    upt_size = 0;
  }
  void wait_cycles(std::size_t cycles = 1) {
    while (cycles-- > 0)
      wait(clk.posedge_event());
  }
#undef UPT_PORTS
};

struct SortedListsTb : libtb2::Top<uut_t> {
  sc_core::sc_port<QryIntf> qry;
  sc_core::sc_port<UptIntf> upt;

  SC_HAS_PROCESS(SortedListsTb);
  SortedListsTb(sc_core::sc_module_name mn = "t")
      : uut_("uut")
      , notify_checker_(mdl_)
      , query_checker_(mdl_)
#define __construct_signal(__name, __type)      \
        , __name##_(#__name)
      PORTS(__construct_signal)
#undef __construct_signal
  {
    //
    //    wd_.clk(clk_);
    //
    sampler_.clk(clk_);
    //
    resetter_.clk(clk_);
    resetter_.rst(rst_);
    //
    notify_checker_.clk(clk_);
    notify_checker_.ntf_vld_r(ntf_vld_r_);
    notify_checker_.ntf_id_r(ntf_id_r_);
    notify_checker_.ntf_key_r(ntf_key_r_);
    notify_checker_.ntf_size_r(ntf_size_r_);
    //
    query_checker_.clk(clk_);
    query_checker_.qry_resp_vld_r(qry_resp_vld_r_);
    query_checker_.qry_key_r(qry_key_r_);
    query_checker_.qry_size_r(qry_size_r_);
    query_checker_.qry_error_r(qry_error_r_);
    query_checker_.qry_listsize_r(qry_listsize_r_);
    query_checker_.qry_id_r(qry_id_r_);
    //
    qry.bind(qry_xactor_);
    qry_xactor_.clk(clk_);
    qry_xactor_.qry_vld(qry_vld_);
    qry_xactor_.qry_id(qry_id_);
    qry_xactor_.qry_level(qry_level_);

    //
    upt.bind(upt_xactor_);
    upt_xactor_.clk(clk_);
    upt_xactor_.upt_vld(upt_vld_);
    upt_xactor_.upt_id(upt_id_);
    upt_xactor_.upt_op(upt_op_);
    upt_xactor_.upt_key(upt_key_);
    upt_xactor_.upt_size(upt_size_);

    //
    SC_THREAD(t_stimulus);
    SC_THREAD(t_update);

    uut_.clk(clk_);
    uut_.rst(rst_);
#define __bind_signal(__name, __type)           \
    uut_.__name(__name##_);
    PORTS(__bind_signal)
#undef __bind_signals

    //
    register_uut(uut_);
    //    vcd_on();
  }
 private:
  void t_update() {
    resetter_.wait_reset_done();

    LOGGER(INFO) << "Setting configuration...\n";
    for (int i = 0; i < opts::M; i++) {
      const sl::Update u = mdl_.random_update();
      upt->issue(u);
      mdl_.update(u);
    }
    LOGGER(INFO) << "Configuration set...\n";

    wait_cycles(10);
    update_done_event_.notify();

    scv_smart_ptr<bool> pupt;
    scv_bag<bool> puptbg;
    puptbg.add(true, 1);
    puptbg.add(false, 99);
    pupt->set_mode(puptbg);

    mdl_.update_actives();
    while (true) {
      pupt->next();

      const bool issue_new_update_set = *pupt;
      if (issue_new_update_set) {
        // Allow time for inflight updates to complete.
        //
        wait_cycles(10);
        mdl_.update_actives();

        // Additional delay to allow for inflight QUERY commands to
        // complete before any attempt to (potentially) modify their
        // state.
        //
        wait_cycles(10);
      }

      const sl::Update u = mdl_.random_update();
      upt->issue(u);
      mdl_.update(u);
    }
    wait();
  }

  void t_stimulus() {
    resetter_.wait_reset_done();
    wait(update_done_event_);

    LOGGER(INFO) << "Stimulus starts...\n";
    while (true) {
      const sl::Query q = mdl_.random_query();

      qry->issue(q);
      sl::QueryResult qr;
      mdl_.apply_query(q, qr);
      qr.query = q;

      query_checker_.add_pending_result(qr);
    }
    LOGGER(INFO) << "Stimulus ends...\n";

    wait();
  }

  void wait_cycles(std::size_t cycles = 1) {
    while (cycles-- > 0)
      wait(clk_.posedge_event());
  }
  
  MachineModel mdl_;
  QryXactor qry_xactor_;
  UptXactor upt_xactor_;
  NotifyChecker notify_checker_;
  QueryChecker query_checker_;
  
  sc_core::sc_event update_done_event_;
  
  libtb2::Resetter resetter_;
  //  libtb2::SimWatchDogCycles wd_;
  libtb2::Sampler sampler_;
  sc_core::sc_clock clk_;
  sc_core::sc_signal<bool> rst_;
#define __declare_signal(__name, __type)        \
  sc_core::sc_signal<__type> __name##_;
  PORTS(__declare_signal)
#undef __declare_signal
  uut_t uut_;
};
SC_MODULE_EXPORT(SortedListsTb);

int sc_main(int argc, char **argv) {
  SortedListsTb tb;
  return libtb2::Sim::start(argc, argv);
}
