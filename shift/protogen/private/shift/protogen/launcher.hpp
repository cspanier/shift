#ifndef SHIFT_PROTO_GEN_LAUNCHER_HPP
#define SHIFT_PROTO_GEN_LAUNCHER_HPP

#include "shift/protogen/program_options.hpp"

namespace boost
{
void validate(boost::any& v, const std::vector<std::string>& values,
              std::pair<std::string, std::string>* /*target_type*/,
              int /*unused*/)
{
  using namespace boost::program_options;
  const auto& value = values[0];
  auto equal = value.find("=");
  if (equal == std::string::npos || equal == 0)
    throw validation_error(validation_error::invalid_option_value);
  v = std::make_pair(value.substr(0, equal), value.substr(equal + 1));
}
}

namespace shift::proto
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
      "cache-path", opt::value(&cache_filename)->required(),
      "Path to store temporary data.");
    base_t::_visible_options.add_options()(
      "source", opt::value(&proto_sources),
      "Path to a protocol definition file.");
    base_t::_visible_options.add_options()(
      "verbose", opt::bool_switch(&verbose)->default_value(false),
      "Enable additional output messages.");
    base_t::_visible_options.add_options()(
      "strip", opt::value(&strip_namescopes),
      "Strip the specific name-scopes in generated code.");
    base_t::_visible_options.add_options()(
      "limit", opt::value(&limit_namescopes),
      "Limits code generation to specific name-scopes, relative to those "
      "specified using --strip.");
    base_t::_visible_options.add_options()(
      "exclude", opt::value(&exclude_namescopes)->composing(),
      "Excludes certain name-scopes from code generation.");
    base_t::_visible_options.add_options()(
      "define,D", opt::value(&definitions)->composing(),
      "Preprocessor definitions.");
    base_t::_visible_options.add_options()(
      "force,f", opt::value(&force_rewrite)->default_value(false),
      "Force code generation.");
    base_t::_positional_options.add("source", -1);

    base_t::_visible_options.add_options()(
      "cpp-source-path", opt::value(&cpp_source_output_path),
      "Base path where the C++ generator shall write all source files to.");
    base_t::_visible_options.add_options()(
      "cpp-include-path", opt::value(&cpp_include_output_path),
      "Base path where the C++ generator shall write all include files to.");
    base_t::_visible_options.add_options()(
      "cpp-folder", opt::value(&cpp_folder),
      "Sets the folder where to generate to source code.");
    base_t::_visible_options.add_options()(
      "cpp-namespace", opt::value(&cpp_namespace), "Set the target namespace.");
    base_t::_visible_options.add_options()(
      "cpp-include", opt::value(&cpp_includes)->composing(),
      "Add include directives to each generated source file.");
    base_t::_visible_options.add_options()(
      "cpp-naming-convention",
      opt::value(&cpp_naming_convention_name)->composing(),
      "Select naming convention of generated symbols. Choose between "
      "'lower-delimited' (default) and 'camel-case'.");
    base_t::_visible_options.add_options()(
      "cpp-clang-format", opt::value(&cpp_clang_format),
      "Path to the clang-format tool, used to automatically format the "
      "generated source code.");

    base_t::_visible_options.add_options()(
      "c#-path", opt::value(&cs_output_path),
      "Base path where the C# generator shall write to.");
    base_t::_visible_options.add_options()(
      "c#-namespace", opt::value(&cs_namespace), "Set the target namespace");
    base_t::_visible_options.add_options()(
      "c#-using", opt::value(&cs_usings)->composing(),
      "Add using directives to each generated source file.");

    base_t::_visible_options.add_options()(
      "dot-path", opt::value(&dot_output_path),
      "Base path where the GraphViz generator shall write to.");

    base_t::_visible_options.add_options()(
      "proto-path", opt::value(&proto_output_path),
      "Base path where the Proto generator shall write to.");
  }
};
}

#endif
