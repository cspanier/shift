#ifndef SHIFT_RC_IMPORTER_MAT_FILTER_HPP
#define SHIFT_RC_IMPORTER_MAT_FILTER_HPP

#include <shift/core/exception.hpp>
#include <shift/parser/json/json.hpp>
#include <shift/resource_db/repository.hpp>
#include "shift/rc/types.hpp"

namespace shift::resource_db
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
    static int run(const std::filesystem::path& input_filename,
                   resource_db::archive& target,
                   const parser::json::object& options);
  };
}
}

#endif
