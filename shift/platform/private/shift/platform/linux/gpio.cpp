#include "shift/platform/gpio.h"
#include "shift/platform/types.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <chrono>
#include <array>
#include <poll.h>
#include <fcntl.h>

namespace shift::platform
{
gpio::gpio(std::string source) : _source(std::move(source))
{
  _direction_file.open(
    (boost::filesystem::path(_source) / "direction").string(),
    std::ios_base::out);
  _edge_file.open((boost::filesystem::path(_source) / "edge").string(),
                  std::ios_base::out);
  /// ToDo: Read the initial state rather than setting a predefined one.
  direction(io_direction::in);
  edge(edge_trigger::none);
}

gpio::~gpio()
{
  close();
  if (_direction_file.is_open())
    _direction_file.close();
  if (_edge_file.is_open())
    _edge_file.close();
}

std::string_view gpio::source() const
{
  return _source;
}

bool gpio::open()
{
  if (_value_handle >= 0)
    close();
  std::string path = (boost::filesystem::path(_source) / "value").string();
  if (_direction == io_direction::in)
  {
    if (_edge == edge_trigger::none)
      _value_handle = ::open(path.c_str(), O_RDONLY);
    else
      _value_handle = ::open(path.c_str(), O_RDONLY | O_NONBLOCK);
  }
  else
    _value_handle = ::open(path.c_str(), O_WRONLY);
  return _value_handle >= 0;
}

void gpio::close()
{
  if (_value_handle >= 0)
  {
    ::close(_value_handle);
    _value_handle = -1;
  }
}

void gpio::direction(gpio::io_direction value)
{
  if (!_direction_file.is_open())
    return;

  BOOST_ASSERT(_value_handle < 0);
  if (_value_handle >= 0)
    return;

  if (value == io_direction::in)
    _direction_file << "in" << std::flush;
  else
    _direction_file << "out" << std::flush;
  _direction = value;
}

gpio::io_direction gpio::direction() const
{
  return _direction;
}

void gpio::edge(gpio::edge_trigger value)
{
  BOOST_ASSERT(_direction == io_direction::in);
  BOOST_ASSERT(_value_handle < 0);
  if (_direction != io_direction::in || _value_handle >= 0 ||
      !_edge_file.is_open())
  {
    return;
  }

  switch (value)
  {
  case edge_trigger::none:
    _edge_file << "none" << std::flush;
    break;

  case edge_trigger::rising:
    _edge_file << "rising" << std::flush;
    break;

  case edge_trigger::falling:
    _edge_file << "falling" << std::flush;
    break;

  case edge_trigger::both:
    _edge_file << "both" << std::flush;
    break;

  default:
    BOOST_ASSERT(false);
    return;
  }
  _edge = value;
}

gpio::edge_trigger gpio::edge() const
{
  return _edge;
}

void gpio::inverted(bool value)
{
  BOOST_ASSERT(_value_handle < 0);
  if (_value_handle >= 0)
    return;

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

void gpio::update(bool forceUpdate)
{
  BOOST_ASSERT(_value_handle >= 0);
  if (_value_handle < 0 || _direction != io_direction::in)
    return;

  // The file will either contain "0\n" or "1\n".
  std::array<char, 2> buffer;
  auto len = ::pread(_value_handle, buffer.data(), buffer.size(), 0);
  if (len == 0)
    return;
  bool newValue = (buffer[0] == '1');
  if (_value != newValue || boost::indeterminate(_value) || forceUpdate)
  {
    _value = newValue;
    if (on_changed)
      on_changed(*this, value() != nullptr);
  }
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

  BOOST_ASSERT(_value_handle >= 0);
  if (_direction != io_direction::out || _value_handle < 0 ||
      boost::indeterminate(_value))
  {
    return;
  }

  auto buffer = (_value != nullptr) ? '1' : '0';
  write(_value_handle, &buffer, sizeof(buffer));
  fsync(_value_handle);
}

gpio::watch_event gpio::watch(const std::vector<std::shared_ptr<gpio>>& inputs,
                              std::chrono::milliseconds timeout)
{
  std::vector<pollfd> pollDescriptors;
  pollDescriptors.resize(inputs.size());
  std::size_t index = 0;
  for (auto& input : inputs)
  {
    pollDescriptors[index].fd = input->handle();
    pollDescriptors[index].events = POLLPRI | POLLERR;
    pollDescriptors[index].revents = 0;
    ++index;
  }
  /// ToDo: Disable signals, see ppoll (man 2).
  auto result = poll(pollDescriptors.data(), inputs.size(),
                     static_cast<int>(timeout.count()));
  if (result < 0)
    return watch_event::error;
  else if (result == 0)
    return watch_event::timeout;

  auto input = inputs.begin();
  for (std::size_t index = 0; index < inputs.size(); ++index, ++input)
  {
    if ((pollDescriptors[index].revents & POLLPRI) != 0)
      (*input)->update();
  }
  return watch_event::changed;
}
}
