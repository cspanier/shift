#include "shift/platform/fpexceptions.hpp"

namespace shift::platform
{
floating_point_exceptions::floating_point_exceptions(unsigned int exceptions,
                                                     unsigned int mask)
{
  _controlfp_s(&_old_control_word, 0, 0);
  _clearfp();
  _controlfp_s(nullptr, ~exceptions & _MCW_EM, mask);
}

floating_point_exceptions::~floating_point_exceptions()
{
  _clearfp();
  _controlfp_s(nullptr, _old_control_word, _MCW_EM);
}
}
