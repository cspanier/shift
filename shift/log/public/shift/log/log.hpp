#ifndef SHIFT_LOG_LOG_HPP
#define SHIFT_LOG_LOG_HPP

#include <sstream>
#include <shift/core/core.hpp>
#include <shift/core/types.hpp>

namespace shift::log
{
/// Severity of log messages sent through a channel.
enum class channel_severity
{
  debug,
  info,
  warning,
  error,
  exception
};

/// The log priority can be set per log entry. High priority logs are always
/// written and are not filtered by the anti-spam mechanism.
enum class channel_priority
{
  normal,
  high
};

/// The color code used for escaped color formatting.
enum class color : char
{
  black = '0',
  maroon = '1',
  green = '2',
  navy = '3',
  purple = '4',
  olive = '5',
  teal = '6',
  gray = '7',
  silver = '8',
  red = '9',
  lime = 'A',
  blue = 'B',
  fuchsia = 'C',
  yellow = 'D',
  aqua = 'E',
  white = 'F',
  COUNT
};

static constexpr int status_channel = -1;
static constexpr int debug_channel = -2;
static constexpr int info_channel = -3;
static constexpr int warning_channel = -4;
static constexpr int error_channel = -5;
static constexpr int exception_channel = -6;

/// Base class of log_line which transfers the log message to our log server.
class base_log
{
public:
  /// Constructor.
  base_log(int channel, channel_priority priority);

  /// Destructor that automatically flushes any remaining data to the server.
  ~base_log();

protected:
  int _channel;
  channel_priority _priority;
  int _uncaught_exceptions;
  std::stringbuf _message_buffer;
};

///
template <int Channel, bool Enabled = true>
class log_line : public std::basic_ostream<char>, protected base_log
{
public:
  enum
  {
    channel = Channel
  };

  /// Constructor.
  log_line(channel_priority priority = channel_priority::normal) noexcept
  : std::basic_ostream<char>(&_message_buffer), base_log(Channel, priority)
  {
  }

  log_line(const log_line&) = delete;
  log_line(log_line&&) = default;
  log_line& operator=(const log_line&) = delete;
  log_line& operator=(log_line&&) = default;

  /// Function operator simply returning the object itself.
  log_line& operator()()
  {
    return *this;
  }
};

///
template <int Channel>
class log_line<Channel, false>
{
public:
  enum
  {
    channel = Channel
  };

  /// Constructor.
  log_line(channel_priority /*priority*/ = channel_priority::normal)
  {
  }

  log_line(const log_line&) = delete;
  log_line(log_line&&) = default;
  log_line& operator=(const log_line&) = delete;
  log_line& operator=(log_line&&) = default;

  /// Function operator simply returning the object itself.
  log_line& operator()()
  {
    return *this;
  }
};

using status = log_line<status_channel>;
using debug = log_line<debug_channel>;
using info = log_line<info_channel>;
using warning = log_line<warning_channel>;
using error = log_line<error_channel>;
using exception = log_line<exception_channel>;
}

#endif
