#include "shift/platform/environment.h"
#include <vector>
#include <csignal>
#include <fcntl.h>

namespace shift::platform
{
std::string environment::hostname()
{
  std::vector<char> buffer;
  unsigned long size = 256;  /// ToDo: Why is HOST_NAME_MAX not defined?
  buffer.resize(size, 0);
  if (gethostname(buffer.data(), size) == 0)
    return std::string(buffer.data());
  else
    return UNKNOWN_HOSTNAME;
}

std::string environment::username()
{
  std::vector<char> buffer;
  unsigned long size = L_cuserid + 1;
  buffer.resize(size, 0);
  if (getlogin_r(buffer.data(), size))
    return std::string(buffer.data());
  else
    return UNKNOWN_USERNAME;
}

boost::filesystem::path environment::executable_path()
{
  std::vector<char> buffer;
  buffer.resize(256, 0);
  readlink("/proc/self/exe", buffer.data(), buffer.size());
  while (errno == ENAMETOOLONG)
  {
    buffer.resize(buffer.size() * 2, 0);
    readlink("/proc/self/exe", buffer.data(), buffer.size());
  }
  return boost::filesystem::system_complete(
    boost::filesystem::path(buffer.data()));
}

void environment::thread_debug_name(const std::string& /*name*/)
{
  // NOP.
}

bool environment::is_debugger_present()
{
  int status_fd = open("/proc/self/status", O_RDONLY);
  if (status_fd == -1)
    return false;

  std::array<char, 1024> buffer;
  ssize_t bytes_read = read(status_fd, buffer.data(), buffer.size() - 1);

  if (bytes_read <= 0)
    return false;

  static const char tracer_pid_string[] = "TracerPid:";

  buffer[static_cast<std::size_t>(bytes_read)] = 0;
  if (char* tracer_pid = std::strstr(buffer.data(), tracer_pid_string))
  {
    char* begin = tracer_pid + sizeof(tracer_pid_string) - 1;
    char* end = tracer_pid;
    return (std::strtol(begin, &end, 10) != 0) && (begin != end);
  }
  return false;
}

void environment::debug_break()
{
  // To continue from here in GDB use "signal 0".
  // std::raise(SIGABRT);

  /// ToDo: Test alternative:
  std::raise(SIGINT);
}
}
