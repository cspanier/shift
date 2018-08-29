#ifndef SHIFT_LOG_LOGSINK_H
#define SHIFT_LOG_LOGSINK_H

#include <cstddef>
#include <chrono>
#include <string>
#include <string_view>
#include "shift/log/log.h"
#include "shift/log/log_channel_config.h"

namespace shift::log
{
/// Base class for consumers of log messages.
class log_sink
{
public:
  /// Destructor.
  virtual ~log_sink();

  /// Notifies the log sink about new dimensions.
  virtual void max_channel_name_length(std::size_t length);

  /// Writes a log message.
  /// @remarks
  ///   Thread safety is assured by a log server internal lock.
  virtual void write(std::chrono::system_clock::duration time_since_start,
                     int channel, const channel_config& channel_config,
                     channel_priority priority, std::string_view message,
                     bool is_spam) = 0;

protected:
  std::size_t _max_channel_name_length = 0;
};
}

#endif
