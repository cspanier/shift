#ifndef SHIFT_RC_ACTION_MESH_TOOTLE_HPP
#define SHIFT_RC_ACTION_MESH_TOOTLE_HPP

#include <shift/resource/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
/// Make use of AMD Tootle library to perform optimizations on a mesh.
class action_mesh_tootle : public action_base,
                           public core::singleton<action_mesh_tootle>
{
public:
  static constexpr const char* action_name = "mesh-tootle";
  static constexpr const char* action_version = "1.0.0001";
  static constexpr const bool support_multithreading = false;

public:
  /// Default constructor.
  action_mesh_tootle();

  /// Destructor.
  ~action_mesh_tootle() override;

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
