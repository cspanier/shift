#include "shift/platform/single_instance.hpp"
#include <sstream>
#include <array>
#include <Windows.h>

namespace shift::platform
{
struct single_instance::impl
{
  void* eventHandle = nullptr;
};

single_instance::single_instance() : _impl(std::make_unique<impl>())
{
  // NOP.
}

single_instance::~single_instance()
{
  if (_impl->eventHandle)
  {
    CloseHandle(_impl->eventHandle);
    _impl->eventHandle = nullptr;
  }
}

bool single_instance::check()
{
  std::array<char, MAX_PATH> filename;
  auto length = GetModuleFileName(nullptr, filename.data(), filename.size());
  auto begin = filename.begin();
  auto end = filename.begin() + length;
  for (auto i = filename.begin(); i != filename.end(); ++i)
  {
    if (*i == '\\' || *i == '/')
      begin = i + 1;
  }

  std::stringstream name;
  name << "Global\\Shift_" << std::string(begin, end);
  _impl->eventHandle = CreateEvent(nullptr, TRUE, FALSE, name.str().c_str());
  if (!_impl->eventHandle || GetLastError() == ERROR_ALREADY_EXISTS)
    return false;
  return true;
}
}
