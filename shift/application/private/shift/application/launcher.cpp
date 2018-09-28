#include "shift/application/launcher.h"
#include <shift/log/log_server.h>
#include <shift/log/log.h>
#include <shift/platform/console.h>
#include <shift/platform/environment.h>
#include <shift/platform/console.h>
#include <shift/platform/environment.h>

namespace shift::application
{
launcher_core::launcher_core(int argc, char* argv[])
: _visible_options("Allowed options"),
  _hidden_options("Hidden options"),
  _argc(argc),
  _argv(argv),
  _executable_path(platform::environment::executable_path()),
  _initial_working_directory(_executable_path.parent_path().parent_path()),
  _pre_init_helper(*this),
  _crash_guard(_initial_working_directory / "crashes")
{
  namespace opt = boost::program_options;

  _visible_options.add_options()("help", "Shows this help message.");
  _visible_options.add_options()("silent", "Disables logging to console.");
  _visible_options.add_options()("no-logfile", "Disables logging to file.");
  _visible_options.add_options()(
    "log-level", opt::value<std::string>(&log_level)->default_value("warn"),
    "Selects a log level (may be one of 'debug', 'info', 'warn', or 'error')");
  _visible_options.add_options()("log-arguments",
                                 "Writes all program arguments to the log.");
  _visible_options.add_options()(
    "show-console", opt::value<bool>(&show_console)->default_value(true),
    "Show or hide the console window");

  _hidden_options.add_options()("hidden-help",
                                "Shows all command-line arguments.");
  _hidden_options.add_options()("log-disable-timestamp",
                                "Disables timestamp information in logs.");
  _hidden_options.add_options()("log-disable-channel",
                                "Disables channel name information in logs.");
  _hidden_options.add_options()(
    "log-disable-separator",
    "Disables separator between log channel and message data.");
  _hidden_options.add_options()(
    "log-record-separator",
    "Prints the ASCII character 0x1E at the end of each log record.");
  _hidden_options.add_options()(
    "single-instance",
    opt::value<bool>(&limit_single_instance)->default_value(false),
    "When set to true, this prevents multiple instances of an application.");
}

launcher_core::~launcher_core()
{
  log::log_server::singleton_destroy();
}

int launcher_core::execute(std::function<int()> handler)
{
  using namespace boost::signals2;

  int result = EXIT_FAILURE;
  try
  {
    bool quit;
    std::tie(result, quit) = process_options();
    if (result != EXIT_SUCCESS || quit)
      return result;

    start();

    // Register signal handler to catch events like close console window,
    // ctrl+break, ctrl+c...
    scoped_connection connection =
      _signal.handler.connect([&](platform::signal signal) {
        shared_connection_block block{connection};
        if (_signal_handler)
          return _signal_handler(signal);
        else
          return true;
      });

    result = handler();
  }
  catch (boost::exception& e)
  {
    std::cerr << boost::diagnostic_information(e) << std::endl;
    result = EXIT_FAILURE;
  }
  catch (std::exception& e)
  {
    std::cerr << "Caught unhandled standard exception: " << e.what()
              << std::endl;
    result = EXIT_FAILURE;
  }
  catch (...)
  {
    std::cerr << "Caught unhandled exception of unknown type." << std::endl;
    result = EXIT_FAILURE;
  }

  stop();

  return result;
}

int& launcher_core::argc()
{
  return _argc;
}

char** launcher_core::argv()
{
  return _argv;
}

boost::filesystem::path launcher_core::executable_path() const
{
  return _executable_path;
}

boost::filesystem::path launcher_core::initial_working_directory() const
{
  return _initial_working_directory;
}

void launcher_core::start()
{
  /// NOP.
}

void launcher_core::stop()
{
  /// NOP.
}

std::pair<int, bool> launcher_core::process_options()
{
  namespace opt = boost::program_options;

  int result = EXIT_SUCCESS;
  opt::variables_map options;
  try
  {
    boost::program_options::options_description all_options;
    all_options.add(_visible_options).add(_hidden_options);
    opt::store(opt::command_line_parser(_argc, _argv)
                 .options(all_options)
                 .positional(_positional_options)
                 .allow_unregistered()
                 .run(),
               options);
    opt::notify(options);
  }
  catch (opt::error& ex)
  {
    log::exception() << ex.what();
    result = EXIT_FAILURE;
  }

  if ((options.count("help") != 0u) || result != EXIT_SUCCESS)
  {
    log::info() << _visible_options;
    return {result, true};
  }

  if (options.count("hidden-help") != 0u)
  {
    log::info() << _visible_options << "\n" << _hidden_options;
    return {result, true};
  }

  auto& log_server = log::log_server::singleton_instance();
  if (log_level == "error")
  {
    log_server.enable_channel(log::warning_channel, false);
    log_server.enable_channel(log::info_channel, false);
    log_server.enable_channel(log::debug_channel, false);
  }
  else if (log_level == "warn")
  {
    log_server.enable_channel(log::info_channel, false);
    log_server.enable_channel(log::debug_channel, false);
  }
  else if (log_level == "info")
  {
    log_server.enable_channel(log::debug_channel, false);
  }
  else if (log_level != "debug")
  {
    log::error() << "Invalid value for parameter log-level.";
    return {EXIT_FAILURE, true};
  }

  if (limit_single_instance && !_single_instance.check())
  {
    log::error() << "You cannot start multiple instances of this process.";
    return {EXIT_FAILURE, true};
  }

  if (!show_console)
    platform::console::singleton_instance().hide();

  return {result, false};
}

launcher_core::pre_init_helper_t::pre_init_helper_t(launcher_core& launcher)
{
  bool silent = false;
  bool no_logfile = false;
  bool log_arguments = false;
  bool log_enable_timestamp = true;
  bool log_enable_channel = true;
  bool log_enable_separator = true;
  bool log_record_separator = false;

  // Early check if console logging shall be enabled.
  for (int i = 1; i < launcher._argc; ++i)
  {
    std::string argument{launcher._argv[i]};
    if (argument == "--silent")
      silent = true;
    else if (argument == "--no-logfile")
      no_logfile = true;
    else if (argument == "--log-arguments")
      log_arguments = true;
    else if (argument == "--log-disable-timestamp")
      log_enable_timestamp = false;
    else if (argument == "--log-disable-channel")
      log_enable_channel = false;
    else if (argument == "--log-disable-separator")
      log_enable_separator = false;
    else if (argument == "--log-record-separator")
      log_record_separator = true;
  }

  auto& log_server = log::log_server::singleton_create();
  if (!silent)
  {
    log_server.add_console_sink(log_enable_timestamp, log_enable_channel,
                                log_enable_separator, log_record_separator);
    /// ToDo: Refactor the Curses based console to Create a Curses based
    /// console sink only if there is a console attached to the process.
    // if (platform::Console::singleton_instance().exists())
  }

  // Change the current working directory to the parent folder of the folder
  // which contains the current executable file (e.g. /a/b/exe -> /a/).
  namespace fs = boost::filesystem;
  if (launcher._initial_working_directory != fs::current_path())
  {
    log::info() << "Changing current working directory to "
                << launcher._initial_working_directory;
    fs::current_path(launcher._initial_working_directory);
  }

  if (!no_logfile)
  {
    log_server.add_file_sink("log",
                             launcher._executable_path
                               .filename()
#if defined(SHIFT_PLATFORM_WINDOWS)
                               // Remove .exe extension on Windows.
                               .replace_extension()
#endif
                               .generic_string(),
                             std::string{}, log_enable_timestamp,
                             log_enable_channel, log_enable_separator);
  }

  if (log_arguments)
  {
    log::info log;
    log << "Command line:";
    for (int i = 0; i < launcher._argc; ++i)
      log << " " << launcher._argv[i];
  }
}
}
