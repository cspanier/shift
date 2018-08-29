#include "shift/task/mutex.h"
#include "shift/task/task_system.h"

namespace shift::task
{
mutex::~mutex() = default;

void mutex::lock()
{
  if (_flag.test_and_set(std::memory_order_acquire))
  {
    // We could not acquire the mutex, so perform a system call which
    // eventually sets this task to sleep.
    task_system::singleton_instance().lock_mutex(*this);
  }
}

bool mutex::try_lock()
{
  return !_flag.test_and_set(std::memory_order_acquire);
}

void mutex::unlock()
{
  // Resets _flag and wakes any other tasks waiting on this mutex.
  task_system::singleton_instance().unlock_mutex(*this);
}
}
