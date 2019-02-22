#ifndef SHIFT_PLATFORM_ENVIRONMENT_HPP
#define SHIFT_PLATFORM_ENVIRONMENT_HPP

#include <string>
#include <filesystem>

namespace shift::platform
{
/// A collection of functions to query verious environmental variables and
/// settings.
class environment
{
public:
#define UNKNOWN_HOSTNAME "unknown-host"
#define UNKNOWN_USERNAME "unknown-user"

  /// Returns the name of the local computer or UNKNOWN_HOSTNAME if there was
  /// an issue aquiring the name.
  /// @remarks
  ///   On Windows platforms the name returned will be transformed into lower
  ///   case letters. On Linux platforms the original case is preserved
  ///   because names are case sensitive.
  static std::string hostname();

  /// Returns the name of the current user or UNKNOWN_USERNAME if there was an
  /// issue aquiring the name.
  /// @remarks
  ///   On Windows platforms the name returned will be transformed into lower
  ///   case letters. On Linux platforms the original case is preserved
  ///   because names are case sensitive.
  static std::string username();

  /// Returns a full path to the current executable.
  static std::filesystem::path executable_path();

  /// Assigns the current thread a readable name to support debugging.
  static void thread_debug_name(const std::string& name);

  /// Returns true if a debugger is detected to be attached to the process.
  static bool is_debugger_present();

  /// Attempt to break into the debugger.
  static void debug_break();
};
}

#endif
