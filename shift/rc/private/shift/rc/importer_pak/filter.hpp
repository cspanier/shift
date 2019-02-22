#ifndef SHIFT_RC_IMPORTER_PAK_FILTER_HPP
#define SHIFT_RC_IMPORTER_PAK_FILTER_HPP

#include <filesystem>
#include <shift/core/exception.hpp>
#include <shift/parser/json/json.hpp>
#include "shift/rc/types.hpp"

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
  static bool read(const std::filesystem::path& filename, file_type input_type);
};
}

#endif
