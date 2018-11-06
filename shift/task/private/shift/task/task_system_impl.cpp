#include "shift/task/task_system_impl.hpp"

namespace shift::task
{
thread_local task_base* task_system::impl::current_task;
thread_local std::uint32_t task_system::impl::current_worker_id;

task_system::impl::impl(unsigned int fp_exceptions,
                        unsigned int fp_exception_mask)
: fp_exceptions(fp_exceptions), fp_exception_mask(fp_exception_mask)
{
}

task_system::impl::~impl() = default;

std::unique_ptr<task_base> task_system::impl::operator()(
  task_queue& command, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& /*queue_lock*/)
{
  BOOST_ASSERT(command.task);
  if (!command.task)
    return calling_task;

  // Put new tasks on the task queue.
  worker_should_have_tasks |= command.task->worker_affinity();
  queued_tasks.emplace_back(std::move(command.task));
  ++task_count;

  /// ToDo: Only signal a single worker.
  for (auto& worker : workers)
    worker.queue_condition->notify_one();

  // Continue executing current task.
  return calling_task;
}

std::unique_ptr<task_base> task_system::impl::operator()(
  task_queue_blocked& command, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& queue_lock)
{
  // Put new tasks on the blocked task queue.
  (*this)(task_unlock_mutex{command.lock}, queue_lock);
  blocking_tasks.insert(
    std::make_pair(command.condition, std::move(command.task)));
  ++task_count;
  /// ToDo: Only signal a single worker.
  for (auto& worker : workers)
    worker.queue_condition->notify_one();

  // Continue executing current task.
  return calling_task;
}

std::unique_ptr<task_base> task_system::impl::operator()(
  task_yield /*command*/, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& queue_lock)
{
  if (calling_task)
  {
    if (queued_tasks.empty())
    {
      // We would get the current task again, so take the fast path out.
      return calling_task;
    }
    worker_should_have_tasks |= calling_task->worker_affinity();
    queued_tasks.emplace_back(std::move(calling_task));
  }
  return schedule_task(queue_lock);
}

std::unique_ptr<task_base> task_system::impl::operator()(
  task_lock_mutex command, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& queue_lock)
{
  BOOST_ASSERT(calling_task);
  if (!calling_task)
    return nullptr;

  if (command.lock->_flag.test_and_set(std::memory_order_acquire))
  {
    blocking_tasks.insert(
      std::make_pair(command.lock, std::move(calling_task)));
    return schedule_task(queue_lock);
  }
  else
    return calling_task;
}

std::unique_ptr<task_base> task_system::impl::operator()(
  task_wait_condition command, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& queue_lock)
{
  BOOST_ASSERT(calling_task);
  if (!calling_task)
    return nullptr;

  // Unlocking a mutex won't change calling_task.
  /// ToDo: Do we have to unlock the mutex before inserting the blocking task?
  (*this)(task_unlock_mutex{command.lock}, queue_lock);
  blocking_tasks.insert(
    std::make_pair(command.condition, std::move(calling_task)));
  return schedule_task(queue_lock);
}

std::unique_ptr<task_base> task_system::impl::operator()(
  task_end /*command*/, std::unique_ptr<task_base> calling_task,
  task_system::impl::queue_lock_t& queue_lock)
{
  BOOST_ASSERT(calling_task);
  if (!calling_task)
    return nullptr;

  // Free and forget calling_task.
  if (--task_count == 0)
  {
    quit = true;
    for (auto& worker : workers)
      worker.queue_condition->notify_one();
  }
  return schedule_task(queue_lock);
}

void task_system::impl::operator()(
  task_unlock_mutex command, task_system::impl::queue_lock_t& /*queue_lock*/)
{
  // Find tasks in blocking_tasks and move them to the queued_tasks queue.
  command.lock->_flag.clear(std::memory_order_release);
  auto range = blocking_tasks.equal_range(command.lock);
  for (auto task_iter = range.first; task_iter != range.second; ++task_iter)
  {
    worker_should_have_tasks |= task_iter->second->worker_affinity();
    queued_tasks.emplace_back(std::move(task_iter->second));

    // Notify an idle worker thread.
    /// ToDo: Delay notification until the global queue_mutex has been
    /// released.
    /// ToDo: Only signal a single worker.
    for (auto& worker : workers)
      worker.queue_condition->notify_one();
  }
  blocking_tasks.erase(range.first, range.second);
}

void task_system::impl::operator()(
  task_notify_condition command,
  task_system::impl::queue_lock_t& /*queue_lock*/)
{
  // Find tasks in blocking_tasks and move them to the queued_tasks queue.
  auto range = blocking_tasks.equal_range(command.condition);
  auto task_iter = range.first;
  while (task_iter != range.second)
  {
    worker_should_have_tasks |= task_iter->second->worker_affinity();
    queued_tasks.emplace_back(std::move(task_iter->second));
    ++task_iter;

    /// ToDo: Only signal a single worker, but not the wrong one.
    for (auto& worker : workers)
      worker.queue_condition->notify_one();
    if (command.count > 0)
    {
      if (--command.count == 0)
        break;
    }
  }
  blocking_tasks.erase(range.first, task_iter);
}

std::unique_ptr<task_base> task_system::impl::schedule_task(
  task_system::impl::queue_lock_t& queue_lock)
{
  for (;;)
  {
    workers[current_worker_id - 1].queue_condition->wait(queue_lock, [&]() {
      return quit || (!queued_tasks.empty() &&
                      ((worker_should_have_tasks &
                        (1u << (current_worker_id - 1))) != 0u));
    });
    if (quit)
      return nullptr;
    else if (!queued_tasks.empty())
    {
      // Walk through list of queued tasks to find one that is affine to this
      // worker.
      for (auto task_iter = std::begin(queued_tasks);
           task_iter != std::end(queued_tasks); ++task_iter)
      {
        if (((*task_iter)->worker_affinity() &
             (1u << (current_worker_id - 1))) != 0u)
        {
          auto task = std::move(*task_iter);
          queued_tasks.erase(task_iter);
          current_task = task.get();
          return task;
        }
      }
      // Even though worker_should_have_tasks had this worker's bit set, it
      // was no guarantee that there actually was a waiting task affine to
      // this worker. Thus, clear the bit now and try again.
      worker_should_have_tasks &= ~(1u << (current_worker_id - 1));
      continue;
    }
    else
    {
      BOOST_ASSERT(false);
      return nullptr;
    }
  }
}
}
