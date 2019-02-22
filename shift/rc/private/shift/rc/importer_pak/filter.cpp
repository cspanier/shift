#include "shift/rc/importer_pak/filter.hpp"
#include <shift/resource_db/repository.hpp>
#include <shift/log/log.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <filesystem>

namespace shift::rc::pak
{
namespace fs = std::filesystem;

int converter::run()
{
  // if (!fs::exists(input_filename) || !fs::is_regular_file(input_filename))
  //{
  //  log::error() << "Cannot open input file " << input_filename << ".";
  //  return EXIT_FAILURE;
  //}

  // resource_db::repository repository;
  // auto input_archive =
  //  repository.mount(input_filename.generic_string(), true);
  // auto output_archive =
  //  repository.mount(output_filename.generic_string(), false);

  return EXIT_SUCCESS;
}
}
