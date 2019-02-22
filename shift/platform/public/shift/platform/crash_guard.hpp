#ifndef SHIFT_PLATFORM_CRASHHANDLER_HPP
#define SHIFT_PLATFORM_CRASHHANDLER_HPP

#include <cstdint>
#include <memory>
#include <list>
#include <filesystem>
#include <shift/core/bit_field.hpp>
#include <shift/core/singleton.hpp>

namespace shift::platform
{
struct memory_region
{
  std::uint64_t address;
  std::uint32_t length;

  bool operator==(const struct memory_region& other) const
  {
    return address == other.address;
  }

  bool operator==(const void* other) const
  {
    return address == reinterpret_cast<std::uint64_t>(other);
  }
};
using memory_region_list = std::list<memory_region>;

struct minidump
{
  struct stream
  {
    enum class Type : std::uint32_t
    {
      metadata = 0xc0ffee01
    };
  };
};

struct MetaInformation
{
  std::uint32_t requesting_thread_id = 0;
  std::uint32_t dump_thread_id = 0;
};

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
