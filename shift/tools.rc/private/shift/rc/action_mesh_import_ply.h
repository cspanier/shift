#ifndef SHIFT_RC_ACTION_MESH_IMPORT_PLY_H
#define SHIFT_RC_ACTION_MESH_IMPORT_PLY_H

#include <shift/core/exception.h>
#include <shift/resource/repository.h>
#include "shift/rc/types.h"
#include "shift/rc/action_base.h"

namespace shift::rc
{
class action_mesh_import_ply : public action_base,
                               public core::singleton<action_mesh_import_ply>
{
public:
  /// Default constructor.
  action_mesh_import_ply();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  static constexpr const char* action_name = "mesh-import-ply";
  static constexpr const char* action_version = "1.0.0002";
};
}

#endif
