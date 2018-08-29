#ifndef SHIFT_TASK_LAUNCHER_H
#define SHIFT_TASK_LAUNCHER_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/task/program_options.h"
#include "shift/task/task_system.h"

namespace shift::task
{
template <typename NextModule>
class launcher : public NextModule, public program_options
{
public:
  using base_t = NextModule;

  ///
  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    namespace opt = boost::program_options;

    base_t::_visible_options.add_options()(
      "task-num-workers",
      opt::value<std::uint32_t>(&_num_workers)->default_value(0),
      "Number of worker threads to use to process tasks. The default value "
      "of zero lets the application automatically chose the number of "
      "threads.");
    base_t::_visible_options.add_options()(
      "floating-point-exceptions",
      opt::value<bool>(&_floating_point_exceptions)->default_value(true),
      "Enable floating-point exceptions.");
  }

  ///
  int execute(std::function<int()> handler) override
  {
    return base_t::execute([&]() -> int {
      return task_system(_floating_point_exceptions
                           ? _MCW_EM & ~(_EM_INEXACT | _EM_UNDERFLOW)
                           : 0)
        .num_workers(_num_workers)
        .start(handler)
        .join();
    });
  }
};
}

#endif
