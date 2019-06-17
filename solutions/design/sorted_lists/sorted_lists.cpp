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

#include "libtb/libtb.hpp"
#include "libtb/verilator.hpp"
#include <gtest/gtest.h>
#include <algorithm>
#include <vector>
#include <sstream>
#include <utility>
#include <memory>
#include <sstream>
#include <array>
#include "vobj/Vsorted_lists.h"

using word_type = vluint32_t;
using long_type = vluint64_t;

#define PORTS(__func)                           \
  __func(upt_vld, bool)                         \
  __func(upt_id, word_type)                     \
  __func(upt_op, word_type)                     \
  __func(upt_key, long_type)                    \
  __func(upt_size, word_type)                   \
  __func(upt_error_vld_r, bool)                 \
  __func(upt_error_id_r, word_type)             \
  __func(qry_vld, bool)                         \
  __func(qry_id, word_type)                     \
  __func(qry_level, word_type)                  \
  __func(qry_resp_vld_r, bool)                  \
  __func(qry_key_r, long_type)                  \
  __func(qry_size_r, word_type)                 \
  __func(qry_error_r, bool)                     \
  __func(qry_listsize_r, word_type)             \
  __func(qry_id_r, word_type)                   \
  __func(ntf_vld_r, bool)                       \
  __func(ntf_id_r, word_type)                   \
  __func(ntf_key_r, long_type)                  \
  __func(ntf_size_r, word_type)

struct Update {
  friend bool operator==(const Update & a, const Update & b) {
    if (a.id != b.id)
      return false;
    if (a.op != b.op)
      return false;
    if (a.size != b.size)
      return false;
    if (a.key != b.key)
      return false;

    return true;
  }
  
  word_type id, op, size;
  long_type key;
};

struct UpdateResponse {
  friend bool operator==(const UpdateResponse & a, const UpdateResponse & b) {
    if (a.vld != b.vld)
      return false;
    if (a.id != b.id)
      return false;
    return true;
  }
  friend std::ostream & operator<<(std::ostream & os, const UpdateResponse & ur) {
    return os << " vld:" << ur.vld
              << " id:" << ur.id;
  }
  
  bool vld{false};
  word_type id;
};

struct QueryCommand {
  word_type id, level;
  bool error_expected{false};
};

struct QueryResponse {
  friend bool operator==(const QueryResponse & a, const QueryResponse & b) {
    if (a.vld != b.vld)
      return false;
    if (a.size != b.size)
      return false;
    if (a.listsize != b.listsize)
      return false;
    if (a.id != b.id)
      return false;
    if (a.key != b.key)
      return false;
    if (a.error != b.error)
      return false;

    return true;
  }
  friend std::ostream& operator<<(std::ostream & os, const QueryResponse & qr) {
    return os << " vld:" << qr.vld
              << " size:" << qr.size
              << " listsize:" << qr.listsize
              << " id:" << qr.id
              << " key:" << qr.key
              << " error:" << qr.error;
  }
  
  bool vld{false};
  word_type size, listsize, id;
  long_type key;
  bool error;
};

struct Notify {
  friend bool operator==(const Notify & a, const Notify & b) {
    if (a.vld != b.vld)
      return false;
    if (a.id != b.id)
      return false;
    if (a.size != b.size)
      return false;
    if (a.key != b.key)
      return false;

    return true;
  }
  friend std::ostream& operator<<(std::ostream & os, const Notify & n) {
    return os << " vld:" << n.vld
              << " id:" << n.id
              << " size:" << n.size
              << " key:" << n.key;
  }
  
  bool vld{false};
  word_type id, size;
  long_type key;
};

enum : word_type { OP_CLEAR = 0, OP_ADD = 1, OP_DELETE = 2, OP_REPLACE = 3 };

