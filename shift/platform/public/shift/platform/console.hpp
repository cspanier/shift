#ifndef SHIFT_PLATFORM_CONSOLE_HPP
#define SHIFT_PLATFORM_CONSOLE_HPP

#include <string>

namespace shift::platform
{
///
class console
{
public:
  /// Returns a reference to the singleton object.
  static console& singleton_instance();

  /// Returns whether there is a console attached to the current process.
  bool exists() const;

  /// Shows the console window.
  void show();

  /// Hides the console window.
  void hide();

  /// Updates the console title.
  void title(std::string_view string);

private:
  /// Default constructor for the singleton class. Note that only one
  /// instance may be created at any time.
  console();

  /// Destructor.
  ~console();

  static console _instance;
};
}

#endif
