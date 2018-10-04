#ifndef SHIFT_RC_ACTION_GROUP_RESOURCES_H
#define SHIFT_RC_ACTION_GROUP_RESOURCES_H

#include <shift/resource/repository.h>
#include "shift/rc/types.h"
#include "shift/rc/action_base.h"

namespace shift::rc
{
///
class action_group_resources : public action_base,
                               public core::singleton<action_group_resources>
{
public:
  /// Default constructor.
  action_group_resources();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  static constexpr const char* action_name = "group-resources";
  static constexpr const char* action_version = "1.0.0001";
};
}

#endif