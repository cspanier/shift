#include "shift/platform/console.h"
#include <algorithm>
#include <string>
#include <Windows.h>

namespace shift::platform
{
console console::_instance;

console& console::singleton_instance()
{
  return _instance;
}

bool console::exists() const
{
  return GetConsoleWindow() != FALSE;
}

void console::show()
{
  ShowWindow(GetConsoleWindow(), SW_SHOW);
}

void console::hide()
{
  ShowWindow(GetConsoleWindow(), SW_HIDE);
}

void console::title(std::string_view string)
{
  SetConsoleTitle(std::string(string.data(), string.size()).c_str());
}

console::console()
{
  if (!GetConsoleWindow())
  {
    if (!AllocConsole())
      return;
  }
}

console::~console()
{
}
}