struct TOP : tb::Top {
  TOP() {
    v.rst(rst);
    v.clk(clk);
#define __bind_signals(__name, __type)          \
    v.__name(__name);
    PORTS(__bind_signals)
#undef __bind_signals
    start_tracing();
  }
  ~TOP() {
    stop_tracing();
  }
  void update_idle() {
    upt_vld = false;
  }
  void t_update_issue(const Update & u) {
    upt_vld = true;
    upt_id = u.id;
    upt_op = u.op;
    upt_key = u.key;
    upt_size = u.size;
    t_await_cycles(1);
    update_idle();
  }
  UpdateResponse t_update_response() {
    while (!upt_error_vld_r)
      t_await_cycles();
    return UpdateResponse{true, upt_error_id_r};
  }
  void query_idle() {
    qry_vld = false;
  }
  void t_query_issue(const QueryCommand & q) {
    qry_vld = true;
    qry_id = q.id;
    qry_level = q.level;
    t_await_cycles(1);
    query_idle();
    // NOP cycle as per cycle.
    t_await_cycles(1);
  }
  QueryResponse t_query_response() {
    while (!qry_resp_vld_r)
      t_await_cycles();

    return QueryResponse{true, qry_size_r, qry_listsize_r,
        qry_id_r, qry_key_r, qry_error_r};
  }
  Notify t_notify() {
    while (!ntf_vld_r)
      t_await_cycles();

    return Notify{true, ntf_id_r, ntf_size_r, ntf_key_r};
  }
  void t_apply_reset() {
    rst = true;
    t_await_cycles(2);
    rst = false;
    t_await_cycles(2);
  }
  void t_await_cycles(std::size_t n = 1) {
    while (n--)
      wait(clk.negedge_event());
  }
  Vsorted_lists v{"sorted_lists"};
  ::sc_core::sc_signal<bool> rst{"rst"};
  ::sc_core::sc_clock clk{"clk"};
#define __declare_signal(__name, __type)        \
  ::sc_core::sc_signal<__type> __name{#__name};
  PORTS(__declare_signal)
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

TOP top;
tb::TaskRunner TaskRunner;

struct KV { long_type key; word_type size; };
bool operator<(const KV & a, const KV & b) { return a.key < b.key; }
using Entry = std::vector<KV>;
    
struct KeyFinder {
  KeyFinder(long_type key) : key_(key) {}
  bool operator()(const KV & kv) const { return (key_ == kv.key); }
 private:
  long_type key_;
};

class MachineModel {
 public:
  MachineModel() = default;
  void update_actives() {
    tb::Random::UniformRandomInterval<word_type> id{63, 0};

    active_updates_.clear();
    for (int i = 0; i < 20; i++)
      active_updates_.insert(id());
  }

  // Construct a random query based upon the known state of the machine.
  //
  Update random_update() {
    Update upt;

    // If the ACTIVE UPDATE set is non-empty, ensure that ID is a random
    // entry in the SET. We cannot touch ID outside of this range.
    if (active_updates_.size() != 0) {
      tb::Random::Bag<word_type> rndid{active_updates_.begin(), active_updates_.end()};
      upt.id = rndid();
    } else {
      tb::Random::UniformRandomInterval<word_type> rndid{63, 0};
      upt.id = rndid();
    }

    upt.op = random_update_op(upt.id, upt.key);
    tb::Random::UniformRandomInterval<word_type> rndsize;
    upt.size = rndsize();
    return upt;
  }

  QueryCommand random_query() {
    QueryCommand q;
    int id;
    // Specifically choose an ID outside of the ACTIVE UPDATE set.
    //
    while (true) {
      tb::Random::UniformRandomInterval<word_type> rndid{63};
      id = rndid();
      if (active_updates_.find(id) == active_updates_.end())
        break;
    }
    tb::Random::UniformRandomInterval<word_type> rndlevel{3};
    q.id = id;
    q.level = rndlevel();
    const std::size_t size = t_[id].size();
    q.error_expected = (size == 0) || (q.level >= size);
    return q;
  }

  void apply_query(const QueryCommand & q, QueryResponse & qr) {
    qr.vld = true;
    qr.id = q.id;
    qr.key = 0;
    qr.size = 0;
    qr.listsize = 0;
    qr.error = t_[q.id].empty() || (q.level >= t_[q.id].size());
    if (qr.error)
      return;
    // Sort by key
    std::sort(t_[q.id].begin(), t_[q.id].end());
    qr.key = t_[q.id][q.level].key;
    qr.size = t_[q.id][q.level].size;
    qr.listsize = t_[q.id].size();
  }

  std::pair<bool, bool> apply_update(const Update & u, Notify & notify) {
    bool error{false}, issue_notify{false};
    switch (u.op) {
      case OP_CLEAR: {
        t_[u.id].clear();
        issue_notify = true;
      } break;
      case OP_ADD: {
        if (t_[u.id].size() < 4)
          t_[u.id].push_back(KV{u.key,u.size});
        else
          error = true;
      } break;
      case OP_DELETE: {
        auto it = std::find_if(t_[u.id].begin(), t_[u.id].end(), KeyFinder(u.key));
        if (!t_[u.id].empty() && (it != t_[u.id].end())) {
          t_[u.id].erase(it);
          issue_notify = t_[u.id].empty();
        } else {
          error = true;
        }
      } break;
      case OP_REPLACE: {
        auto it = std::find_if(t_[u.id].begin(), t_[u.id].end(), KeyFinder(u.key));
        if (it != t_[u.id].end())
          it->size = u.size;
        else
          error = true;
      } break;
    }
    if (issue_notify)
      notify = Notify{true, u.id, u.size, u.key};
    return std::make_pair(error, issue_notify);
  }

 private:

  // Intelligently construct a opcode based upon the current machine
  // with appropriate weights where required.
  //
  word_type random_update_op(word_type id, long_type & k) {
    word_type r = OP_CLEAR;
    k = 0;
    const bool allow_error{tb::Random::Bool(1,9)};
    tb::Random::Bag<word_type> opbg;
    opbg.add(OP_ADD, 10);
    opbg.add(OP_DELETE, 10);
    opbg.add(OP_REPLACE, 10);
    opbg.finalize();

    const int ROUNDS = 10;
    for (int i = 0; i < ROUNDS; i++) {
      const std::size_t sz = t_[id].size();

      switch (opbg()) {
        case OP_ADD: {
          if (sz < 4 || allow_error) {
            tb::Random::UniformRandomInterval<word_type> rnd_key;
            k = rnd_key();
            r = OP_ADD;
            goto __end;
          }
        } break;
        case OP_DELETE: {
          if (sz != 0 || allow_error) {
            if (!allow_error) {
              tb::Random::Bag<long_type> rnd_key;
              for (const KV & kv : t_[id])
                rnd_key.add(kv.key);
              k = rnd_key();
            } else
              k = 0;
            r = OP_DELETE;

            goto __end;
          }
        } break;
        case OP_REPLACE: {
          if (sz != 0 || allow_error) {
            if (!allow_error) {
              tb::Random::Bag<long_type> rnd_key;
              for (const KV & kv : t_[id])
                rnd_key.add(kv.key);
              k = rnd_key();
            }
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
  std::array<Entry, 64> t_;
  std::set<word_type> active_updates_;
};

} // namespace

TEST(SortedListsTest, Basic) {
  struct SortedListsTask : ::tb::Task {
    SortedListsTask(TOP & top) : top_(top) {}
    void execute() override {
      using namespace sc_core;

      // Spawn global thread to consume outbound notifies.
      //
      auto ph0{
        sc_spawn(std::bind(&SortedListsTask::t_consume_notifies, this),
                 "t_consume_notifies")};
      // Spawn global thread to consume query respones.
      //
      auto ph1{
        sc_spawn(std::bind(&SortedListsTask::t_consume_query_responses, this),
                 "t_consume_query_responses")};
      // Spawn global thread to consume update responses.
      //
      auto ph2{
        sc_spawn(std::bind(&SortedListsTask::t_consume_update_responses, this),
                 "t_consumes_update_responses")};
      // Spawn global thread to consume update responses.
      //
      auto ph3{
        sc_spawn(std::bind(&SortedListsTask::t_update_driver, this),
                 "t_update_driver")};
      // Spawn global thread to consume update responses.
      //
      auto ph4{
        sc_spawn(std::bind(&SortedListsTask::t_query_driver, this),
                 "t_query_driver")};
      
      top_.t_apply_reset();

      // Perform N update, query rounds where the active set of ID are
      // continuously swapped as per spec. requirements.
      //
      tb::Random::UniformRandomInterval<word_type> dly{63};
      for (std::size_t round = 0; round < 16; round++) {
        // Compute the update set.
        mdl_.update_actives();
        for (std::size_t i = 0; i < 1024; i++) {
          const Update & upt{mdl_.random_update()};
          updates_.push_back(upt);
          Notify notify;
          bool error, issue_notify;
          std::tie(error, issue_notify) = mdl_.apply_update(upt, notify);
          if (error)
            update_responses_.push_back(UpdateResponse{true, upt.id});
          if (issue_notify)
            notifies_.push_back(notify);
        }
        // Compute queries
        for (std::size_t i = 0; i < 1024; i++) {
          const QueryCommand qry{mdl_.random_query()};
          queries_.push_back(qry);
          QueryResponse qryr;
          mdl_.apply_query(qry, qryr);
          EXPECT_EQ(qry.error_expected, qryr.error);
          query_responses_.push_back(qryr);
        }
        round_start_.notify();
        wait(done_update_driver_ & done_query_driver_);
        top_.t_await_cycles(dly());
      }
      finish();
    }
   private:
    void t_update_driver() {
      while (true) {
        wait(round_start_);
        for (const Update & u : updates_)
          top_.t_update_issue(u);
        done_update_driver_.notify();
      }
    }
    void t_query_driver() {
      while (true) {
        wait(round_start_);
        for (const QueryCommand & cmd : queries_)
          top_.t_query_issue(cmd);
        done_query_driver_.notify();
      }
    }
    void t_consume_notifies() {
      const Notify rtl{top_.t_notify()};
      ASSERT_FALSE(notifies_.empty());
      const Notify tb{notifies_.front()};
      ASSERT_EQ(rtl, tb) << sc_core::sc_time_stamp();
      notifies_.pop_front();
    }
    void t_consume_query_responses() {
      const QueryResponse rtl{top_.t_query_response()};
      ASSERT_FALSE(query_responses_.empty());
      const QueryResponse tb{query_responses_.front()};
      ASSERT_EQ(rtl, tb) << sc_core::sc_time_stamp();
      query_responses_.pop_front();
    }
    void t_consume_update_responses() {
      const UpdateResponse rtl{top_.t_update_response()};
      ASSERT_FALSE(update_responses_.empty());
      const UpdateResponse tb{update_responses_.front()};
      ASSERT_EQ(rtl, tb) << sc_core::sc_time_stamp();
      update_responses_.pop_front();
    }
    std::vector<Update> updates_;
    std::vector<QueryCommand> queries_;
    std::deque<Notify> notifies_;
    std::deque<QueryResponse> query_responses_;
    std::deque<UpdateResponse> update_responses_;
    sc_core::sc_event round_start_;
    sc_core::sc_event done_update_driver_;
    sc_core::sc_event done_query_driver_;
    MachineModel mdl_;
    TOP & top_;
  };
  auto task = std::make_unique<SortedListsTask>(top);
  TaskRunner.set_task(std::move(task));
  TaskRunner.run_until_exhausted(true);
}

int sc_main(int argc, char ** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  ::tb::initialize(argc, argv);
  return RUN_ALL_TESTS();
}

