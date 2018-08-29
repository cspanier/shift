#include "shift/platform/single_instance.h"

namespace shift::platform
{
struct single_instance::impl
{
};

single_instance::single_instance() : _impl(std::make_unique<impl>())
{
}

single_instance::~single_instance() = default;

bool single_instance::check()
{
  /// ToDo: ...
  return true;
}
}
