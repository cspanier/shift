#include "shift/rc/action_scene_compile.h"

namespace shift::rc
{
action_scene_compile::action_scene_compile()
: action_base(action_name, action_version)
{
}

bool action_scene_compile::process(resource_compiler_impl& /*compiler*/,
                                   job_description& /*job*/) const
{
  /// ToDo: implement...
  return true;
}
}
