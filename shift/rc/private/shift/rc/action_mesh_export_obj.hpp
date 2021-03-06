#ifndef SHIFT_RC_ACTION_MESH_EXPORT_OBJ_HPP
#define SHIFT_RC_ACTION_MESH_EXPORT_OBJ_HPP

#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
class action_mesh_export_obj : public action_base,
                               public core::singleton<action_mesh_export_obj>
{
public:
  static constexpr const char* action_name = "mesh-export-obj";
  static constexpr const char* action_version = "1.0.0004";

public:
  /// Default constructor.
  action_mesh_export_obj();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
