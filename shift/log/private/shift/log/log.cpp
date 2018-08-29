#include "shift/log/log.h"
#include "shift/log/log_server.h"
#include <shift/platform/assert.h>
#include <string>

namespace shift::log
{
base_log::base_log(int channel, channel_priority priority)
: _channel(channel),
  _priority(priority),
  _uncaught_exceptions(std::uncaught_exceptions())
{
}

base_log::~base_log()
{
  // Don't log this message when an exception occured while creating it.
  if (_uncaught_exceptions != std::uncaught_exceptions())
    return;

  auto message = _message_buffer.str();
  if (!message.empty())
  {
    BOOST_ASSERT(log_server::singleton_instantiated());
    if (log_server::singleton_instantiated())
    {
      log_server::singleton_instance().write(_channel, _priority,
                                             std::move(message));
    }
  }
}
}
