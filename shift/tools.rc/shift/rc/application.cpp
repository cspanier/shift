#include "shift/rc/application.h"
#include "shift/resource/repository.h"
#include <shift/log/log.h>
#include <shift/core/exception.h>
#include <shift/core/mpl.h>
#include <shift/core/string_util.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <memory>

namespace shift
{
namespace rc
{
  boost::filesystem::path program_options::input_path;
  boost::filesystem::path program_options::build_path;
  boost::filesystem::path program_options::output_path;
  std::string program_options::rules_filename;
  std::string program_options::cache_filename;
  std::uint32_t program_options::verbose;
  std::string program_options::image_magick;

  application::application() = default;

  int application::run()
  {
    namespace fs = boost::filesystem;
    using std::begin;
    using std::end;

    resource::repository repository;
    repository.mount(build_path, false);
    repository.mount(output_path, false);

    _compiler.input_path(input_path);
    _compiler.build_path(build_path);
    _compiler.output_path(output_path);
    _compiler.verbose(verbose);
    _compiler.image_magick(image_magick);
    auto cache_filepath = build_path / cache_filename;
    if (fs::exists(cache_filepath) && !_compiler.load_cache(cache_filepath))
      log::warning() << "Cannot read cache file " << cache_filepath;
    _compiler.load_rules(rules_filename);
    _compiler.update();

    _compiler.collect_garbage();
    _compiler.save_cache(cache_filepath);

    return EXIT_SUCCESS;
  }
}
}
