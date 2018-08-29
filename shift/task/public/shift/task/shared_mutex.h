#ifndef SHIFT_TASK_SHARED_MUTEX_H
#define SHIFT_TASK_SHARED_MUTEX_H

#include <cstdint>
#include <atomic>
#include <shift/core/types.h>

namespace shift::task
{
///
class shared_mutex
{
public:
  ///
  shared_mutex() noexcept = default;

  shared_mutex(const shared_mutex&) = delete;
  shared_mutex(shared_mutex&&) = default;

  ///
  ~shared_mutex();

  shared_mutex& operator=(const shared_mutex&) = delete;
  shared_mutex& operator=(shared_mutex&&) = default;

  ///
  void lock();

  ///
  bool try_lock();

  ///
  void unlock();

  ///
  void lock_shared();

  ///
  bool try_lock_shared();

  ///
  void unlock_shared();

private:
  friend class task_system;

  std::atomic_flag _flag = ATOMIC_FLAG_INIT;
};
}

#endif
