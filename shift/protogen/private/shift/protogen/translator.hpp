#ifndef SHIFT_PROTO_GEN_TRANSLATOR_HPP
#define SHIFT_PROTO_GEN_TRANSLATOR_HPP

#include <memory>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/program_options.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/application/launcher.hpp>
#include "shift/protogen/file_cache.hpp"
#include "shift/protogen/generator/base_generator.hpp"

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
