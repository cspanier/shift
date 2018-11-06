#ifndef SHIFT_TASK_TASK_SYSTEM_IMPL_HPP
#define SHIFT_TASK_TASK_SYSTEM_IMPL_HPP

#include <vector>
#include <deque>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "shift/task/system_call.hpp"
#include "shift/task/task_system.hpp"

namespace shift::task
{
struct task_system::impl
{
  using return_type = std::unique_ptr<task_base>;
  using queue_lock_t = std::unique_lock<std::mutex>;

  /// Constructor.
  impl(unsigned int fp_exceptions, unsigned int fp_exception_mask);

  /// Destructor.
  ~impl();

  ///
  std::unique_ptr<task_base> operator()(task_queue& command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> operator()(task_queue_blocked& command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> operator()(task_yield command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> operator()(task_lock_mutex command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> operator()(task_wait_condition command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> operator()(task_end command,
                                        std::unique_ptr<task_base> calling_task,
                                        queue_lock_t& queue_lock);

  ///
  void operator()(task_unlock_mutex command, queue_lock_t& queue_lock);

  ///
  void operator()(task_notify_condition command, queue_lock_t& queue_lock);

  ///
  std::unique_ptr<task_base> schedule_task(queue_lock_t& queue_lock);

  struct worker_thread_t
  {
    std::thread thread;
    std::condition_variable* queue_condition = nullptr;
  };

  static thread_local task_base* current_task;
  static thread_local std::uint32_t current_worker_id;
  bool quit = false;
  std::atomic<std::uint32_t> worker_threads_starting = ATOMIC_VAR_INIT(0);
  std::atomic<std::uint32_t> worker_threads_stopping = ATOMIC_VAR_INIT(0);
  std::mutex start_stop_mutex;
  std::condition_variable start_stop_condition;
  bool running = false;
  std::vector<worker_thread_t> workers;
  std::mutex queue_mutex;
  std::atomic<std::uint32_t> task_count = ATOMIC_VAR_INIT(0);
  std::deque<std::unique_ptr<task_base>> queued_tasks;
  std::atomic<worker_affinity_t> worker_should_have_tasks = ATOMIC_VAR_INIT(0);
  std::unordered_multimap<void*, std::unique_ptr<task_base>> blocking_tasks;
  std::mutex block_mutex;
  unsigned int fp_exceptions;
  unsigned int fp_exception_mask;
  future<int> result;
};
}

#endif
