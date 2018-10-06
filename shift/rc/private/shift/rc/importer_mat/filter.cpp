#include "shift/rc/importer_mat/filter.h"
#include <shift/resource/image.h>
#include <shift/log/log.h>
#include <shift/math/utility.h>
#include <shift/math/vector.h>
#include <shift/core/mpl.h>
#include <shift/core/string_util.h>
#include <boost/filesystem.hpp>

namespace shift::resource
{
class archive;
}

namespace shift::rc
{
namespace mat
{
  namespace fs = boost::filesystem;

  int converter::run(const boost::filesystem::path& input_filename,
                     resource::archive& /*target*/,
                     const parser::json::object& /*options*/)
  {
    if (!fs::exists(input_filename) || !fs::is_regular_file(input_filename))
    {
      log::error() << "Cannot open input file " << input_filename << ".";
      return EXIT_FAILURE;
    }

    // resource::repository::singleton_instance().save(
    //  image->id(), *image, input_filename.generic_string(), target);
    log::info() << input_filename.generic_string();
    return EXIT_SUCCESS;
  }
}
}
