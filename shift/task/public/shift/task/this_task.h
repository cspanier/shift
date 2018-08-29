#ifndef SHIFT_TASK_THIS_TASK_H
#define SHIFT_TASK_THIS_TASK_H

#include "shift/task/types.h"

namespace shift::task
{
///
class this_task
{
public:
  /// Calls task_base::yield() on the task currently being executed by this
  /// worker thread.
  static void yield();

  /// Returns the id of the current worker thread.
  /// @remarks
  ///   Calling this method from outside a task results in undefined behavior.
  /// @remarks
  ///   This effectively calls task_system::current_task, but the function is
  ///   present to reduce include dependencies.
  static worker_id_t current_worker_id();

  /// Calls task_base::id() on the task currently being executed by this
  /// worker thread.
  /// @remarks
  ///   Calling this method from outside a task results in undefined behavior.
  static task_id_t task_id();

  /// Forces this task to be processed only on the current thread.
  static void bind_to_current_worker();

  /// Forces the task to be processed only by specific worker threads.
  static void worker_affinity(worker_affinity_t affinity);

  /// Returns the worker affinity mask.
  static worker_affinity_t worker_affinity();
};
}

#endif
