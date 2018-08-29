#include "shift/task/this_task.h"
#include "shift/task/task.h"
#include "shift/task/task_system.h"

namespace shift::task
{
void this_task::yield()
{
  auto* task = task_system::singleton_instance().current_task();
  BOOST_ASSERT(task);
  if (task)
    task->yield();
}

worker_id_t this_task::current_worker_id()
{
  return task_system::singleton_instance().current_worker_id();
}

task_id_t this_task::task_id()
{
  auto* task = task_system::singleton_instance().current_task();
  BOOST_ASSERT(task);
  if (task)
    return task->id();
  else
    return 0;
}

void this_task::bind_to_current_worker()
{
  auto* task = task_system::singleton_instance().current_task();
  auto worker_id = task_system::singleton_instance().current_worker_id();
  BOOST_ASSERT(task);
  BOOST_ASSERT(worker_id != no_worker);
  if (task)
    task->worker_affinity(worker_id);
}

void this_task::worker_affinity(worker_affinity_t affinity)
{
  auto* task = task_system::singleton_instance().current_task();
  BOOST_ASSERT(task);
  if (!task)
    return;
  /// ToDo: We cannot simply change the worker affinity mask and be donewith
  /// it!
  BOOST_ASSERT(false);
  task->worker_affinity(affinity);
}

worker_affinity_t this_task::worker_affinity()
{
  auto* task = task_system::singleton_instance().current_task();
  BOOST_ASSERT(task);
  if (!task)
    return 0;
  return task->worker_affinity();
}
}
