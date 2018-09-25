#ifndef SHIFT_RC_IMPORTER_GEODATA_LARGE_IMAGES_H
#define SHIFT_RC_IMPORTER_GEODATA_LARGE_IMAGES_H

#include <shift/core/exception.h>
#include <shift/resource/repository.h>
#include "shift/rc/types.h"

namespace shift::rc
{
class large_image_importer
{
public:
  static constexpr const char* name = "import-geodata-large-image";

  static constexpr const char* version = "1.0.0001";

  static bool process(resource_compiler_impl& compiler, job_description& job);
};
}

#endif
