#include "shift/task/future.hpp"
#include "shift/task/task_system.hpp"

namespace shift::task::detail
{
void associated_state_base::queue_task(std::unique_ptr<task_base> new_task)
{
  if (ready != 0)
    task_system::singleton_instance().queue(std::move(new_task));
  else
  {
    task_system::singleton_instance().queue_blocked(std::move(new_task),
                                                    condition, state_mutex);
  }
}
}
