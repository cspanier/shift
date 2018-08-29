#include "shift/platform/gpio.h"
#include "shift/platform/types.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::platform
{
gpio::gpio(std::string source) : _source(std::move(source))
{
}

gpio::~gpio()
{
}

std::string_view gpio::source() const
{
  return _source;
}

bool gpio::open()
{
  return false;
}

void gpio::close()
{
}

void gpio::direction(gpio::io_direction value)
{
  _direction = value;
}

gpio::io_direction gpio::direction() const
{
  return _direction;
}

void gpio::edge(gpio::edge_trigger value)
{
  _edge = value;
}

gpio::edge_trigger gpio::edge() const
{
  return _edge;
}

void gpio::inverted(bool value)
{
  _inverted = value;
}

bool gpio::inverted() const
{
  return _inverted;
}

int gpio::handle() const
{
  return _value_handle;
}

void gpio::update(bool /*force_update*/)
{
  _value = boost::indeterminate;
}

boost::tribool gpio::value() const
{
  return _inverted ? !_value : _value;
}

void gpio::value(boost::tribool state)
{
  if (_inverted)
    _value = !state;
  else
    _value = state;
}

gpio::watch_event gpio::watch(
  const std::vector<std::shared_ptr<gpio>>& /*inputs*/,
  std::chrono::milliseconds /*timeout*/)
{
  return watch_event::error;
}
}
