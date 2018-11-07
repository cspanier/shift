#ifndef SHIFT_PROTO_GEN_PROGRAMOPTIONS_HPP
#define SHIFT_PROTO_GEN_PROGRAMOPTIONS_HPP

#include <string>
#include <vector>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::proto
{
struct program_options
{
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
  static std::uint32_t cpp_indent_width;
  static std::string cpp_clang_format;

  static boost::filesystem::path cs_output_path;
  static std::string cs_namespace;
  static std::vector<std::string> cs_usings;
  static std::uint32_t cs_indent_width;

  static boost::filesystem::path dot_output_path;
  static std::uint32_t dot_indent_width;

  static boost::filesystem::path proto_output_path;
  static std::uint32_t proto_indent_width;
};
}

#endif
