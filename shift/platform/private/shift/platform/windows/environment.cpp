#include "shift/platform/environment.hpp"
#include "shift/core/string_util.hpp"
#include <vector>
#include <string>
#include <utility>
#include <cstdint>
#include <Lmcons.h>
#include <basetsd.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <debugapi.h>
#include <errhandlingapi.h>
#include <excpt.h>
#include <minwindef.h>
#include <winnt.h>

namespace shift::platform
{
std::string environment::hostname()
{
  std::string buffer;
  unsigned long size = MAX_COMPUTERNAME_LENGTH;
  buffer.resize(size);
  if (GetComputerNameA(buffer.data(), &size) && size)
  {
    buffer.resize(size);
    return core::to_lower(buffer);
  }
  else
    return UNKNOWN_HOSTNAME;
}

std::string environment::username()
{
  std::string buffer;
  unsigned long size = UNLEN;
  buffer.resize(size);
  if (GetUserNameA(buffer.data(), &size) && size)
  {
    buffer.resize(size);
    return core::to_lower(buffer.data());
  }
  else
    return UNKNOWN_USERNAME;
}

std::filesystem::path environment::executable_path()
{
  std::vector<char> buffer;
  buffer.resize(MAX_PATH, 0);
  GetModuleFileName(nullptr, buffer.data(),
                    static_cast<std::uint32_t>(buffer.size()));
  while (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    buffer.resize(buffer.size() * 2, 0);
    GetModuleFileName(nullptr, buffer.data(),
                      static_cast<std::uint32_t>(buffer.size()));
  }
  return std::filesystem::system_complete(
    std::filesystem::path(buffer.data()));
}

#pragma pack(push, 8)
struct THREADNAME_INFO
{
  // Must be 0x1000.
  DWORD type;
  // Pointer to name (in user addr space).
  LPCSTR name;
  // Thread ID (std::numeric_limits<DWORD>::max() = caller thread).
  DWORD threadID;
  // Reserved for future use, must be zero.
  DWORD flags;
};
#pragma pack(pop)

// See http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx for reference.
void environment::thread_debug_name(const std::string& name)
{
#if defined(_MSC_VER)
  THREADNAME_INFO thread_name;
  thread_name.type = 0x1000;
  thread_name.name = name.c_str();
  thread_name.threadID = std::numeric_limits<DWORD>::max();
  thread_name.flags = 0;

  constexpr DWORD EXCEPTION_MSVC_DEBUG_THREAD_NAME = 0x406D1388;

  if (is_debugger_present())
  {
    RaiseException(EXCEPTION_MSVC_DEBUG_THREAD_NAME, 0,
                   sizeof(thread_name) / sizeof(DWORD),
                   reinterpret_cast<ULONG_PTR*>(&thread_name));
  }
#else
  BOOST_ASSERT(false);
#endif
}

bool environment::is_debugger_present()
{
  return IsDebuggerPresent() != 0;
}

void environment::debug_break()
{
  /// ToDo: Replace this with "_asm { int 3 }" for 32bit;
  DebugBreak();
}
}
