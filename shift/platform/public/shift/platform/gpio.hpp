#ifndef SHIFT_PLATFORM_GPIO_HPP
#define SHIFT_PLATFORM_GPIO_HPP

#include <fstream>
#include <functional>
#include <chrono>
#include <vector>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/logic/tribool.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/types.hpp>
#include <shift/core/singleton.hpp>

namespace shift::platform
{
///
class gpio
{
public:
  ///
  enum class io_direction
  {
    in,
    out
  };

  ///
  enum class edge_trigger
  {
    none,
    rising,
    falling,
    both
  };

  ///
  enum class watch_event
  {
    error,
    timeout,
    changed
  };

public:
  /// Constructor taking a platform dependent identifier of the hardware GPIO
  /// channel to use.
  /// @remarks
  ///   The Linux implementation expects a sysfs path to the GPIO interface
  ///   (e.g. "/sys/class/gpio/gpio<NUM>").
  gpio(std::string source);

  gpio(const gpio&) = delete;

  /// Move constructor.
  gpio(gpio&& other) = default;

  /// Destructor.
  ~gpio();

  gpio& operator=(const gpio&) = delete;

  /// Move assignment operator.
  gpio& operator=(gpio&& other) = default;

  /// Returns the source of this GPIO.
  std::string_view source() const;

  /// Opens the GPIO data connection.
  bool open();

  /// Closes the GPIO data connection.
  void close();

  /// Sets whether this GPIO is used for input or output.
  /// @remarks
  ///   This may not be changed while the data connection is open.
  void direction(io_direction value);

  /// Returns whether this GPIO is used for input or output.
  io_direction direction() const;

  /// Sets whether IRQ based edge detection should be enabled.
  /// @remarks
  ///   This may not be changed while the data connection is open.
  void edge(edge_trigger value);

  /// Returns the edge detection state.
  edge_trigger edge() const;

  /// Sets whether the GPIO value should be inverted in software.
  /// @remarks
  ///   This may not be changed while the data connection is open.
  void inverted(bool value);

  /// Returns whether the GPIO value gets inverted in software.
  bool inverted() const;

  /// Returns the internal file handle used for the value file.
  int handle() const;

  /// Updates the GPIO value.
  void update(bool forceUpdate = false);

  /// Returns the cached value last queried by calling update.
  boost::tribool value() const;

  /// Writes a new value to the GPIO.
  /// @remarks
  ///   This call is only valid when direction is set to output.
  void value(boost::tribool state);

  /// Watches a list of GPIOs for changes.
  /// @remarks
  ///   The call blocks until at least one edge in the GPIO signals matches
  ///   its GPIO edge setting, in which case the callback gets invoked for
  ///   each matching GPIO, or until timeout milliseconds have passed.
  static watch_event watch(const std::vector<std::shared_ptr<gpio>>& inputs,
                           std::chrono::milliseconds timeout);

  std::function<void(gpio& input, bool value)> on_changed;

private:
  std::string _source;

  io_direction _direction;
  std::ofstream _direction_file;

  edge_trigger _edge;
  std::ofstream _edge_file;

  bool _inverted = false;
  int _value_handle = -1;
  boost::tribool _value = boost::indeterminate;
};
}

#endif
