#include "shift/platform/adc.h"
#include "shift/platform/types.h"
#include <shift/core/exception.h>
#include <gsl/gsl>
#include <array>
#include <fcntl.h>

namespace shift::platform
{
adc::adc(std::string source) : _source(std::move(source))
{
  _handle = ::open(_source.c_str(), O_RDONLY);
  if (_handle == -1)
  {
    BOOST_THROW_EXCEPTION(core::file_open_error()
                          << core::file_name_info(_source));
  }
}

adc::~adc()
{
  if (_handle >= 0)
  {
    ::close(_handle);
    _handle = -1;
  }
}

std::string_view adc::source() const
{
  return _source;
}

void adc::update()
{
  std::array<char, 16> buffer;
  auto length = static_cast<std::ptrdiff_t>(
    ::pread(_handle, buffer.data(), buffer.size() - 1, 0));
  if (length < 0)
    return;
  gsl::at(buffer, length) = 0;
  auto value = std::atoi(buffer.data());

  if (_min > value)
    _min = value;
  if (_max < value)
    _max = value;

  if (_inverted)
    value = _max - value + _min;
  if (_value != value)
  {
    _value = value;
    if (on_changed)
      on_changed(*this, _value);
  }
}

int adc::value() const
{
  return _value;
}

int adc::min() const
{
  return _min;
}

void adc::min(int value)
{
  _min = value;
}

int adc::max() const
{
  return _max;
}

void adc::max(int value)
{
  _max = value;
}

bool adc::inverted() const
{
  return _inverted;
}

void adc::inverted(bool value)
{
  _inverted = value;
}

float adc::postScale() const
{
  return _postScale;
}

void adc::postScale(float value)
{
  _postScale = value;
}
}
