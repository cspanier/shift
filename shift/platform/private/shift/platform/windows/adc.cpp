#include "shift/platform/adc.hpp"
#include "shift/platform/types.hpp"
#include <shift/core/exception.hpp>

namespace shift::platform
{
adc::adc(std::string source) : _source(std::move(source))
{
}

adc::~adc()
{
}

std::string_view adc::source() const
{
  return _source;
}

void adc::update()
{
  _handle = -1;
  _value = 0;
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
