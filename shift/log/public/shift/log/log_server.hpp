#ifndef SHIFT_LOG_LOGSERVER_HPP
#define SHIFT_LOG_LOGSERVER_HPP

#include <chrono>
#include <string>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/singleton.hpp>
#include "shift/log/log.hpp"

namespace shift::log
{
class log_sink;

/// A central log server which synchronizes parallel logging.
class log_server : public core::singleton<log_server, core::create::using_new>
{
public:
  /// Default constructor.
  log_server();

  /// Destructor.
  ~log_server();

  /// Registers a console sink.
  void add_console_sink(bool enable_timestamp = true,
                        bool enable_channel = true,
                        bool enable_separator = true,
                        bool record_separator = false);

  /// Registers a file sink.
  void add_file_sink(boost::filesystem::path log_folder,
                     std::string_view application_name, std::string_view suffix,
                     bool enable_timestamp = true, bool enable_channel = true,
                     bool enable_separator = true);

  /// Register a custom log sink.
  void add_custom_sink(std::unique_ptr<log_sink>&& sink);

  /// Sets configuration properties for the specified log channel.
  void configure_channel(int channel, channel_severity severity,
                         channel_priority priority, std::string system_name);

  /// Returns the channel's assigned name.
  std::string channel_name(int channel);

  /// Enables or disables the specified channel.
  void enable_channel(int channel, bool enable);

  /// Checks if a channel is enabled.
  /// @return
  ///   True if the specified channel exists and is enabled. False otherwise.
  bool channel_enabled(int channel) const;

  /// Writes a log message.
  void write(int channel, channel_priority priority, std::string message);

  /// Sets the anti-spam delay time for which duplicate messages will be
  /// filtered.
  void spam_delay(std::chrono::milliseconds delay);

  /// Returns the time point the server was created at.
  std::chrono::system_clock::time_point start_time() const;

  /// Returns the currently set default log system name.
  std::string_view default_system_name() const;

  /// Sets the default log system name.
  void default_system_name(std::string new_name);

private:
  struct impl;

  std::unique_ptr<impl> _impl;
};
}

#endif
