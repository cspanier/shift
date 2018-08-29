#ifndef SHIFT_PROTO_GEN_TRANSLATOR_H
#define SHIFT_PROTO_GEN_TRANSLATOR_H

#include <memory>
#include <unordered_map>
#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/application/launcher.h>
#include "shift/protogen/file_cache.h"
#include "shift/protogen/generator/base_generator.h"

namespace shift::proto
{
///
class translator : public program_options
{
public:
  /// Constructor.
  translator();

  /// Main application routine.
  int run();

protected:
  file_cache _file_cache;
  std::unordered_map<std::string, std::unique_ptr<generator::base_generator>>
    _generators;
};
}

#endif
