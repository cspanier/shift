#ifndef SHIFT_LOG_LOGCHANNELCONFIG_HPP
#define SHIFT_LOG_LOGCHANNELCONFIG_HPP

#include <string>
#include "shift/log/log.hpp"

namespace shift::log
{
///
struct channel_config
{
  bool enabled = false;
  channel_severity severity;
  channel_priority priority;
  std::string system_name;
  std::string channel_name;
  std::size_t indentation;
};
}

#endif
