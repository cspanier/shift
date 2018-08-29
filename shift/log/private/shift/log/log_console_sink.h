#ifndef SHIFT_LOG_LOGCONSOLESINK_H
#define SHIFT_LOG_LOGCONSOLESINK_H

#include <cstddef>
#include <chrono>
#include <string>
#include "shift/core/singleton.h"
#include "shift/log/log_sink.h"

namespace shift::log
{
/// A simple console log sink using standard output.
class log_console_sink
: public log_sink,
  public core::singleton<log_console_sink, core::create::on_stack>
{
public:
  /// Default constructor.
  log_console_sink(bool enable_timestamp, bool enable_channel,
                   bool enable_separator, bool record_separator);

  /// Destructor.
  ~log_console_sink() override;

  /// @see LogSink::write.
  void write(std::chrono::system_clock::duration time_since_start, int channel,
             const channel_config& channel_config, channel_priority priority,
             std::string_view message, bool isSpam) override;

private:
  bool _enable_timestamp;
  bool _enable_channel;
  bool _enable_separator;
  bool _record_separator;
};
}

#endif
