#ifndef SHIFT_PLATFORM_CRASHHANDLER_HPP
#define SHIFT_PLATFORM_CRASHHANDLER_HPP

#include <memory>
#include <filesystem>
#include <shift/core/singleton.hpp>

namespace shift::platform
{
/// The crash guard registers a global system exception handler which attempts
/// to gather as much information as possible in an event of a fatal crash.
class crash_guard : public core::singleton<crash_guard, core::create::on_stack>
{
public:
  /// Constructor.
  crash_guard(std::filesystem::path dump_path);

  /// Destructor.
  ~crash_guard();

private:
  class impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
