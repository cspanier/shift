#ifndef SHIFT_RC_LAUNCHER_H
#define SHIFT_RC_LAUNCHER_H

#include <boost/program_options.hpp>
#include "shift/rc/program_options.h"

namespace shift
{
namespace rc
{
  template <typename NextModule>
  class launcher : public NextModule, public program_options
  {
  public:
    using base_t = NextModule;

    launcher(int argc, char* argv[]) : base_t(argc, argv)
    {
      namespace opt = boost::program_options;

      base_t::_visible_options.add_options()(
        "input,i", opt::value(&input_path)->default_value("resources"),
        "Base path to all source files to run through the resource compiler.");

      base_t::_visible_options.add_options()(
        "build,b", opt::value(&build_path)->default_value("build-rc"),
        "Base path to write temporary resource files to.");

      base_t::_visible_options.add_options()(
        "output,o", opt::value(&output_path)->default_value("production"),
        "Base path to write compiled files to.");

      base_t::_visible_options.add_options()(
        "rules,r", opt::value(&rules_filename)->default_value(".rc-rules.json"),
        "Name of rules json files to search for.");

      base_t::_visible_options.add_options()(
        "cache,c", opt::value(&cache_filename)->default_value(".rc-cache.json"),
        "Name of a cache json file used to store private data which is used to "
        "improve performance of subsequent rc invocations.");

      base_t::_visible_options.add_options()(
        "verbose,v", opt::value(&verbose)->default_value(0)->implicit_value(1),
        "Print more information.");

      base_t::_visible_options.add_options()(
        "image-magick", opt::value(&image_magick)->default_value("magick"),
        "Image Magick's command line executable.");
    }
  };
}
}

#endif
