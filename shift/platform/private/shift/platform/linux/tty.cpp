#include "shift/platform/tty.hpp"
#include <shift/platform/assert.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <termios.h>
#include <unistd.h>

namespace shift::platform
{
speed_t mapSpeed(unsigned int speed)
{
  switch (speed)
  {
  case 0:
    return static_cast<speed_t>(B0);
  case 50:
    return static_cast<speed_t>(B50);
  case 75:
    return static_cast<speed_t>(B75);
  case 110:
    return static_cast<speed_t>(B110);
  case 134:
    return static_cast<speed_t>(B134);
  case 150:
    return static_cast<speed_t>(B150);
  case 200:
    return static_cast<speed_t>(B200);
  case 300:
    return static_cast<speed_t>(B300);
  case 600:
    return static_cast<speed_t>(B600);
  case 1200:
    return static_cast<speed_t>(B1200);
  case 1800:
    return static_cast<speed_t>(B1800);
  case 2400:
    return static_cast<speed_t>(B2400);
  case 4800:
    return static_cast<speed_t>(B4800);
  case 9600:
    return static_cast<speed_t>(B9600);
  case 19200:
    return static_cast<speed_t>(B19200);
  case 38400:
    return static_cast<speed_t>(B38400);
  case 57600:
    return static_cast<speed_t>(B57600);
  case 115200:
    return static_cast<speed_t>(B115200);
  case 230400:
    return static_cast<speed_t>(B230400);
  default:
    BOOST_ASSERT(false);
    return static_cast<speed_t>(B0);
  }
}

tcflag_t mapCharacterSize(unsigned int characterSizeInBits)
{
  switch (characterSizeInBits)
  {
  case 5:
    return static_cast<tcflag_t>(CS5);
  case 6:
    return static_cast<tcflag_t>(CS6);
  case 7:
    return static_cast<tcflag_t>(CS7);
  case 8:
    return static_cast<tcflag_t>(CS8);
  default:
    BOOST_ASSERT(false);
    return static_cast<tcflag_t>(CS8);
  }
}

class tty::impl
{
public:
  int handle = -1;
  termios ttyNew = {};
  termios ttyOld = {};
};

tty::tty(const std::string& device, unsigned int speed,
         bool useHardwareFlowControl, bool parityCheck, bool useTwoStopBits,
         unsigned int characterSizeInBits)
: _impl(std::make_unique<impl>())
{
  _impl->handle = open(device.c_str(), O_RDWR | O_NOCTTY);
  if (_impl->handle == -1)
  {
    /// ToDo: Throw error.
    // std::cerr << "O" << strerror(errno) << std::endl;
    return;
  }

  if (tcgetattr(_impl->handle, &_impl->ttyOld) != 0)
  {
    /// ToDo: Throw error.
    // std::cerr << "T" << strerror(errno) << std::endl;
    return;
  }

  _impl->ttyNew = _impl->ttyOld;
  cfsetospeed(&_impl->ttyNew, mapSpeed(speed));
  cfsetispeed(&_impl->ttyNew, mapSpeed(speed));

  if (parityCheck)
    _impl->ttyNew.c_cflag |= PARENB;
  else
    _impl->ttyNew.c_cflag &= ~PARENB;

  if (useTwoStopBits)
    _impl->ttyNew.c_cflag |= CSTOPB;
  else
    _impl->ttyNew.c_cflag &= ~CSTOPB;

  _impl->ttyNew.c_cflag &= ~CSIZE;
  _impl->ttyNew.c_cflag |= mapCharacterSize(characterSizeInBits);

  if (useHardwareFlowControl)
    _impl->ttyNew.c_cflag |= CRTSCTS;
  else
    _impl->ttyNew.c_cflag &= ~CRTSCTS;

  // Expect at least one character.
  _impl->ttyNew.c_cc[VMIN] = 1;
  // Let read timeout after 0.1 seconds after the first character received.
  _impl->ttyNew.c_cc[VTIME] = 1;
  // Turn on read and ignore ctrl lines
  _impl->ttyNew.c_cflag |= CREAD | CLOCAL;

  cfmakeraw(&_impl->ttyNew);
  tcflush(_impl->handle, TCIFLUSH);
  if (tcsetattr(_impl->handle, TCSANOW, &_impl->ttyNew) != 0)
  {
    /// ToDo: Throw error.
    // std::cerr << "A" << strerror(errno) << std::endl;
    return;
  }
}

tty::~tty()
{
  if (_impl->handle != -1)
  {
    tcflush(_impl->handle, TCIFLUSH);
    tcsetattr(_impl->handle, TCSANOW, &_impl->ttyOld);
    close(_impl->handle);
    _impl->handle = -1;
  }
}

std::size_t tty::read(char* buffer, std::size_t size, std::size_t offset)
{
  int bytesRead = 0;
  auto bytesRemaining = size - offset;
  do
  {
    bytesRead = ::read(_impl->handle, &buffer[offset], bytesRemaining);
    if (bytesRead > 0)
    {
      offset += static_cast<std::size_t>(bytesRead);
      bytesRemaining -= static_cast<std::size_t>(bytesRead);
    }
  } while (bytesRead >= 0 && bytesRemaining > 0);
  /// ToDo: Throw error.
  // if (bytesRead == -1)
  //   std::cerr << "R" << strerror(errno) << std::endl;
  BOOST_ASSERT(bytesRemaining == 0);
  return bytesRemaining;
}

std::size_t tty::write(const char* buffer, std::size_t size, std::size_t offset)
{
  int bytesWritten = 0;
  auto bytesRemaining = size - offset;
  do
  {
    bytesWritten = ::write(_impl->handle, &buffer[offset], bytesRemaining);
    if (bytesWritten > 0)
    {
      offset += bytesWritten;
      bytesRemaining -= bytesWritten;
    }
  } while (bytesWritten >= 0 && bytesRemaining > 0);
  /// ToDo: Throw error.
  // if (bytesWritten == -1)
  //   std::cerr << "W" << strerror(errno) << std::endl;
  BOOST_ASSERT(bytesRemaining == 0);
  return bytesRemaining;
}
}
