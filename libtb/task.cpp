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

#include "task.hpp"

namespace tb {

TaskRunner::TaskRunner() :
    ::sc_core::sc_module{::sc_core::sc_module_name{"TaskRunner"}} {
  SC_THREAD(t_stimulus);
}

void TaskRunner::set_task(std::unique_ptr<Task> && task) {
  task_ = std::move(task);
}

void TaskRunner::run_until_exhausted(bool do_stop) {
  using namespace sc_core;

  e_start_tb_.notify(1, SC_NS);
  while (!task_->is_completed())
    ::sc_core::sc_start(10, SC_US);
  if (do_stop)
    ::sc_core::sc_stop();
}

void TaskRunner::t_stimulus() {
  while (true) {
    wait(e_start_tb_);
    t_main_loop();
  }
}

void TaskRunner::t_main_loop() {
  if (task_)
    task_->execute();
}

} // namespace tb
