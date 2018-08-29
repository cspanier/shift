#ifndef SHIFT_TASK_TASK_CREATE_INFO_H
#define SHIFT_TASK_TASK_CREATE_INFO_H

#include "shift/task/types.h"

namespace shift::task
{
///
class task_create_info
{
public:
  task_create_info() = default;

  ///
  worker_affinity_t worker_affinity() const
  {
    return _worker_affinity;
  }

  ///
  task_create_info& worker_affinity(worker_affinity_t affinity)
  {
    _worker_affinity = affinity;
    return *this;
  };

private:
  worker_affinity_t _worker_affinity = all_workers;
};
}

#endif
