#include "shift/task/task_impl.h"
#include "shift/task/task_system_impl.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/coroutine2/coroutine.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <functional>
#include <utility>

namespace shift::task
{
std::atomic<task_id_t> task_base::impl::next_id = ATOMIC_VAR_INIT(0);

task_base::impl::impl(const task_create_info& create_info,
                      std::function<void()> function) noexcept
: task_create_info(create_info),
  id(++next_id),
  wrapped_function(std::move(function))
{
}

system_call_t task_base::impl::resume()
{
  using std::begin;
  using std::end;

  if (!coro)
  {
    // Creating the coroutine executes the lambda.
    coro = std::make_unique<asymmetric_coroutine::pull_type>(
      [&](asymmetric_coroutine::push_type& the_sink) {
        // Store a pointer to the sink in a member variable, so it does not
        // have to be passed through each function call.
        sink = &the_sink;
        // Execute the actual function.
        wrapped_function();
      });
  }
  else
    (*coro)();
  if (*coro)
    return coro->get();
  else
    return task_end{};
}

void task_base::impl::system_call(system_call_t system_call_args)
{
  BOOST_ASSERT(sink);
  (*sink)(std::move(system_call_args));
}

task_base::task_base(const task_create_info& create_info,
                     std::function<void()> wrapped_function)
: _impl(std::make_unique<impl>(create_info, std::move(wrapped_function)))
{
}

task_base::~task_base() = default;

void task_base::yield()
{
  /// ToDo: Check if called from current task.
  BOOST_ASSERT(*_impl->sink);
  (*_impl->sink)(task_yield{});
}

task_id_t task_base::id() const
{
  return _impl->id;
}

void task_base::worker_affinity(worker_affinity_t affinity)
{
  _impl->worker_affinity(affinity);
}

worker_affinity_t task_base::worker_affinity() const
{
  return _impl->worker_affinity();
}
}
