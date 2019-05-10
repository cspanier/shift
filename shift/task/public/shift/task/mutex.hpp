#ifndef SHIFT_TASK_MUTEX_HPP
#define SHIFT_TASK_MUTEX_HPP

#include <cstdint>
#include <atomic>
#include <shift/core/types.hpp>

namespace shift::task
{
///
class mutex
{
public:
  ///
  mutex() noexcept = default;

  mutex(const mutex&) = delete;
  mutex(mutex&&) = delete;

  ///
  ~mutex();

  mutex& operator=(const mutex&) = delete;
  mutex& operator=(mutex&&) = delete;

  ///
  void lock();

  ///
  bool try_lock();

  ///
  void unlock();

private:
  friend class task_system;

  std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};
}

#endif
