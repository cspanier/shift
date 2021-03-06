#ifndef SHIFT_PROTO_GEN_PROGRAMOPTIONS_HPP
#define SHIFT_PROTO_GEN_PROGRAMOPTIONS_HPP

#include <string>
#include <vector>
#include <filesystem>

namespace shift::proto
{
struct program_options
{
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
};
}

#endif
