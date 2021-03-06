#ifndef SHIFT_RC_ACTION_SCENE_COMPILE_HPP
#define SHIFT_RC_ACTION_SCENE_COMPILE_HPP

#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
/// This action converts JSON-based scene files to binary format.
class action_scene_compile : public action_base,
                             public core::singleton<action_scene_compile>
{
public:
  static constexpr const char* action_name = "scene-compile";
  static constexpr const char* action_version = "1.0.0001";

public:
  /// Default constructor.
  action_scene_compile();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
