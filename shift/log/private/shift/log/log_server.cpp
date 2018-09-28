#include "shift/log/log_server.h"
#include "shift/log/log_server_impl.h"
#include "shift/log/log.h"
#include "shift/log/log_file_sink.h"
#include "shift/log/log_console_sink.h"
#include <shift/platform/assert.h>
#include <deque>
#include <list>
#include <mutex>
#include <ratio>
#include <string>
#include <memory>
#include <functional>
#include <utility>

namespace shift::log
{
log_server::log_server() : _impl(std::make_unique<impl>())
{
  configure_channel(debug::channel, channel_severity::debug,
                    channel_priority::normal, "");
  enable_channel(debug::channel, true);
  configure_channel(info::channel, channel_severity::info,
                    channel_priority::normal, "");
  enable_channel(info::channel, true);
  configure_channel(warning::channel, channel_severity::warning,
                    channel_priority::high, "");
  enable_channel(warning::channel, true);
  configure_channel(error::channel, channel_severity::error,
                    channel_priority::high, "");
  enable_channel(error::channel, true);
  configure_channel(exception::channel, channel_severity::exception,
                    channel_priority::high, "");
  enable_channel(exception::channel, true);
  configure_channel(status::channel, channel_severity::info,
                    channel_priority::high, "");
  enable_channel(status::channel, true);
}

log_server::~log_server() = default;

void log_server::add_console_sink(bool enable_timestamp, bool enable_channel,
                                  bool enable_separator, bool record_separator)
{
  std::lock_guard lock(_impl->mutex);
  _impl->sinks.push_back(std::make_unique<log_console_sink>(
    enable_timestamp, enable_channel, enable_separator, record_separator));
}

void log_server::add_file_sink(boost::filesystem::path log_folder,
                               std::string_view application_name,
                               std::string_view suffix, bool enable_timestamp,
                               bool enable_channel, bool enable_separator)
{
  std::lock_guard lock(_impl->mutex);
  _impl->sinks.push_back(std::make_unique<log_file_sink>(
    log_folder, application_name, suffix, enable_timestamp, enable_channel,
    enable_separator));
}

void log_server::add_custom_sink(std::unique_ptr<log_sink>&& sink)
{
  std::lock_guard lock(_impl->mutex);
  _impl->sinks.push_back(std::move(sink));
}

void log_server::configure_channel(int channel, channel_severity severity,
                                   channel_priority priority,
                                   std::string system_name)
{
  auto& config = _impl->config[channel];
  config.severity = severity;
  config.priority = priority;
  config.system_name = std::move(system_name);
  switch (severity)
  {
  case channel_severity::debug:
    config.channel_name = "debug";
    break;

  case channel_severity::info:
    config.channel_name = "info";
    break;

  case channel_severity::warning:
    config.channel_name = "warning";
    break;

  case channel_severity::error:
    config.channel_name = "error";
    break;

  case channel_severity::exception:
    config.channel_name = "exception";
    break;

  default:
    BOOST_ASSERT(false);
    config.channel_name = "unknown";
  }
  config.indentation = 0;

  std::size_t max_channel_name_length = 3;
  for (auto& i : _impl->config)
  {
    if (!i.second.enabled)
      continue;
    std::size_t length = i.second.channel_name.length();
    if (length > max_channel_name_length)
      max_channel_name_length = length;
  }
  for (auto& i : _impl->config)
  {
    i.second.indentation =
      max_channel_name_length - i.second.channel_name.length();
  }

  std::lock_guard lock(_impl->mutex);
  for (auto& sink : _impl->sinks)
    sink->max_channel_name_length(max_channel_name_length);
}

std::string log_server::channel_name(int channel)
{
  auto config = _impl->config.find(channel);
  if (config != _impl->config.end())
    return config->second.channel_name;
  else
    return "";
}

void log_server::enable_channel(int channel, bool enable)
{
  auto config = _impl->config.find(channel);
  if (config == _impl->config.end())
    return;
  config->second.enabled = enable;
}

bool log_server::channel_enabled(int channel) const
{
  auto config = _impl->config.find(channel);
  if (config == _impl->config.end())
    return false;
  return config->second.enabled;
}

void log_server::write(int channel, channel_priority priority,
                       std::string message)
{
  // Remove trailing whitespace.
  static const std::string whitespace = "\n\r\t ";
  while (!message.empty() &&
         whitespace.find(message.back()) != std::string::npos)
  {
    message.pop_back();
  }
  if (message.empty())
    return;

  std::lock_guard lock(_impl->mutex);

  auto config = _impl->config.find(channel);
  if (config == _impl->config.end() || !config->second.enabled)
    return;

  auto now = std::chrono::system_clock::now();

  // Purge old entries from the anti-spam history.
  while (!_impl->recent_history.empty() &&
         now - _impl->recent_history.front().first > _impl->spam_delay)
  {
    _impl->recent_history.pop_front();
  }

  bool is_spam = false;
  if (priority == channel_priority::normal &&
      config->second.priority == channel_priority::normal)
  {
    // Check if the message already has been written within the last
    // spam_delay milliseconds.
    if (_impl->spam_delay.count() > 0)
    {
      for (auto& history : _impl->recent_history)
      {
        if (message == history.second)
        {
          is_spam = true;
          break;
        }
      }
      if (!is_spam)
        _impl->recent_history.emplace_back(now, message);
    }
  }

  if (_impl->lock_count != 0u)
  {
    // Detected recursive logging from within a log sink.
    BOOST_ASSERT(false);
    return;
  }
  ++_impl->lock_count;

  auto time_since_start = now - _impl->startup;
  for (auto& sink : _impl->sinks)
  {
    sink->write(time_since_start, channel, config->second, priority, message,
                is_spam);
  }

  --_impl->lock_count;
}

void log_server::spam_delay(std::chrono::milliseconds delay)
{
  _impl->spam_delay = delay;
}

std::chrono::system_clock::time_point log_server::start_time() const
{
  return _impl->startup;
}

std::string_view log_server::default_system_name() const
{
  return _impl->default_system_name;
}

void log_server::default_system_name(std::string new_name)
{
  _impl->default_system_name = std::move(new_name);
}
}
