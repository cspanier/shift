#ifndef SHIFT_TASK_TYPES_H
#define SHIFT_TASK_TYPES_H

#include <memory>
#include <shift/core/bit_field.h>

namespace shift::task
{
namespace detail
{
  template <typename R>
  class associated_state;

  template <typename R>
  class state_manager;
}

struct worker_thread_t;

class condition_variable;
class mutex;

template <typename R>
class future;

template <typename R>
class promise;

class task_create_info;
class task_base;

template <typename Function, typename... Args>
class task;

///
template <typename Function, typename... Args>
std::unique_ptr<task<Function, Args...>> make_task(
  const task_create_info& create_info, Function&& function, Args&&... args)
{
  return std::make_unique<task<Function, Args...>>(
    create_info, std::forward<Function>(function), std::forward<Args>(args)...);
}

class this_task;
class task_system;

using worker_id_t = std::uint32_t;

/// Each worker gets uniquely identifies by a bit, thus allowing up to 32
/// workers right now.
using worker_affinity_t = std::uint32_t;
static constexpr worker_affinity_t no_worker = 0x00000000u;
static constexpr worker_affinity_t all_workers = 0xFFFFFFFFu;

using task_id_t = std::uint32_t;
}

#endif
