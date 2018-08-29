#ifndef SHIFT_TASK_CONDITION_VARIABLE_H
#define SHIFT_TASK_CONDITION_VARIABLE_H

#include <cstdint>
#include <mutex>  /// ToDo: Needed only for std::unique_lock.
#include <shift/core/types.h>
#include "shift/task/mutex.h"

namespace shift::task
{
///
class condition_variable
{
public:
  ///
  condition_variable();

  condition_variable(const condition_variable&) = delete;
  condition_variable(condition_variable&&) = default;

  ///
  ~condition_variable();

  condition_variable& operator=(const condition_variable&) = delete;
  condition_variable& operator=(condition_variable&&) = default;

  ///
  void notify_one() noexcept;

  ///
  void notify_all() noexcept;

  ///
  void notify_on_task_exit(std::unique_lock<mutex>& lock, int* ready);

  ///
  void wait(std::unique_lock<mutex>& lock);

  ///
  template <typename Predicate>
  void wait(std::unique_lock<mutex>& lock, Predicate predicate)
  {
    while (!predicate())
      wait(lock);
  }
};
}

#endif
