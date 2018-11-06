#ifndef SHIFT_PLATFORM_ADC_HPP
#define SHIFT_PLATFORM_ADC_HPP

#include <limits>
#include <string>
#include <functional>
#include <shift/core/types.hpp>

namespace shift::platform
{
/// An analog digital converter abstraction class.
class adc
{
public:
  /// Constructor taking a platform dependent identifier of the hardware ADC
  /// to use.
  adc(std::string source);

  adc(const adc&) = delete;

  /// Move constructor.
  adc(adc&& other) = default;

  /// Destructor.
  ~adc();

  adc& operator=(const adc&) = delete;

  /// Move assignment operator.
  adc& operator=(adc&& other) = default;

  /// Returns the source identifier of this GPIO.
  std::string_view source() const;

  /// Updates the GPIO value.
  void update();

  /// Returns the cached value last queried by calling update.
  int value() const;

  ///
  int min() const;

  ///
  void min(int value);

  ///
  int max() const;

  ///
  void max(int value);

  ///
  bool inverted() const;

  ///
  void inverted(bool value);

  ///
  float postScale() const;

  ///
  void postScale(float value);

  std::function<void(adc& sender, int value)> on_changed;

private:
  std::string _source;
  int _handle = -1;
  int _value = 0;
  int _min = std::numeric_limits<int>::max();
  int _max = std::numeric_limits<int>::min();
  bool _inverted = false;
  float _postScale = 1.0f;
};
}

#endif
