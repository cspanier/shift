#ifndef SHIFT_TOOLS_PROTOGEN_TRANSLATOR_H
#define SHIFT_TOOLS_PROTOGEN_TRANSLATOR_H

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/application/launcher.h>
#include "shift/tools/protogen/file_cache.h"
#include "shift/tools/protogen/generators/base_generator.h"

namespace shift::tools::protogen
{
///
class translator
{
public:
  /// Constructor.
  translator();

  /// Main application routine.
  int run();

public:
  static boost::filesystem::path cache_filename;
  static std::vector<boost::filesystem::path> proto_sources;
  static bool verbose;
  static std::string strip_namescopes;
  static std::string limit_namescopes;
  static std::vector<std::string> exclude_namescopes;
  static std::vector<std::pair<std::string, std::string>> definitions;
  static bool force_rewrite;

  static boost::filesystem::path cpp_source_output_path;
  static boost::filesystem::path cpp_include_output_path;
  static std::string cpp_folder;
  static std::string cpp_namespace;
  static std::vector<std::string> cpp_includes;
  static std::string cpp_naming_convention_name;
  static std::string cpp_clang_format;

  static boost::filesystem::path cs_output_path;
  static std::string cs_namespace;
  static std::vector<std::string> cs_usings;

  static boost::filesystem::path dot_output_path;

  static boost::filesystem::path proto_output_path;

protected:
  file_cache _file_cache;
  std::unordered_map<std::string, std::unique_ptr<generators::base_generator>>
    _generators;
};
}

#endif
