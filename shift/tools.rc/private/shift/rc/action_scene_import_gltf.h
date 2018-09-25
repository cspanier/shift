#ifndef SHIFT_RC_ACTION_SCENE_IMPORT_GLTF_H
#define SHIFT_RC_ACTION_SCENE_IMPORT_GLTF_H

#include <shift/resource/repository.h>
#include "shift/rc/types.h"
#include "shift/rc/action_base.h"

namespace shift::rc
{
struct gltf_error : virtual core::runtime_error
{
};

/// Reads and converts GLTF files.
class action_scene_import_gltf
: public action_base,
  public core::singleton<action_scene_import_gltf>
{
public:
  /// Default constructor.
  action_scene_import_gltf();

  /// Destructor.
  ~action_scene_import_gltf() override;

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  struct context_t;

  static constexpr const char* action_name = "scene-import-gltf";
  static constexpr const char* action_version = "1.0.0001";
  static constexpr const bool support_multithreading = true;
};
}

#endif
