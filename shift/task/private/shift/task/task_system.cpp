#include "shift/task/task_system_impl.h"
#include "shift/task/task_impl.h"
#include <shift/platform/fpexceptions.h>
#include <shift/platform/environment.h>
#include <thread>

namespace shift::task
{
task_system::task_system(unsigned int fp_exceptions,
                         unsigned int fp_exception_mask)
: _impl(std::make_unique<impl>(fp_exceptions, fp_exception_mask))
{
  // Nop.
}

task_system::~task_system() = default;

task_system& task_system::num_workers(std::size_t count)
{
  BOOST_ASSERT(!_impl->running);
  if (_impl->running)
    return *this;  /// ToDo: Throw logic_error.

  if (count == 0)
    count = std::max(std::thread::hardware_concurrency(), 1u);
  // Limit the number of workers to the number of bits in worker_affinity_t.
  count = std::min(count, sizeof(worker_affinity_t) * 8);
  _impl->workers.resize(count);
  return *this;
}

std::uint32_t task_system::num_workers() const
{
  return static_cast<std::uint32_t>(_impl->workers.size());
}

int task_system::join()
{
  for (auto& worker : _impl->workers)
  {
    if (worker.thread.joinable())
      worker.thread.join();
  }
  _impl->workers.clear();
  _impl->running = false;
  return _impl->result.get();
}

bool task_system::running() const noexcept
{
  return _impl->running;
}

task_base* task_system::current_task() const
{
  return _impl->current_task;
}

worker_id_t task_system::current_worker_id() const
{
  return _impl->current_worker_id;
}

void task_system::initialize(std::unique_ptr<task_result<int>> primary_task)
{
  BOOST_ASSERT(!_impl->running);
  if (_impl->running)
    return;  /// ToDo: Throw logic_error.

  if (num_workers() == 0)
  {
    /// ToDo: Determ the optimal number of worker threads.
    num_workers(8);
  }

  _impl->running = true;
  _impl->result = primary_task->get_future();
  _impl->queued_tasks.emplace_back(std::move(primary_task));
  _impl->worker_should_have_tasks = all_workers;
  ++_impl->task_count;

  auto process_work = [&](worker_id_t worker_thread_id) {
    _impl->current_worker_id = worker_thread_id;

    platform::floating_point_exceptions fpexceptions(_impl->fp_exceptions,
                                                     _impl->fp_exception_mask);

    std::condition_variable queue_condition;
    _impl->workers[worker_thread_id - 1].queue_condition = &queue_condition;

    std::stringstream thread_debug_name;
    thread_debug_name << "task worker thread #" << worker_thread_id;
    platform::environment::thread_debug_name(thread_debug_name.str());

    // Wait until all worker threads started.
    {
      impl::queue_lock_t start_lock(_impl->start_stop_mutex);
      --_impl->worker_threads_starting;
      // The last thread started will simply walk through the wait.
      _impl->start_stop_condition.wait(start_lock, [&]() {
        return _impl->quit || _impl->worker_threads_starting == 0;
      });
    }
    // Wake all the other waiting worker threads.
    _impl->start_stop_condition.notify_all();

    if (!_impl->quit)
    {
      std::unique_ptr<task_base> current_task;
      system_call_t system_call = task_yield{};
      for (;;)
      {
        // Process system call.
        {
          impl::queue_lock_t queue_lock(_impl->queue_mutex);
          current_task = std::visit(
            [&](auto& command) -> std::unique_ptr<task_base> {
              return (*_impl)(command, std::move(current_task), queue_lock);
            },
            system_call);
        }
        if (!current_task)
          break;

        // Resume task execution until it returns a system call request.
        system_call = current_task->_impl->resume();
      }
    }

    // Wait until all worker threads finished processing tasks.
    {
      impl::queue_lock_t stop_lock(_impl->start_stop_mutex);
      --_impl->worker_threads_stopping;
      // The last thread started will simply walk through the wait.
      _impl->start_stop_condition.wait(
        stop_lock, [&]() { return _impl->worker_threads_stopping == 0; });
    }
    // Wake all the other waiting worker threads.
    _impl->start_stop_condition.notify_all();

    _impl->workers[worker_thread_id - 1].queue_condition = nullptr;
  };

  _impl->worker_threads_starting =
    static_cast<std::uint32_t>(_impl->workers.size());
  _impl->worker_threads_stopping =
    static_cast<std::uint32_t>(_impl->workers.size());
  worker_id_t id = no_worker;
  for (auto& worker : _impl->workers)
    worker.thread = std::thread(process_work, ++id);
}

void task_system::queue(std::unique_ptr<task_base> new_task)
{
  BOOST_ASSERT(_impl->current_task);
  if (_impl->current_task != nullptr)
    _impl->current_task->_impl->system_call(task_queue{std::move(new_task)});
}

void task_system::queue_blocked(std::unique_ptr<task_base> new_task,
                                condition_variable& condition, mutex& lock)
{
  BOOST_ASSERT(_impl->current_task);
  if (_impl->current_task != nullptr)
  {
    _impl->current_task->_impl->system_call(
      task_queue_blocked{std::move(new_task), &condition, &lock});
  }
}

void task_system::lock_mutex(mutex& lock)
{
  BOOST_ASSERT(_impl->current_task);
  if (_impl->current_task != nullptr)
    _impl->current_task->_impl->system_call(task_lock_mutex{&lock});
}

void task_system::unlock_mutex(mutex& lock)
{
  // Directly call implementation without leaving coroutine context.
  impl::queue_lock_t queue_lock(_impl->queue_mutex);
  (*_impl)(task_unlock_mutex{&lock}, queue_lock);
}

void task_system::wait_condition(condition_variable& condition, mutex& lock)
{
  BOOST_ASSERT(_impl->current_task);
  if (_impl->current_task != nullptr)
  {
    _impl->current_task->_impl->system_call(
      task_wait_condition{&condition, &lock});
    // Reacquire the lock.
    lock.lock();
  }
}

void task_system::notify_condition(condition_variable& condition,
                                   std::size_t count)
{
  // Directly call implementation without leaving coroutine context.
  impl::queue_lock_t queue_lock(_impl->queue_mutex);
  (*_impl)(task_notify_condition{&condition, count}, queue_lock);
}
}
