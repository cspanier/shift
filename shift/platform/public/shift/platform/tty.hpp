#ifndef SHIFT_PLATFORM_TTY_HPP
#define SHIFT_PLATFORM_TTY_HPP

#include <memory>
#include <string>

namespace shift::platform
{
class tty
{
public:
  /// Constructor.
  tty(const std::string& device, unsigned int speed,
      bool useHardwareFlowControl = true, bool parityCheck = false,
      bool useTwoStopBits = false, unsigned int characterSizeInBits = 8);

  /// Destructor.
  ~tty();

  ///
  std::size_t read(char* buffer, std::size_t size, std::size_t offset = 0);

  ///
  std::size_t write(const char* buffer, std::size_t size,
                    std::size_t offset = 0);

private:
  class impl;
  std::unique_ptr<impl> _impl;
};
}

#endif
