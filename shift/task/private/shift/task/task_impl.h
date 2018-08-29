#ifndef SHIFT_TASK_TASK_IMPL_H
#define SHIFT_TASK_TASK_IMPL_H

#include <functional>
#include <shift/core/boost_disable_warnings.h>
#include <boost/coroutine2/coroutine.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/task/system_call.h"
#include "shift/task/task.h"

namespace shift::task
{
using asymmetric_coroutine =
  boost::coroutines2::asymmetric_coroutine<system_call_t>;

struct task_base::impl : public task_create_info
{
  /// Constructor.
  explicit impl(const task_create_info& create_info,
                std::function<void()> function) noexcept;

  impl(const impl&) = delete;
  impl(impl&&) = delete;
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

  ///
  system_call_t resume();

  ///
  void system_call(system_call_t system_call_args);

  static std::atomic<task_id_t> next_id;
  task_id_t id;
  std::unique_ptr<asymmetric_coroutine::pull_type> coro;
  asymmetric_coroutine::push_type* sink = nullptr;
  std::function<void()> wrapped_function;
};
}

#endif
