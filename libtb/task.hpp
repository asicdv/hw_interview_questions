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

#ifndef __TASK_HPP__
#define __TASK_HPP__

#define SC_INCLUDE_DYNAMIC_PROCESSES
#include <systemc>
#include <memory>
#include <queue>
#include <gtest/gtest.h>
#include "top.hpp"

namespace tb {

struct Task {
  virtual ~Task() {}
  virtual bool is_finished() const { return finish_; }
  virtual bool is_completed() const { return false; }
  virtual void execute() = 0;
  virtual void finish() { finish_ = true; }
 protected:
  virtual void t_join_handles(std::vector<::sc_core::sc_process_handle> & hs);
 private:
  bool finish_{false};
};

template<typename TOP>
struct BasicNotFailTask : Task {
  using stimulus_type = typename TOP::stimulus_type;

  BasicNotFailTask(TOP & top) : top_(top) {}
  bool is_completed() const override { return is_completed_; }
  void add_stimulus(const stimulus_type & stim) {
    stimulus_.push(stim);
  }
  void execute() override {
    using namespace sc_core;

    // Fork:
    sc_process_handle h_stim =
        sc_spawn(std::bind(&BasicNotFailTask::t_stimulus, this),
                 "t_stimulus");

    sc_spawn_options copts;
    copts.set_sensitivity(&top_.clk.negedge_event());
    copts.spawn_method();
    copts.dont_initialize();

    sc_process_handle h_check = 
        sc_spawn(std::bind(&BasicNotFailTask::m_checker, this),
                 "m_checker", &copts );

    // Join:
    wait(h_stim.terminated_event());
    if (!h_check.terminated())
      h_check.kill();
  }
 private:
  void t_stimulus() {
    top_.set_idle();
    while (!stimulus_.empty() && !fail()) {
      const stimulus_type & s{stimulus_.front()};
      top_.t_set_stimulus(s);
      stimulus_.pop();
    }
    top_.set_idle();
    wait (100, ::sc_core::SC_NS);
    is_completed_ = true; 
  }
  void m_checker() {
    ASSERT_FALSE(top_.is_fail()) << "["
                                 << ::sc_core::sc_time_stamp()
                                 << "]: Fail Asserted!";
  }
  bool fail() const { return ::testing::Test::HasFatalFailure(); }
  bool is_completed_{false};
  std::queue<stimulus_type> stimulus_;
  TOP & top_;
};

template<typename TOP>
struct BasicPassValidNotBusyTask : Task {
  using stimulus_type = typename TOP::stimulus_type;
  using expected_type = typename TOP::expected_type;
  
  BasicPassValidNotBusyTask(TOP & top) : top_(top) {}
  bool is_completed() const override { return is_completed_; }
  void add_stimulus(const stimulus_type & stim) {
    stimulus_.push(stim);
  }
  virtual void add_expected(const expected_type & expect) {
    expected_.push(expect);
  }
  void execute() override {
    using namespace sc_core;

    // Fork:
    sc_process_handle h_stim =
        sc_spawn(std::bind(&BasicPassValidNotBusyTask::t_stimulus, this),
                 "t_stimulus");

    sc_spawn_options copts;
    copts.set_sensitivity(&top_.clk.negedge_event());
    copts.spawn_method();
    copts.dont_initialize();

    sc_process_handle h_check = 
        sc_spawn(std::bind(&BasicPassValidNotBusyTask::m_checker, this),
                 "m_checker", &copts );

    // Join:
    wait(h_stim.terminated_event());
    if (!h_check.terminated())
      h_check.kill();
  }
 protected:
  virtual void t_stimulus() {
    top_.set_idle();
    top_.t_apply_reset();
    while (!stimulus_.empty() && !fail()) {
      const stimulus_type & s{stimulus_.front()};
      top_.t_set_stimulus(s);
      stimulus_.pop();
    }
    top_.set_idle();
    wait (100, ::sc_core::SC_NS);
    is_completed_ = true; 
  }
  virtual void m_checker() {
    if (top_.out_is_valid()) {
      ASSERT_FALSE(expected_.empty());

      const expected_type & tb_out{expected_.front()};
      const expected_type & rtl_out{top_.get_expect()};
      ASSERT_EQ(tb_out, rtl_out) << "["
                                 << ::sc_core::sc_time_stamp()
                                 << "]: Stimulus mismatch!"
                                 << " Expected: " << tb_out
                                 << " Actual: " << rtl_out;
      expected_.pop();
    }
  }
  bool fail() const { return ::testing::Test::HasFatalFailure(); }
  bool is_completed_{false};
  std::queue<stimulus_type> stimulus_;
  std::queue<expected_type> expected_;
  TOP & top_;
};

class TaskRunner : public ::sc_core::sc_module {
 public:
  SC_HAS_PROCESS(TaskRunner);
  TaskRunner();
  void set_task(std::unique_ptr<Task> && task);
  void run_until_exhausted(bool do_stop = false);

 private:
  void t_stimulus();
  void t_main_loop();

  ::sc_core::sc_event e_start_tb_;
  std::unique_ptr<Task> task_;
};

} // tb

#endif
