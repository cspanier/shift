#ifndef SHIFT_RC_ACTION_IMAGE_EXPORT_TIFF_HPP
#define SHIFT_RC_ACTION_IMAGE_EXPORT_TIFF_HPP

#include <shift/core/exception.hpp>
#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::rc
{
///
class action_image_export_tiff
: public action_base,
  public core::singleton<action_image_export_tiff>
{
public:
  static constexpr const char* action_name = "image-export-tiff";
  static constexpr const char* action_version = "1.0.0001";

public:
  /// Default constructor.
  action_image_export_tiff();

  /// @see action_base::process.
  bool process(resource_compiler_impl& compiler,
               job_description& job) const override;
};
}

#endif
