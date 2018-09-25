#ifndef SHIFT_RC_ACTION_MESH_TOOTLE_H
#define SHIFT_RC_ACTION_MESH_TOOTLE_H

#include <shift/resource/repository.h>
#include "shift/rc/types.h"
#include "shift/rc/action_base.h"

namespace shift::rc
{
/// Make use of AMD Tootle library to perform optimizations on a mesh.
class action_mesh_tootle : public action_base,
                           public core::singleton<action_mesh_tootle>
{
public:
  /// Default constructor.
  action_mesh_tootle();

  /// Destructor.
  ~action_mesh_tootle() override;

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  static constexpr const char* action_name = "mesh-tootle";
  static constexpr const char* action_version = "1.0.0001";
  static constexpr const bool support_multithreading = false;
};
}

#endif
