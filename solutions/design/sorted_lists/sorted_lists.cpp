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
  word_type id, op, size;
  long_type key;
};

struct UpdateError {
  word_type id;
};

struct QueryCommand {
  word_type id, level;
};

struct QueryResponse {
  word_type size, listsize, id;
  long_type key;
  bool error;
};

struct Notify {
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
  UpdateError t_update_error() {
    while (!upt_error_vld_r)
      t_await_cycles();
    return UpdateError{upt_error_id_r};
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
  }
  QueryResponse t_query_response() {
    while (!qry_resp_vld_r)
      t_await_cycles();

    return QueryResponse{qry_size_r, qry_listsize_r, qry_id_r, qry_key_r, qry_error_r};
  }
  Notify t_notify() {
    while (!ntf_vld_r)
      t_await_cycles();

    return Notify{ntf_id_r, ntf_size_r, ntf_key_r};
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
  struct KV { long_type key; word_type value; };
  class Entry {
   public:
    Entry() {}

    void clear() {
      keys.clear();
    }
    void add(const KV & kv) {
      keys.push_back(kv);
    }
    bool del(word_type key, Notify & notify) {
      auto it = std::remove_if(keys.begin(), keys.end(),
                            [&](const KV & kv) { return kv.key == key; });

      const bool did_replace{it != keys.end()};
      if (did_replace)
        keys.erase(it, keys.end());

      
      return did_replace;
    }
    bool replace(const KV & kv) {
      auto it = std::find_if(keys.begin(), keys.end(),
                             [&](const KV & b) { return kv.key == b.key; });

      const bool did_replace{it != keys.end()};
      if (did_replace)
        *it = kv;
      return did_replace;
    }
   private:
    std::vector<KV> keys;
  };
 public:
  MachineModel() {}

  bool apply(const Update & update, Notify & notify) {
    bool ret{false};
    switch (update.op) {
      case OP_CLEAR:
        s_[update.id].clear();
        break;
      case OP_ADD:
        s_[update.id].add(KV{update.key, update.size});
        break;
      case OP_DELETE:
        ret = s_[update.id].del(update.key, notify));
        break;
      case OP_REPLACE:
        EXPECT_TRUE(s_[update.id].replace(KV{update.key, update.size}));
        break;
    }
    return ret;
  }

  QueryResponse apply(const QueryCommand & query) {
    QueryResponse resp;

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
      
      sc_process_handle h_update = 
          sc_spawn(std::bind(&SortedListsTask::t_update, this), "t_update");
      sc_process_handle h_query = 
          sc_spawn(std::bind(&SortedListsTask::t_query, this), "t_query");
    }
   private:
    void t_update() {
    }
    void t_query() {
    }
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

