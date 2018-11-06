#ifndef SHIFT_TASK_TASK_SYSTEM_HPP
#define SHIFT_TASK_TASK_SYSTEM_HPP

#include <cfloat>
#include <memory>
#include <shift/platform/fpexceptions.hpp>
#include <shift/core/types.hpp>
#include <shift/core/singleton.hpp>
#include "shift/task/task.hpp"
#include "shift/task/this_task.hpp"
#include "shift/task/when_any.hpp"
#include "shift/task/when_all.hpp"

namespace shift::task
{
class task_system : public core::singleton<task_system, core::create::on_stack>
{
public:
  /// Constructor.
  /// @param fp_exceptions
  ///   See platform::floating_point_exceptions for details.
  /// @param fp_exception_mask
  ///   See platform::floating_point_exceptions for details.
  task_system(unsigned int fp_exceptions = _MCW_EM &
                                           ~(_EM_INEXACT | _EM_UNDERFLOW),
              unsigned int fp_exception_mask = _MCW_EM);

  ///
  ~task_system();

  /// Overwrites the number of worker threads to create.
  /// @param count
  ///   The number of worker threads to create, or zero to let the task_system
  ///   automatically select the optimal number of worker threads based on the
  ///   current hardware.
  /// @remarks
  ///   This call is only meaningful before start is called. After that it has
  ///   no effect.
  task_system& num_workers(std::size_t count);

  /// Returns the number of worker threads.
  std::uint32_t num_workers() const;

  /// Start the primary task.
  /// @remarks
  ///   At any given time there may only be a single primary task.
  template <typename Function, typename... Args,
            ENABLE_IF((std::is_same_v<
                       std::invoke_result_t<std::decay_t<Function>>, int>))>
  task_system& start(Function&& primary_function, Args&&... args)
  {
    BOOST_ASSERT(!running());
    static_assert(
      std::is_same<typename task<Function, Args...>::result_t, int>::value,
      "The primary task must return a value of type int.");
    initialize(make_task(task_create_info{}.worker_affinity(1u),
                         std::forward<Function>(primary_function),
                         std::forward<Args>(args)...));
    return *this;
  }

  /// Waits until all tasks have been processed.
  /// @return
  ///   Returns the result of the primary task.
  int join();

  /// Returns whether the task_system is busy processing tasks.
  bool running() const noexcept;

private:
private:
  friend class mutex;
  friend class condition_variable;
  friend class detail::associated_state_base;

  template <typename Function, typename... Args>
  friend auto async(const task_create_info& /*create_info*/,
                    Function&& function, Args&&... args);

  ///
  void queue(std::unique_ptr<task_base> new_task);

  ///
  void queue_blocked(std::unique_ptr<task_base> new_task,
                     condition_variable& condition, mutex& lock);

  ///
  void lock_mutex(mutex& lock);

  ///
  void unlock_mutex(mutex& lock);

  ///
  void wait_condition(condition_variable& condition, mutex& lock);

  ///
  void notify_condition(condition_variable& condition, std::size_t count);

private:
  friend class this_task;

  ///
  void initialize(std::unique_ptr<task_result<int>> primary_task);

  /// Returns the task being executed by the current worker thread.
  /// @remarks
  ///   Calling this method from a non-task-worker-thread results in undefined
  ///   behavior.
  task_base* current_task() const;

  /// Returns the id of the current worker thread.
  /// @remarks
  ///   Calling this method from a non-task-worker-thread results in undefined
  ///   behavior.
  worker_id_t current_worker_id() const;

  struct impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
