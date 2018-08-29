#include <string>
#include <iostream>
#include "shift/platform/console.h"

namespace shift::platform
{
console console::_instance;

console& console::singleton_instance()
{
  return _instance;
}

bool console::exists() const
{
  return true;
}

void console::show()
{
}

void console::hide()
{
}

void console::title(std::string_view /*string*/)
{
  /// ToDo: ...
}

console::console() = default;

console::~console() = default;
}
