#ifndef SHIFT_RC_IMPORTER_GEODATA_LARGE_IMAGES_HPP
#define SHIFT_RC_IMPORTER_GEODATA_LARGE_IMAGES_HPP

#include <shift/core/exception.hpp>
#include <shift/resource/repository.hpp>
#include "shift/rc/types.hpp"

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
