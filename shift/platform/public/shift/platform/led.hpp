#ifndef SHIFT_PLATFORM_LED_HPP
#define SHIFT_PLATFORM_LED_HPP

#include <chrono>
#include <string>
#include <fstream>

namespace shift::platform
{
///
class Led
{
public:
  ///
  enum class Trigger
  {
    none,
    timer,
    oneShot
  };

public:
  ///
  Led(std::string name);

  ///
  void trigger(Trigger value);

  ///
  Trigger trigger() const;

  ///
  void brightness(std::uint8_t value);

  ///
  std::uint8_t brightness() const;

  ///
  void delayOn(std::chrono::milliseconds value);

  ///
  std::chrono::milliseconds delayOn() const;

  ///
  void delayOff(std::chrono::milliseconds value);

  ///
  std::chrono::milliseconds delayOff() const;

  ///
  void shot();

private:
  std::string _name;
  std::ofstream _triggerFile;
  Trigger _trigger = Trigger::none;

  std::ofstream _brightnessFile;
  std::uint8_t _brightness;

  std::ofstream _delayOnFile;
  std::chrono::milliseconds _delayOn;

  std::ofstream _delayOffFile;
  std::chrono::milliseconds _delayOff;

  std::ofstream _shotFile;
};
}

#endif
