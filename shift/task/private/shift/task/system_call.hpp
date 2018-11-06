#ifndef SHIFT_TASK_SYSTEM_CALL_HPP
#define SHIFT_TASK_SYSTEM_CALL_HPP

#include <memory>
#include <variant>
#include "shift/task/types.hpp"
#include "shift/task/condition_variable.hpp"

namespace shift::task
{
struct task_queue
{
  std::unique_ptr<task_base> task;
};

struct task_queue_blocked
{
  std::unique_ptr<task_base> task;
  condition_variable* condition = nullptr;
  mutex* lock = nullptr;
};

struct task_yield
{
};

struct task_lock_mutex
{
  mutex* lock = nullptr;
};

struct task_unlock_mutex
{
  mutex* lock = nullptr;
};

struct task_wait_condition
{
  condition_variable* condition = nullptr;
  mutex* lock = nullptr;
};

struct task_notify_condition
{
  condition_variable* condition = nullptr;
  std::size_t count;
};

struct task_end
{
};

using system_call_t =
  std::variant<task_queue, task_queue_blocked, task_yield, task_lock_mutex,
               task_wait_condition, task_end>;
}

#endif
