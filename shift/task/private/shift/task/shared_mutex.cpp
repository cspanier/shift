#include "shift/task/shared_mutex.hpp"
#include "shift/task/task_system.hpp"

namespace shift::task
{
shared_mutex::~shared_mutex() = default;

void shared_mutex::lock()
{
  // if (_flag.test_and_set(std::memory_order_acquire))
  //{
  //  // We could not acquire the shared_mutex, so perform a system call which
  //  // eventually sets this task to sleep.
  //  task_system::singleton_instance().lock_shared_mutex(*this);
  //}
}

bool shared_mutex::try_lock()
{
  return !_flag.test_and_set(std::memory_order_acquire);
}

void shared_mutex::unlock()
{
  //// Resets _flag and wakes any other tasks waiting on this shared_mutex.
  // task_system::singleton_instance().unlock_shared_mutex(*this);
}

void shared_mutex::lock_shared()
{
  // if (_flag.test_and_set(std::memory_order_acquire))
  //{
  //  // We could not acquire the shared_mutex, so perform a system call which
  //  // eventually sets this task to sleep.
  //  task_system::singleton_instance().lock_shared_mutex_shared(*this);
  //}
}

bool shared_mutex::try_lock_shared()
{
  return !_flag.test_and_set(std::memory_order_acquire);
}

void shared_mutex::unlock_shared()
{
  //// Resets _flag and wakes any other tasks waiting on this shared_mutex.
  // task_system::singleton_instance().unlock_shared_mutex_shared(*this);
}
}
