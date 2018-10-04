#include "shift/rc/importer_pak/filter.h"
#include <shift/resource/repository.h>
#include <shift/log/log.h>
#include <shift/core/mpl.h>
#include <shift/core/string_util.h>
#include <boost/filesystem.hpp>

namespace shift::rc::pak
{
namespace fs = boost::filesystem;

int converter::run()
{
  // if (!fs::exists(input_filename) || !fs::is_regular_file(input_filename))
  //{
  //  log::error() << "Cannot open input file " << input_filename << ".";
  //  return EXIT_FAILURE;
  //}

  // resource::repository repository;
  // auto input_archive =
  //  repository.mount(input_filename.generic_string(), true);
  // auto output_archive =
  //  repository.mount(output_filename.generic_string(), false);

  return EXIT_SUCCESS;
}
}
