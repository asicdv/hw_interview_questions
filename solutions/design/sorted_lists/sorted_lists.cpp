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
  
  bool vld{false};
  word_type id;
};

struct QueryCommand {
  word_type id, level;
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

} // namespace

class MachineModel {
  struct KV { long_type key; word_type size; };
  class Entry {
   public:
    Entry() {}

    bool query(long_type key, KV & kv) const {
      auto it = std::find_if(keys.begin(), keys.end(),
                             [&](const KV & i) { return i.key == key; });
      const bool found{it != keys.end()};
      if (found)
        kv = *it;
      return !found;
    }
    std::size_t listsize() const { return keys.size(); }
    void sort() {
      std::sort(keys.begin(), keys.end(),
                [](const KV & a, const KV & b) { return a.key < b.key; });
    }
    bool clear() {
      keys.clear();
      return false;
    }
    bool add(const KV & kv) {
      const bool error{keys.size() == 4};
      if (!error)
        keys.push_back(kv);

      return error;
    }
    std::tuple<bool, bool> del(word_type key) {
      auto it = std::remove_if(keys.begin(), keys.end(),
                            [&](const KV & kv) { return kv.key == key; });

      const bool did_replace{it != keys.end()};
      if (did_replace)
        keys.erase(it, keys.end());

      return {keys.empty(), !did_replace};
    }
    bool replace(const KV & kv) {
      auto it = std::find_if(keys.begin(), keys.end(),
                             [&](const KV & b) { return kv.key == b.key; });

      const bool did_replace{it != keys.end()};
      if (did_replace)
        *it = kv;

      return !did_replace;
    }
   private:
    std::vector<KV> keys;
  };
 public:
  MachineModel() {}

  bool apply(const Update & update, Notify & notify) {
    bool error;
    switch (update.op) {
      case OP_CLEAR:
        error = s_[update.id].clear();
        if (error)
          notify = Notify{true, update.id, update.size, update.key};
        break;
      case OP_ADD:
        error = s_[update.id].add(KV{update.key, update.size});
        break;
      case OP_DELETE: {
        auto res{s_[update.id].del(update.key)};
        error = std::get<0>(res);
        notify.vld = std::get<1>(res);
        notify.id = update.id;
        notify.size = update.size;
        notify.key = update.key;
      } break;
      case OP_REPLACE:
        error = s_[update.id].replace(KV{update.key, update.size});
        break;
    }
    return error;
  }

  QueryResponse apply(const QueryCommand & query) {
    QueryResponse resp;

    auto it{s_.find(query.id)};
    if (it != s_.end()) {
      Entry & e{s_[query.id]};
      e.sort();
      resp.vld = true;
      KV kv;
      resp.error = e.query(query.level, kv);
      if (!resp.error) {
        resp.key = kv.key;
        resp.size = kv.size;
        resp.listsize = e.listsize();
        resp.id = query.id;
      }
    }
    return resp;
  }

 private:
  std::map<word_type, Entry> s_;
};

TEST(SortedListsTest, Basic) {
  struct SortedListsTask : ::tb::Task {
    SortedListsTask(TOP & top) : top_(top) {}
    void execute() override {
      using namespace sc_core;

      top_.t_apply_reset();

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
      

      // Perform N update, query rounds where the active set of ID are
      // continuously swapped as per spec. requirements.
      //
      tb::Random::UniformRandomInterval<word_type> dly{63};
      for (std::size_t round = 0; round < 16; round++) {
        std::vector<::sc_core::sc_process_handle> ph;

        // Compute the update set.
        //
        std::vector<word_type> ids_update;
        for (std::size_t i = 0; i < 8; i++)
          ids_update.push_back(dly());

        // Spawn thread to drive updates
        //
        ph.push_back(sc_spawn(std::bind(&SortedListsTask::t_update, this, ids_update),
                              "t_update"));

        // Compute the active set (disjoint from active set).
        //
        std::vector<word_type> ids_active;
        while (ids_active.size() != 24) {
          const word_type id{dly()};
          if (std::find(ids_active.begin(), ids_active.end(), id) != ids_active.end())
            ids_active.push_back(id);
        }

        // Spawn thread to drive queries.
        //
        ph.push_back(sc_spawn(std::bind(&SortedListsTask::t_query, this, ids_active),
                              "t_query"));

        // Join on threads before proceeding to next round.
        //
        t_join_handles(ph);
      }
    }
   private:
    void t_update(const std::vector<word_type> & ids) {
      tb::Random::UniformRandomInterval<word_type> rnd_size;
      tb::Random::UniformRandomInterval<long_type> rnd_key;
      tb::Random::Bag<word_type> rnd_id;
      rnd_id.add(ids.begin(), ids.end());

      tb::Random::Bag<word_type> rnd_ops;
      rnd_ops.add(OP_CLEAR);
      rnd_ops.add(OP_ADD);
      rnd_ops.add(OP_DELETE);
      rnd_ops.add(OP_REPLACE);
      rnd_ops.finalize();

      // Construct stimulus
      std::vector<Update> upts;
      for (std::size_t i = 0; i < 1024; i++) {
        Update upt;
        upt.id = rnd_id();
        upt.op = rnd_ops();
        upt.size = rnd_size();
        upt.key = rnd_key();
        upts.push_back(upt);
      }
      
      // Apply stimulus
      for (const Update & u : upts) {
        top_.t_update_issue(u);
        Notify notify;
        // TODO: need to rethink this.
        const bool error{m_.apply(u, notify)};
        //        if (error)
          notifies_.push_back(notify);
      }
    }
    void t_query(const std::vector<word_type> & ids) {
      tb::Random::UniformRandomInterval<word_type> level{3,0};
      tb::Random::Bag<word_type> bg;
      bg.add(ids.begin(), ids.end());
      
      // Construct stimulus
      std::vector<QueryCommand> qrys;
      for (std::size_t i = 0; i < 1024; i++)
        qrys.push_back(QueryCommand{bg(), level()});

      // Apply stimulus
      for (const QueryCommand & cmd : qrys) {
        top_.t_query_issue(cmd);
        query_responses_.push_back(m_.apply(cmd));
      }
    }
    void t_consume_notifies() {
      const Notify rtl{top_.t_notify()};
      const Notify tb{notifies_.front()};
      ASSERT_EQ(rtl, tb);
      notifies_.pop_front();
    }
    void t_consume_query_responses() {
      const QueryResponse rtl{top_.t_query_response()};
      const QueryResponse tb{query_responses_.front()};
      ASSERT_EQ(rtl, tb);
      query_responses_.pop_front();
    }
    void t_consume_update_responses() {
      const UpdateResponse rtl{top_.t_update_response()};
      const UpdateResponse tb{update_responses_.front()};
      ASSERT_EQ(rtl, tb);
      update_responses_.pop_front();
    }
    std::deque<Notify> notifies_;
    std::deque<QueryResponse> query_responses_;
    std::deque<UpdateResponse> update_responses_;
    MachineModel m_;
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

