#include "shift/protogen/program_options.hpp"

namespace shift::proto
{
boost::filesystem::path program_options::cache_filename;
std::vector<boost::filesystem::path> program_options::proto_sources;
bool program_options::verbose;
std::string program_options::strip_namescopes;
std::string program_options::limit_namescopes;
std::vector<std::string> program_options::exclude_namescopes;
std::vector<std::pair<std::string, std::string>> program_options::definitions;
bool program_options::force_rewrite;
boost::filesystem::path program_options::cpp_source_output_path;
boost::filesystem::path program_options::cpp_include_output_path;
std::string program_options::cpp_folder;
std::string program_options::cpp_namespace;
std::vector<std::string> program_options::cpp_includes;
std::string program_options::cpp_naming_convention_name;
std::uint32_t program_options::cpp_indent_width;
std::string program_options::cpp_clang_format;
boost::filesystem::path program_options::cs_output_path;
std::string program_options::cs_namespace;
std::vector<std::string> program_options::cs_usings;
std::uint32_t program_options::cs_indent_width;
boost::filesystem::path program_options::dot_output_path;
std::uint32_t program_options::dot_indent_width;
boost::filesystem::path program_options::proto_output_path;
std::uint32_t program_options::proto_indent_width;
}
