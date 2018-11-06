#include "shift/log/log_sink.hpp"

namespace shift::log
{
log_sink::~log_sink() = default;

void log_sink::max_channel_name_length(std::size_t length)
{
  _max_channel_name_length = length;
}
}
