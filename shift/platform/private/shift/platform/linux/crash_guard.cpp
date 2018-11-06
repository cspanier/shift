#include "shift/platform/crash_guard.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <iostream>
#include <client/linux/handler/exception_handler.h>

namespace shift::platform
{
/// The Linux specific GrashGuard implementation.
class crash_guard::impl
{
public:
  /// Constructor setting up all handlers.
  impl(const std::string& dump_path);

  impl(const impl&) = delete;
  impl(impl&&) = delete;
  ~impl() = default;
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

private:
  static bool dump_callback(
    const google_breakpad::MinidumpDescriptor& descriptor, void* /*context*/,
    bool succeeded);

  google_breakpad::MinidumpDescriptor _descriptor;
  google_breakpad::ExceptionHandler _exception_handler;
};

crash_guard::impl::impl(const std::string& dump_path)
: _descriptor(dump_path),
  _exception_handler(_descriptor, nullptr, dump_callback, nullptr, true, -1)
{
}

bool crash_guard::impl::dump_callback(
  const google_breakpad::MinidumpDescriptor& descriptor, void* /*context*/,
  bool succeeded)
{
  std::cerr << "Dump path: " << descriptor.path() << std::endl;
  return succeeded;
}

crash_guard::crash_guard(boost::filesystem::path dump_path)
{
  using namespace boost::filesystem;

  if (!exists(dump_path))
  {
    boost::system::error_code error;
    create_directories(dump_path, error);
    if (error)
      dump_path = current_path();
  }
  else if (!is_directory(dump_path))
  {
    BOOST_ASSERT(false);
    dump_path = current_path();
  }

  _impl = std::make_unique<impl>(dump_path.generic_string());
}

crash_guard::~crash_guard() = default;
}
