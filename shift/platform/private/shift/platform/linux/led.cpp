#include "shift/platform/led.h"
#include "shift/platform/types.h"
#include "shift/core/exception.h"

namespace shift::platform
{
Led::Led(std::string name) : _name(std::move(name))
{
  if (_name.empty())
    return;

  std::string path;

  path = "/sys/class/leds/" + _name + "/brightness";
  _brightnessFile.open(path, std::ios_base::out);
  if (!_brightnessFile.is_open())
  {
    BOOST_THROW_EXCEPTION(core::file_open_error()
                          << core::file_name_info(path));
  }
  brightness(0);

  path = "/sys/class/leds/" + _name + "/trigger";
  _triggerFile.open(path, std::ios_base::out);
  if (!_triggerFile.is_open())
  {
    BOOST_THROW_EXCEPTION(core::file_open_error()
                          << core::file_name_info(path));
  }
  trigger(Trigger::none);
}

void Led::trigger(Led::Trigger value)
{
  if (_name.empty())
    return;
  if (_triggerFile.is_open())
  {
    switch (value)
    {
    case Trigger::none:
      _triggerFile << "none" << std::flush;
      break;
    case Trigger::timer:
      _triggerFile << "timer" << std::flush;
      break;
    case Trigger::oneShot:
      _triggerFile << "oneshot" << std::flush;
      break;
    default:
      BOOST_ASSERT(false);
      return;
    }
  }
  if (value == Trigger::timer || value == Trigger::oneShot)
  {
    std::string path;

    path = "/sys/class/leds/" + _name + "/delay_on";
    _delayOnFile.open(path, std::ios_base::out);
    if (!_delayOnFile.is_open())
    {
      BOOST_THROW_EXCEPTION(core::file_open_error()
                            << core::file_name_info(path));
    }
    _delayOn = std::chrono::milliseconds(500);

    path = "/sys/class/leds/" + _name + "/delay_off";
    _delayOffFile.open(path, std::ios_base::out);
    if (!_delayOffFile.is_open())
    {
      BOOST_THROW_EXCEPTION(core::file_open_error()
                            << core::file_name_info(path));
    }
    _delayOff = std::chrono::milliseconds(500);

    path = "/sys/class/leds/" + _name + "/shot";
    _shotFile.open(path, std::ios_base::out);
    if (!_shotFile.is_open())
    {
      BOOST_THROW_EXCEPTION(core::file_open_error()
                            << core::file_name_info(path));
    }
  }
  else
  {
    _delayOnFile.close();
    _delayOffFile.close();
    _shotFile.close();
    _delayOn = std::chrono::milliseconds(0);
    _delayOff = std::chrono::milliseconds(0);
  }
  _trigger = value;
}

Led::Trigger Led::trigger() const
{
  return _trigger;
}

void Led::brightness(std::uint8_t value)
{
  if (!_brightnessFile.is_open())
    return;
  BOOST_ASSERT(_brightnessFile.is_open());
  _brightnessFile << static_cast<std::uint32_t>(value) << std::flush;
  _brightness = value;
}

std::uint8_t Led::brightness() const
{
  BOOST_ASSERT(_brightnessFile.is_open());
  return _brightness;
}

void Led::delayOn(std::chrono::milliseconds value)
{
  BOOST_ASSERT(_delayOnFile.is_open());
  if (!_delayOnFile.is_open())
    return;
  _delayOnFile << value.count() << std::flush;
  _delayOn = value;
}

std::chrono::milliseconds Led::delayOn() const
{
  BOOST_ASSERT(_delayOnFile.is_open());
  return _delayOn;
}

void Led::delayOff(std::chrono::milliseconds value)
{
  BOOST_ASSERT(_delayOffFile.is_open());
  if (!_delayOffFile.is_open())
    return;
  _delayOffFile << value.count() << std::flush;
  _delayOff = value;
}

std::chrono::milliseconds Led::delayOff() const
{
  BOOST_ASSERT(_delayOffFile.is_open());
  return _delayOff;
}

void Led::shot()
{
  BOOST_ASSERT(_shotFile.is_open());
  if (!_shotFile.is_open())
    return;
  _shotFile << "1" << std::flush;
}
}
