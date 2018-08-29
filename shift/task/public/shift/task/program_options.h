#ifndef SHIFT_TASK_PROGRAM_OPTIONS_H
#define SHIFT_TASK_PROGRAM_OPTIONS_H

namespace shift::task
{
struct program_options
{
  std::uint32_t _num_workers;
  bool _floating_point_exceptions;
};
}

#endif
