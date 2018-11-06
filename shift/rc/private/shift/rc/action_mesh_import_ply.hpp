#ifndef SHIFT_RC_ACTION_MESH_IMPORT_PLY_HPP
#define SHIFT_RC_ACTION_MESH_IMPORT_PLY_HPP

#include <shift/core/exception.hpp>
#include <shift/resource/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
class action_mesh_import_ply : public action_base,
                               public core::singleton<action_mesh_import_ply>
{
public:
  static constexpr const char* action_name = "mesh-import-ply";
  static constexpr const char* action_version = "1.0.0002";

public:
  /// Default constructor.
  action_mesh_import_ply();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
