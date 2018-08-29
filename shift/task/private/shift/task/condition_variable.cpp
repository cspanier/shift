#include "shift/task/condition_variable.h"
#include "shift/task/task_system.h"

namespace shift::task
{
condition_variable::condition_variable() = default;

condition_variable::~condition_variable() = default;

void condition_variable::notify_one() noexcept
{
  task_system::singleton_instance().notify_condition(*this, 1);
}

void condition_variable::notify_all() noexcept
{
  task_system::singleton_instance().notify_condition(*this, 0);
}

void condition_variable::notify_on_task_exit(std::unique_lock<mutex>& /*lock*/,
                                             int* /*ready*/)
{
  BOOST_ASSERT(false);
}

void condition_variable::wait(std::unique_lock<mutex>& lock)
{
  task_system::singleton_instance().wait_condition(*this, *lock.mutex());
}
}
