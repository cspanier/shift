#ifndef SHIFT_RC_ACTION_IMAGE_IMPORT_H
#define SHIFT_RC_ACTION_IMAGE_IMPORT_H

#include <shift/core/exception.h>
#include <shift/resource/repository.h>
#include "shift/rc/types.h"
#include "shift/rc/action_base.h"

namespace shift::rc
{
///
class action_image_import : public action_base,
                            public core::singleton<action_image_import>
{
public:
  /// Default constructor.
  action_image_import();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;

private:
  static constexpr const char* action_name = "image-import";
  static constexpr const char* action_version = "1.0.0001";
};
}

#endif
