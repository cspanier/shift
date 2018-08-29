#ifndef SHIFT_LOG_LOGSERVERIMPL_H
#define SHIFT_LOG_LOGSERVERIMPL_H

#include <cstddef>
#include <chrono>
#include <deque>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <string>
#include "shift/log/log_server.h"
#include "shift/log/log_sink.h"

namespace shift::log
{
struct log_server::impl
{
  std::unordered_map<int, channel_config> config;
  std::vector<std::unique_ptr<log_sink>> sinks;
  std::recursive_mutex mutex;
  std::size_t lock_count = 0;
  std::chrono::system_clock::time_point startup =
    std::chrono::system_clock::now();
  std::string default_system_name = "core";
  std::chrono::milliseconds spam_delay;
  std::deque<std::pair<std::chrono::system_clock::time_point, std::string>>
    recent_history;
};
}

#endif
