#ifndef SHIFT_RC_IMPORTER_MAT_FILTER_HPP
#define SHIFT_RC_IMPORTER_MAT_FILTER_HPP

#include <shift/core/exception.hpp>
#include <shift/parser/json/json.hpp>
#include <shift/resource/repository.hpp>
#include "shift/rc/types.hpp"

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
