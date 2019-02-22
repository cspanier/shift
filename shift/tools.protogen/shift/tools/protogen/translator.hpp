#ifndef SHIFT_TOOLS_PROTOGEN_TRANSLATOR_HPP
#define SHIFT_TOOLS_PROTOGEN_TRANSLATOR_HPP

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <filesystem>
#include <shift/application/launcher.hpp>
#include "shift/tools/protogen/file_cache.hpp"
#include "shift/tools/protogen/generators/base_generator.hpp"

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
  static std::filesystem::path cache_filename;
  static std::vector<std::filesystem::path> proto_sources;
  static bool verbose;
  static std::string strip_namescopes;
  static std::string limit_namescopes;
  static std::vector<std::string> exclude_namescopes;
  static std::vector<std::pair<std::string, std::string>> definitions;
  static bool force_rewrite;

  static std::filesystem::path cpp_source_output_path;
  static std::filesystem::path cpp_include_output_path;
  static std::string cpp_folder;
  static std::string cpp_namespace;
  static std::vector<std::string> cpp_includes;
  static std::string cpp_naming_convention_name;
  static std::uint32_t cpp_indent_width;
  static std::string cpp_clang_format;

  static std::filesystem::path cs_output_path;
  static std::string cs_namespace;
  static std::vector<std::string> cs_usings;
  static std::uint32_t cs_indent_width;

  static std::filesystem::path dot_output_path;
  static std::uint32_t dot_indent_width;

  static std::filesystem::path proto_output_path;
  static std::uint32_t proto_indent_width;

protected:
  file_cache _file_cache;
  std::unordered_map<std::string, std::unique_ptr<generators::base_generator>>
    _generators;
};
}

#endif
