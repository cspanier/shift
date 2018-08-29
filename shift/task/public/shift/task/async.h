#ifndef SHIFT_TASK_ASYNC_H
#define SHIFT_TASK_ASYNC_H

#include "shift/task/task_create_info.h"
#include "shift/task/task_system.h"

namespace shift::task
{
template <typename Function, typename... Args>
auto async(const task_create_info& create_info, Function&& function,
           Args&&... args)
{
  auto new_task = make_task(create_info, std::forward<Function>(function),
                            std::forward<Args>(args)...);
  auto result = new_task->get_future();
  task_system::singleton_instance().queue(std::move(new_task));
  return result;
}

template <
  typename Function, typename... Args,
  DISABLE_IF((std::is_same_v<std::decay_t<Function>, task_create_info>))>
auto async(Function&& function, Args&&... args)
{
  return async(task_create_info{}, std::forward<Function>(function),
               std::forward<Args>(args)...);
}

template <typename Function, typename... Args>
auto multi_async(worker_affinity_t affinity_mask, Function&& function,
                 Args&&... args)
{
  auto num_workers = task_system::singleton_instance().num_workers();
  std::vector<future<decltype(function(std::forward<Args>(args)...))>> results;
  results.reserve(num_workers);
  for (auto worker_id = 0u; worker_id < num_workers; ++worker_id)
  {
    if ((affinity_mask & (1u << worker_id)) != 0)
    {
      results.emplace_back(
        async(task_create_info{}.worker_affinity(1u << worker_id), function,
              std::forward<Args>(args)...));
    }
  }
  return results;
}
}

#endif
