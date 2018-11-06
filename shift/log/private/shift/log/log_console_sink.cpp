#include "shift/log/log_console_sink.hpp"
#include "shift/platform/console.hpp"
#include <iomanip>
#include <iostream>
#include <string>

namespace shift::log
{
log_console_sink::log_console_sink(bool enable_timestamp, bool enable_channel,
                                   bool enable_separator, bool record_separator)
: _enable_timestamp(enable_timestamp),
  _enable_channel(enable_channel),
  _enable_separator(enable_separator),
  _record_separator(record_separator)
{
}

log_console_sink::~log_console_sink() = default;

void log_console_sink::write(
  std::chrono::system_clock::duration time_since_start, int channel,
  const channel_config& channel_config, channel_priority /*priority*/,
  std::string_view message, bool is_spam)
{
  if (channel == status_channel)
  {
    platform::console::singleton_instance().title(message);
    return;
  }
  if (is_spam)
    return;

  if (_enable_timestamp)
  {
    std::cout << std::setw(6) << std::setfill('0')
              << (std::chrono::duration_cast<std::chrono::seconds>(
                    time_since_start)
                    .count() %
                  1000000)
              << ":" << std::setw(3) << std::setfill('0')
              << (std::chrono::duration_cast<std::chrono::milliseconds>(
                    time_since_start)
                    .count() %
                  1000)
              << " ";
  }
  if (_enable_channel)
  {
    std::cout << channel_config.channel_name
              << std::string(channel_config.indentation, ' ');
  }
  if (_enable_separator)
    std::cout << ": ";
  std::cout << message;
  if (_record_separator)
    std::cout << '\x1E';
  std::cout << std::endl;
}
}
