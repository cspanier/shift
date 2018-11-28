#include "shift/rc/importer_mat/filter.hpp"
#include <shift/resource_db/image.hpp>
#include <shift/log/log.hpp>
#include <shift/math/utility.hpp>
#include <shift/math/vector.hpp>
#include <shift/core/mpl.hpp>
#include <shift/core/string_util.hpp>
#include <boost/filesystem.hpp>

namespace shift::resource_db
{
class archive;
}

namespace shift::rc
{
namespace mat
{
  namespace fs = boost::filesystem;

  int converter::run(const boost::filesystem::path& input_filename,
                     resource_db::archive& /*target*/,
                     const parser::json::object& /*options*/)
  {
    if (!fs::exists(input_filename) || !fs::is_regular_file(input_filename))
    {
      log::error() << "Cannot open input file " << input_filename << ".";
      return EXIT_FAILURE;
    }

    // resource_db::repository::singleton_instance().save(
    //  image->id(), *image, input_filename.generic_string(), target);
    log::info() << input_filename.generic_string();
    return EXIT_SUCCESS;
  }
}
}
