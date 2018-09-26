#ifndef SHIFT_VK2CPP_APPLICATION_H
#define SHIFT_VK2CPP_APPLICATION_H

#include <vector>
#include <set>
#include <unordered_set>
#include <unordered_map>
#include <future>
#include <boost/program_options.hpp>
#include <boost/filesystem/path.hpp>
#include <shift/core/bit_field.h>
#include <shift/application/launcher.h>
#include <shift/parser/xml/xml.h>

namespace shift::vk2cpp
{
struct entity_descriptor
{
  const parser::xml::node* node = nullptr;
};

enum class type_category
{
  base_type,    // basetype
  bit_mask,     // bitmask
  handle,       // handle
  function,     // funcpointer
  enum_type,    // enum
  struct_type,  // struct
  union_type,   // union
  include,      // include
  define,       // define
  external      // none
};

enum class member_type_flag
{
  is_const = 1 << 0,
  is_struct = 1 << 1,
  is_pointer = 1 << 2,
  is_pointer_to_pointer = 1 << 3,
  is_optional = 1 << 4,
  is_private = 1 << 5
};

using member_type_flags = core::bit_field<member_type_flag>;

struct type_descriptor;

struct platform_descriptor : public entity_descriptor
{
  std::string macro;
  std::string comment;
};

struct member_descriptor : public entity_descriptor
{
  member_type_flags flags = member_type_flags{0};
  type_descriptor* type = nullptr;
  std::string name;
  std::string new_name;
  std::string default_value;
  // Some struct members' array size is defined using an enum reference.
  const parser::xml::node* array_size_reference = nullptr;
  std::size_t array_size = 1;
  member_descriptor* length = nullptr;
  std::string comment;
  bool read_only = false;
};

struct enumerant_descriptor : public entity_descriptor
{
  std::string name;
  std::string new_name;
  std::string value;
  std::string comment;
};

struct type_descriptor;
struct command_descriptor;

struct feature_descriptor : public entity_descriptor
{
  std::string name;
  std::vector<type_descriptor*> types;
  std::vector<command_descriptor*> commands;
};

struct type_descriptor : public entity_descriptor
{
  std::string name;
  std::string new_name;
  type_category category;
  type_descriptor* require = nullptr;
  type_descriptor* alias = nullptr;
  std::vector<member_descriptor> members;
  std::vector<enumerant_descriptor> enumerants;
  bool is_fake_enum = false;
  bool has_read_only_members = false;
  std::unordered_set<feature_descriptor*> features;
  std::unordered_set<platform_descriptor*> platforms;
};

struct command_descriptor : public entity_descriptor
{
  std::string success_codes;
  std::string error_codes;
  std::string queues;
  std::string render_pass;
  std::string command_buffer_level;
  std::string pipeline;
  member_descriptor prototype;
  std::vector<member_descriptor> parameters;
  std::vector<std::string> validity;
  std::string comment;
};

struct extension_descriptor : public entity_descriptor
{
  std::string name;
  std::uint32_t number = 1;
  platform_descriptor* platform = nullptr;
  std::vector<type_descriptor*> types;
  std::vector<command_descriptor*> commands;
};

struct program_options
{
  static boost::filesystem::path input_path;
  static boost::filesystem::path output_filename;
  static boost::filesystem::path clang_format;
  static boost::filesystem::path snippets_vs_path;
  static boost::filesystem::path snippets_qt_path;
  static boost::filesystem::path graph_filename;
};

template <typename NextModule>
class launcher : public NextModule, public program_options
{
public:
  using base_t = NextModule;

  launcher(int argc, char* argv[]) : base_t(argc, argv)
  {
    namespace opt = boost::program_options;

    base_t::_visible_options.add_options()(
      "input,i",
      opt::value(&input_path)
        ->default_value("../private/vulkan/vk.xml")
        ->required(),
      "Path to the Vulkan XML specification to translate.");
    base_t::_visible_options.add_options()(
      "output,o",
      opt::value(&output_filename)
        ->default_value("../shift/render.vk/public/shift/render/vk/vulkan.h")
        ->required(),
      "Full path to a header file to write.");
    base_t::_visible_options.add_options()(
      "clang-format",
      opt::value(&clang_format)
#if defined(SHIFT_PLATFORM_WINDOWS)
        ->default_value("c:/bin/clang-format.exe"),
#elif defined(SHIFT_PLATFORM_LINUX)
        ->default_value("/usr/bin/clang-format"),
#else
#error You need to select your target platform.
#endif
      "Path to the clang-format tool, used to format the generated source "
      "code.");
    base_t::_visible_options.add_options()(
      "snippets-vs",
      opt::value(&snippets_vs_path)
        ->default_value("../private/vulkan/snippets-visual-studio/")
        ->required(),
      "Path to a folder where to write Visual Studio code snippets.");
    base_t::_visible_options.add_options()(
      "snippets-qt",
      opt::value(&snippets_qt_path)
        ->default_value("../private/vulkan/snippets-qt-creator/")
        ->required(),
      "Path to a folder where to write QTCreator code snippets.");
    base_t::_visible_options.add_options()(
      "graph,g",
      opt::value(&graph_filename)
        ->default_value("../private/vulkan/vulkan.dot")
        ->required(),
      "Full path to a graphviz source file to write.");
  }
};

class application : public program_options
{
public:
  /// Constructor.
  application();

  /// Main application routine.
  int run();

private:
  ///
  void parse_platforms(const parser::xml::node& platforms_node);

  ///
  void parse_type_names(const parser::xml::node& types_node);

  ///
  void parse_type_details(const parser::xml::node& types_node);

  ///
  void parse_member(member_descriptor& member,
                    const parser::xml::node& member_node);

  ///
  void parse_enums(const parser::xml::node& registry_node);

  ///
  void parse_commands(const parser::xml::node& commands_node);

  ///
  void parse_feature(const parser::xml::node& feature_node);

  ///
  void parse_extensions(const parser::xml::node& extensions_node);

  ///
  void fix_names();

  ///
  void add_empty_bit_field_enumerants();

  ///
  std::string cpp_default_value(const type_descriptor& parent_type,
                                const member_descriptor& member);

  ///
  void propagate_feature(feature_descriptor& feature);

  ///
  void propagate_feature(type_descriptor& type, type_descriptor& other_type);

  ///
  void propagate_platform(extension_descriptor& extension);

  ///
  void propagate_platform(type_descriptor& type, type_descriptor& other_type);

  ///
  void print_cpp(const type_descriptor& type);

  ///
  void print_cpp(const member_descriptor& member, std::string prefix);

  ///
  void print_cpp(const command_descriptor& command);

  ///
  void print_cpp(const extension_descriptor& extension);

  ///
  void print_snippets_vs();

  ///
  void print_snippets_qt();

  ///
  void print_graph(const std::string& command_name, const type_descriptor& type,
                   bool input);

  ///
  void print_graph(const command_descriptor& command);

  ///
  void print_graph(const extension_descriptor& extension);

  ///
  type_descriptor* type_by_name(const std::string& name);

  ///
  command_descriptor* command_by_name(const std::string& name);

  std::string _copyright;
  std::unordered_map<std::string, platform_descriptor> _platforms;
  std::vector<type_descriptor> _types;
  std::vector<command_descriptor> _commands;
  std::vector<feature_descriptor> _features;
  std::vector<extension_descriptor> _extensions;
  std::set<const entity_descriptor*> _printed;
  std::ofstream _file;
};
}

#endif
