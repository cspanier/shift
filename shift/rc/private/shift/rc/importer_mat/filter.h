#ifndef SHIFT_RC_IMPORTER_MAT_FILTER_H
#define SHIFT_RC_IMPORTER_MAT_FILTER_H

#include <shift/core/exception.h>
#include <shift/parser/json/json.h>
#include <shift/resource/repository.h>
#include "shift/rc/types.h"

namespace shift::resource
{
class archive;
}

namespace shift::rc
{
namespace mat
{
  ///
  struct converter
  {
    /// Main converter routine.
    static int run(const boost::filesystem::path& input_filename,
                   resource::archive& target,
                   const parser::json::object& options);
  };
}
}

#endif
