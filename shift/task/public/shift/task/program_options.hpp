#ifndef SHIFT_TASK_PROGRAM_OPTIONS_HPP
#define SHIFT_TASK_PROGRAM_OPTIONS_HPP

namespace shift::task
{
struct program_options
{
  std::uint32_t _num_workers;
  bool _floating_point_exceptions;
};
}

#endif
