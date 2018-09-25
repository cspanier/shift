#ifndef SHIFT_RC_IMPORTER_PAK_FILTER_H
#define SHIFT_RC_IMPORTER_PAK_FILTER_H

#include <boost/filesystem/path.hpp>
#include <shift/core/exception.h>
#include <shift/parser/json/json.h>
#include "shift/rc/types.h"

namespace shift::rc::pak
{
///
enum class file_type
{
  png,
  jpeg,
  tiff
};

///
struct converter
{
  /// Main converter routine.
  static int run();

private:
  template <typename Image>
  static bool read(const boost::filesystem::path& filename,
                   file_type input_type);
};
}

#endif
