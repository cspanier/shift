#ifndef SHIFT_APPLICATION_LAUNCHER_H
#define SHIFT_APPLICATION_LAUNCHER_H

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/exception.h>
#include <shift/core/at_exit_scope.h>
#include <shift/platform/single_instance.h>
#include <shift/platform/signal_handler.h>
#include <shift/platform/crash_guard.h>

namespace shift::application
{
/// A simple abstraction of the application's top level entry point.
/// It is meant to take care of the following common tasks:
/// - Switch the current working directory to the parent folder of the
///   application executable.
/// - Initialize the log::log_server singleton.
/// - Setup a console and file log sink.
/// - Install a crash_handler to write mini dumps in case of fatal errors.
/// - Top level exception handling.
class launcher_core
{
public:
  /// Constructor.
  launcher_core(int argc, char* argv[]);

  /// Destructor.
  virtual ~launcher_core();

  /// This is the primary entry point for each specific process type.
  /// It is directly being called by main().
  virtual int execute(std::function<int()> handler);

  ///
  int& argc();

  ///
  char** argv();

  auto signal_handler(std::function<bool(platform::signal signal)> handler)
  {
    _signal_handler = handler;
    return core::make_at_exit_scope([&]() { _signal_handler = nullptr; });
  }

  std::string log_level;
  bool show_console;
  bool limit_single_instance;

protected:
  /// Returns the application executable's complete path.
  boost::filesystem::path executable_path() const;

  /// Returns the application's initial executable path, which this class
  /// automatically changed to.
  boost::filesystem::path initial_working_directory() const;

  ///
  virtual void start();

  ///
  virtual void stop();

  boost::program_options::options_description _visible_options;
  boost::program_options::options_description _hidden_options;
  boost::program_options::positional_options_description _positional_options;

private:
  struct pre_init_helper_t
  {
    /// Constructor.
    pre_init_helper_t(launcher_core& launcher);
  };

  /// Processes all program arguments.
  std::pair<int, bool> process_options();

  int _argc;
  char** _argv;
  boost::filesystem::path _executable_path;
  boost::filesystem::path _initial_working_directory;
  pre_init_helper_t _pre_init_helper;
  platform::single_instance _single_instance;
  platform::crash_guard _crash_guard;
  platform::signal_handler _signal;

  /// A signal handler to catch system events like close console window,
  /// ctrl+break, ctrl+c and so on.
  std::function<bool(platform::signal signal)> _signal_handler;
};

template <template <typename> class... Modules>
struct launcher;

template <template <typename> class NextModule,
          template <typename> class... OtherModules>
struct launcher<NextModule, OtherModules...>
{
  using type = NextModule<typename launcher<OtherModules...>::type>;
};

template <>
struct launcher<>
{
  using type = launcher_core;
};

template <template <typename> class... Modules>
using launcher_t = typename launcher<Modules...>::type;
}

#endif
