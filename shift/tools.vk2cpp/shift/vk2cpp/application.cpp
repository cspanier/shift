#include "shift/vk2cpp/application.h"
#include <shift/log/log.h>
#include <shift/core/exception.h>
#include <shift/core/stream_util.h>
#include <boost/assert.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/filesystem.hpp>
#include <boost/variant.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <queue>
#include <future>
#include <regex>

#if defined(PLATFORM_WINDOWS)
#define br "\r\n"
#else
#define br "\n"
#endif
#define br2 br br

#if defined(_MSC_VER) && (_MSC_VER <= 1900)
#define VS_CONSTEXPR ""
#else
#define VS_CONSTEXPR "constexpr "
#endif

namespace shift::vk2cpp
{
namespace fs = boost::filesystem;
namespace xml = parser::xml;

using core::dec_indent;
using core::inc_indent;
using core::indent;
using core::indent_width;

static std::string strip_comment(const std::string& comment)
{
  std::stringstream result;
  std::size_t begin = 0;
  std::size_t end;
  do
  {
    end = comment.find_first_of('\n', begin);
    if (begin == 0)
    {
      if (end == std::string::npos)
        return boost::trim_copy(comment);
    }
    else
      result << " ";
    result << boost::trim_copy(comment.substr(begin, end));
    begin = end + 1;
  } while (end != std::string::npos);
  return result.str();
}

static std::string strip_vk(const std::string name)
{
  auto prefix = name.substr(0, 2);
  if (prefix == "Vk" || prefix == "vk")
    return name.substr(2);
  prefix = name.substr(0, 3);
  if (prefix == "VK_")
    return name.substr(3);
  return name;
}

static std::string strip_hungarian(const std::string& name)
{
  return std::regex_replace(name, std::regex("^(sz|pp|s|p)(?=[[:upper:]])"),
                            "");
}

static std::string to_lower_case(std::string name)
{
  name = std::regex_replace(name, std::regex("([1234])D"), "$1d_");
  name = std::regex_replace(name, std::regex("([RGBAXDSE])([0-9])"), "$1$$$2");
  boost::replace_all(name, "__", "_");

  std::stringstream new_name;
  bool last_digit = true;
  bool last_upper = true;
  bool last_underscore = true;
  auto is_first = [&]() -> bool { return last_upper && last_underscore; };
  for (auto& c : name)
  {
    if (isupper(c) != 0)
    {
      if (!last_upper && !last_underscore)
        new_name << '_';
      new_name << static_cast<char>(tolower(c));
      last_upper = true;
      last_digit = false;
      last_underscore = false;
    }
    else if (isdigit(c) != 0)
    {
      if (!last_digit && !last_underscore)
        new_name << '_';
      new_name << c;
      last_upper = false;
      last_digit = true;
      last_underscore = false;
    }
    else
    {
      if ((c != '_') || !is_first())
        new_name << c;
      last_upper = false;
      last_digit = false;
      last_underscore = c == '_';
    }
  }
  std::string result = new_name.str();
  // Fix special cases.
  boost::replace_all(result, "$_", "");
  boost::replace_all(result, "win_32_", "win32_");
  boost::replace_all(result, "etc_2", "etc2");
  result = std::regex_replace(result, std::regex("(int|float|pack|bc)_([0-9])"),
                              "$1$2");
  result = std::regex_replace(result, std::regex("([0-9])x_([0-9])"), "$1x$2");
  result = std::regex_replace(result, std::regex("([0-9])_bpp"), "$1bpp");
  boost::replace_all(result, "pvrtc_", "pvrtc");
  boost::replace_all(result, "bc6_h", "bc6h");
  boost::replace_all(result, "src_1", "src1");
  // Remove trailing underscores.
  result = std::regex_replace(result, std::regex("_$"), "");
  return result;
}

static std::string to_upper_case(const std::string& name)
{
  std::stringstream result;
  bool last_upper_or_digit_or_underscore = true;
  for (auto& c : name)
  {
    if (isupper(c) != 0)
    {
      if (!last_upper_or_digit_or_underscore)
        result << '_';
      result << c;
      last_upper_or_digit_or_underscore = true;
    }
    else
    {
      result << static_cast<char>(toupper(c));
      last_upper_or_digit_or_underscore = (isdigit(c) != 0) || (c == '_');
    }
  }
  // Fix special case WIN32xxx -> WIN32_xxx
  return std::regex_replace(result.str(), std::regex("WIN32(?!_)"), "WIN32_");
}

static std::string enumerant_name(const std::string& name,
                                  const std::string& type)
{
  auto prefix =
    to_upper_case(std::regex_replace(type, std::regex("FlagBits.*"), "")) + "_";
  auto substr = name.substr(0, prefix.length());
  std::string result;
  if (substr == prefix)
    result = to_lower_case(strip_vk(name.substr(prefix.length())));
  else
    result = to_lower_case(strip_vk(name));
  if (isdigit(result.front()))
    result = "_" + result;
  else if (result == "inline")
    result = "inline_commands";
  return result;
}

static std::string type_to_string(const member_descriptor& member,
                                  bool dereference = false,
                                  bool read_only = false)
{
  std::stringstream result;

  if (member.array_size > 1 || member.array_size_reference)
    result << "std::array<";
  if ((member.flags & member_type_flag::is_const && !dereference) || read_only)
  {
    result << "const ";
  }
  if (member.flags & member_type_flag::is_struct)
    result << "struct ";
  if (member.type->new_name.empty())
    result << member.type->name;
  else
    result << "vk::" << member.type->new_name;
  if ((member.flags & member_type_flag::is_pointer && !dereference) ||
      (member.flags & member_type_flag::is_pointer_to_pointer && dereference))
  {
    result << "*";
  }
  if (member.flags & member_type_flag::is_pointer_to_pointer && !dereference)
  {
    if (member.flags & member_type_flag::is_const)
      result << "* const*";
    else
      result << "**";
  }

  if (member.array_size > 1)
    result << ", " << member.array_size << ">";
  else if (member.array_size_reference)
    result << ", " << member.array_size_reference->inner_text() << ">";
  return result.str();
}

boost::filesystem::path program_options::input_path;
boost::filesystem::path program_options::output_filename;
boost::filesystem::path program_options::clang_format;
boost::filesystem::path program_options::snippets_vs_path;
boost::filesystem::path program_options::snippets_qt_path;
boost::filesystem::path program_options::graph_filename;

application::application() = default;

int application::run()
{
  if (!fs::exists(input_path))
  {
    log::error() << "Input path " << input_path << " does not exist.";
    return EXIT_FAILURE;
  }
  if (fs::is_directory(input_path))
  {
    log::error() << "Input path " << input_path << " is a directory.";
    return EXIT_FAILURE;
  }

  xml::node root_node;

  std::ifstream input_file(input_path.generic_string(), std::ios_base::in);
  if (!input_file)
  {
    BOOST_THROW_EXCEPTION(core::file_open_error()
                          << core::file_name_info(input_path.generic_string()));
  }
  input_file >> root_node;

  auto* registry_node = root_node.element_by_name("registry");
  BOOST_ASSERT(registry_node);
  if (!registry_node)
    return EXIT_FAILURE;

  auto* comment_node = registry_node->element_by_name("comment");
  BOOST_ASSERT(comment_node);
  if (comment_node)
  {
    _copyright = comment_node->inner_text();
    if (!_copyright.empty())
    {
      _copyright = "// " + _copyright;
      boost::replace_all(_copyright, "\n", br "// ");
    }
  }

  auto* platforms_node = registry_node->element_by_name("platforms");
  auto* types_node = registry_node->element_by_name("types");
  auto* commands_node = registry_node->element_by_name("commands");
  auto* extensions_node = registry_node->element_by_name("extensions");

  BOOST_ASSERT(platforms_node && types_node && commands_node &&
               extensions_node);
  if (!platforms_node || !types_node || !commands_node || !extensions_node)
  {
    return EXIT_FAILURE;
  }

  // Collecting type information is a two pass process. The first pass gathers
  // all type names to make sure the second pass can properly build up the
  // dependency graph.
  parse_platforms(*platforms_node);
  parse_type_names(*types_node);
  parse_type_details(*types_node);
  parse_enums(*registry_node);
  parse_commands(*commands_node);
  for (auto& child_node : registry_node->children)
  {
    if (child_node->name == "feature")
      parse_feature(*child_node);
  }
  parse_extensions(*extensions_node);

  fix_names();
  add_empty_bit_field_enumerants();

  for (auto& feature : _features)
    propagate_feature(feature);
  for (auto& extension : _extensions)
    propagate_platform(extension);

  auto output_path = output_filename.parent_path();
  if (!fs::exists(output_filename))
    fs::create_directories(output_path);
  log::info() << "Writing file "
              << fs::absolute(output_filename).generic_string();
  _file.open(output_filename.generic_string(),
             std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!_file)
  {
    BOOST_THROW_EXCEPTION(core::file_open_error() << core::context_info(
                            output_filename.generic_string()));
  }

  _file << indent_width(2);
  _file << "////////////////////////////////////////////////////////////////" br
           "// WARNING! Automatically generated file.                     //" br
           "// Do not touch anything or your changes will be overwritten! //" br
           "////////////////////////////////////////////////////////////////" br
        << _copyright << br2;
  _file << "#ifndef SHIFT_RENDER_VK_VULKAN_H" br;
  _file << "#define SHIFT_RENDER_VK_VULKAN_H" br2;

  _file << "#include <cstdint>" br;
  _file << "#include <array>" br;
  _file << "#include <vector>" br;
  _file << "#include <vulkan/vulkan.h>" br;
  _file << "#include <shift/core/bit_field.h>" br2;

  _file << indent << "namespace shift::render::vk" << br;
  _file << indent << "{" << br << inc_indent;

  for (const auto& feature : _features)
  {
    for (const auto* type : feature.types)
      print_cpp(*type);
    for (const auto* command : feature.commands)
      print_cpp(*command);
  }
  // Print platform independent extensions first to prevent types or commands
  // wrongly end up in #ifdef blocks.
  for (const auto& extension : _extensions)
  {
    if (extension.platform == nullptr)
      print_cpp(extension);
  }
  // Now print platform dependent extensions.
  for (const auto& extension : _extensions)
  {
    if (extension.platform != nullptr)
      print_cpp(extension);
  }
  // Print all types that have not been referenced by any other type, command,
  // or extension (e.g. VkDebugReportFlagBitsEXT).
  for (const auto& type : _types)
    print_cpp(type);
  _file << dec_indent << indent << "}" br2;

  _file << "#endif" br;
  _file.close();

  if (!clang_format.empty() && fs::exists(clang_format))
  {
    std::system((clang_format.generic_string() + " -style=file -i " +
                 output_filename.generic_string())
                  .c_str());
  }

  if (!fs::exists(snippets_vs_path))
    fs::create_directories(snippets_vs_path);
  print_snippets_vs();

  if (!fs::exists(snippets_qt_path))
    fs::create_directories(snippets_qt_path);
  print_snippets_qt();

  _printed.clear();

  auto graph_path = graph_filename.parent_path();
  if (!fs::exists(graph_filename))
    fs::create_directories(graph_path);
  log::info() << "Writing file "
              << fs::absolute(graph_filename).generic_string();
  _file.open(graph_filename.generic_string(),
             std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!_file)
  {
    BOOST_THROW_EXCEPTION(core::file_open_error() << core::context_info(
                            graph_filename.generic_string()));
  }

  _file << "digraph Vulkan {" br << inc_indent;
  _file << indent << "rankdir=LR;" br;
  _file << indent << "rankdir=LR;" br;
  _file << indent << "size=\"8,5\"" br;
  for (const auto& feature : _features)
  {
    for (const auto* command : feature.commands)
      print_graph(*command);
  }
  for (const auto& extension : _extensions)
    print_graph(extension);
  _file << dec_indent << indent << "}" br;
  _file.close();

  return EXIT_SUCCESS;
}

void application::parse_platforms(const parser::xml::node& platforms_node)
{
  for (auto& platform_node : platforms_node.children)
  {
    if (platform_node->type == xml::node_type::element)
    {
      if (platform_node->name == "platform")
      {
        platform_descriptor platform;
        platform.node = platform_node.get();
        platform.macro = platform_node->attribute("protect");
        if (platform_node->has_attribute("comment"))
          platform.comment = platform_node->attribute("comment");
        _platforms.insert_or_assign(platform_node->attribute("name"),
                                    std::move(platform));
      }
      else
      {
        BOOST_THROW_EXCEPTION(core::runtime_error()
                              << core::context_info("Unexpected XML element")
                              << core::context_info(platform_node->name));
      }
    }
  }
}

void application::parse_type_names(const parser::xml::node& types_node)
{
  for (auto& type_node : types_node.children)
  {
    if (type_node->type == xml::node_type::element)
    {
      if (type_node->name == "comment")
      {
        /// ToDo: Groups of types may have comments like "Basic C types...".
        /// Use these somehow.
        continue;
      }
      else if (type_node->name == "type")
      {
        type_descriptor type;
        type.node = type_node.get();

        type.category = type_category::external;
        if (type_node->has_attribute("category"))
        {
          std::string category_name = type_node->attribute("category");
          if (category_name == "basetype")
            type.category = type_category::base_type;
          else if (category_name == "bitmask")
            type.category = type_category::bit_mask;
          else if (category_name == "handle")
            type.category = type_category::handle;
          else if (category_name == "funcpointer")
            type.category = type_category::function;
          else if (category_name == "enum")
            type.category = type_category::enum_type;
          else if (category_name == "struct")
            type.category = type_category::struct_type;
          else if (category_name == "union")
            type.category = type_category::union_type;
          else if (category_name == "include")
            type.category = type_category::include;
          else if (category_name == "define")
            type.category = type_category::define;
          else
          {
            BOOST_THROW_EXCEPTION(
              core::runtime_error()
              << core::context_info("Unexpected type category")
              << core::context_info(category_name));
          }
        }

        if (type_node->has_attribute("name"))
        {
          type.name = type_node->attribute("name");
          if (type.name == "VkBaseOutStructure" ||
              type.name == "VkBaseInStructure")
          {
            // Ignore these types, which are not used as of 1.1.77.
            continue;
          }
        }
        else
        {
          auto name_node = type_node->element_by_name("name");
          BOOST_ASSERT(name_node);
          type.name = name_node->inner_text();
        }
        if (type.category == type_category::bit_mask ||
            type.category == type_category::enum_type ||
            type.category == type_category::struct_type ||
            type.category == type_category::union_type)
        {
          type.new_name = std::regex_replace(to_lower_case(strip_vk(type.name)),
                                             std::regex("_flag_bits"), "_flag");
        }

        _types.push_back(std::move(type));
      }
      else
      {
        BOOST_THROW_EXCEPTION(core::runtime_error()
                              << core::context_info("Unexpected XML element")
                              << core::context_info(type_node->name));
      }
    }
  }
}

void application::parse_type_details(const parser::xml::node& /*types_node*/)
{
  for (auto& type : _types)
  {
    if (type.node->has_attribute("requires"))
      type.require = type_by_name(type.node->attribute("requires"));

    switch (type.category)
    {
    case type_category::base_type:
    {
      for (const auto& type_child_node : type.node->children)
      {
        if (type_child_node->type != xml::node_type::element)
          continue;
        if (type_child_node->name == "type")
          type.require = type_by_name(type_child_node->inner_text());
      }
      BOOST_ASSERT(type.require);
      break;
    }

    case type_category::enum_type:
    {
      // NOP. Enums have separate <enums> tags.
      break;
    }

    case type_category::struct_type:
    case type_category::union_type:
    {
      // Handle special cases, like e.g.
      // <type category="struct"
      //       name="VkMemoryRequirements2KHR"
      //       alias="VkMemoryRequirements2"/>
      if (type.node->has_attribute("alias"))
      {
        type.alias = type_by_name(type.node->attribute("alias"));
        BOOST_ASSERT(type.alias != nullptr);
      }

      member_descriptor* last_member = nullptr;
      for (const auto& type_child : type.node->children)
      {
        if (type_child->type == xml::node_type::element)
        {
          if (type_child->name == "member")
          {
            member_descriptor member;
            parse_member(member, *type_child);
            if (member.read_only)
              type.has_read_only_members = true;
            type.members.push_back(member);
            last_member = &type.members.back();
          }
          else if (type_child->name == "comment")
          {
            /// ToDo: A group of struct/union members may have comments. Somehow
            /// make use of them.
            log::debug() << "Make use of comment: " << type_child->inner_text();
          }
          else
          {
            BOOST_THROW_EXCEPTION(
              core::runtime_error()
              << core::context_info("Unexpected XML element")
              << core::context_info(type_child->name));
          }
        }
        else if (type_child->type == xml::node_type::comment)
        {
          if (last_member)
          {
            last_member->comment = type_child->text;
            last_member = nullptr;
          }
          else
          {
            /// ToDo: There are some comments that group members together.
            /// Find a way how to handle these (see <!-- memory limits -->).
          }
        }
      }
      for (auto& member : type.members)
      {
        if (member.node->has_attribute("len"))
        {
          auto length_name =
            std::regex_replace(member.node->attribute("len"),
                               std::regex("[,]?null-terminated"), "");
          for (auto& other_member : type.members)
          {
            if (other_member.name == length_name)
            {
              member.length = &other_member;
              break;
            }
          }
        }
      }
      break;
    }

    default:
      break;
    }
  }
}

void application::parse_member(member_descriptor& member,
                               const parser::xml::node& member_node)
{
  member.node = &member_node;
  if (member.node->has_attribute("optional") &&
      member.node->attribute("optional") == "true")
  {
    member.flags |= member_type_flag::is_optional;
  }

  if (member.node->has_attribute("values"))
    member.default_value = member.node->attribute("values");

  bool has_type = false;
  bool has_name = false;
  bool is_array = false;
  bool has_array_size = false;
  for (const auto& member_child : member.node->children)
  {
    if (member_child->type == xml::node_type::element)
    {
      if (!has_type)
      {
        if (member_child->name == "type")
          member.type = type_by_name(member_child->inner_text());
        else
        {
          BOOST_THROW_EXCEPTION(core::runtime_error()
                                << core::context_info("Unexpected XML element")
                                << core::context_info(member_child->name));
        }
        has_type = true;
      }
      else if (!has_name)
      {
        if (member_child->name == "name")
        {
          member.name = member_child->inner_text();
          if (member.name != "sType")
          {
            member.new_name =
              to_lower_case(strip_vk(strip_hungarian(member.name)));
          }
          else
          {
            // Avoid collision between multiple "type" members.
            member.new_name = "structure_type";
          }
        }
        else
        {
          BOOST_THROW_EXCEPTION(core::runtime_error()
                                << core::context_info("Unexpected XML element")
                                << core::context_info(member_child->name));
        }
        member.read_only = member.name == "sType";
        has_name = true;
      }
      else if (is_array && !has_array_size)
      {
        if (member_child->name == "enum")
        {
          member.array_size_reference = member_child.get();
          has_array_size = true;
        }
        else
        {
          BOOST_THROW_EXCEPTION(core::runtime_error()
                                << core::context_info("Unexpected XML element")
                                << core::context_info(member_child->name));
        }
        has_name = true;
      }
      else if (member_child->name == "comment")
      {
        member.comment = member_child->inner_text();
        log::debug() << "member comment: " << member.comment;
      }
      else
      {
        BOOST_ASSERT(false);
      }
    }
    else if (member_child->type == xml::node_type::text)
    {
      auto text = boost::trim_copy(member_child->text);
      if (text.empty())
        continue;
      if (!has_type)
      {
        if (text == "const")
          member.flags |= member_type_flag::is_const;
        else if (text == "struct")
          member.flags |= member_type_flag::is_struct;
        else if (text == "const struct")
        {
          member.flags |= member_type_flag::is_const;
          member.flags |= member_type_flag::is_struct;
        }
        else
          BOOST_ASSERT(false);
      }
      else if (!has_name)
      {
        auto text = boost::trim_copy(member_child->text);
        if (text == "*")
          member.flags |= member_type_flag::is_pointer;
        else if (text == "**")
          member.flags |= member_type_flag::is_pointer_to_pointer;
        else if (text == "* const*")
        {
          member.flags |= member_type_flag::is_const;
          member.flags |= member_type_flag::is_pointer_to_pointer;
        }
        else
          BOOST_ASSERT(false);
      }
      else if (!is_array)
      {
        is_array = true;
        if (member_child->text != "[")
        {
          // In this case the array size is given directly as a
          // numeric constant (e.g. "[2]").
          member.array_size = static_cast<std::size_t>(std::atol(
            member_child->text.substr(1, member_child->text.size() - 2)
              .c_str()));
          BOOST_ASSERT(member.array_size >= 2);
          has_array_size = true;
        }
      }
      else
      {
        BOOST_ASSERT(has_array_size && member.array_size_reference);
        BOOST_ASSERT(member_child->text == "]");
      }
    }
  }
  BOOST_ASSERT(has_type && has_name);
}

void application::parse_enums(const parser::xml::node& registry_node)
{
  for (const auto& registry_child_node : registry_node.children)
  {
    if (registry_child_node->type != xml::node_type::element)
      continue;
    if (registry_child_node->name == "enums")
    {
      if (!registry_child_node->has_attribute("type"))
      {
        /// ToDo: This is the "API Constants" enum that must be translated to
        /// constants.
        continue;
      }
      // auto enum_type = registry_child_node->attribute("type");

      BOOST_ASSERT(registry_child_node->has_attribute("name"));
      auto name = registry_child_node->attribute("name");
      type_descriptor* enum_type = nullptr;
      for (auto& type : _types)
      {
        if (type.name == name)
        {
          enum_type = &type;
          break;
        }
      }
      BOOST_ASSERT(enum_type);
      if (!enum_type)
        continue;
      for (const auto& enum_node : registry_child_node->children)
      {
        if (enum_node->type != xml::node_type::element)
          continue;
        if (enum_node->name == "comment")
        {
          /// ToDo: Process comment tags.
          continue;
        }
        else if (enum_node->name != "enum")
        {
          BOOST_ASSERT(enum_node->name == "unused");
          continue;
        }

        BOOST_ASSERT(enum_node->has_attribute("name") &&
                     (enum_node->has_attribute("value") ||
                      enum_node->has_attribute("bitpos") ||
                      enum_node->has_attribute("alias")));

        enumerant_descriptor enumerant;
        enumerant.name = enum_node->attribute("name");
        enumerant.new_name = enumerant_name(enumerant.name, enum_type->name);

        if (enum_node->has_attribute("value"))
          enumerant.value = enum_node->attribute("value");
        else if (enum_node->has_attribute("bitpos"))
          enumerant.value = "1 << " + enum_node->attribute("bitpos");
        else if (enum_node->has_attribute("alias"))
        {
          // Handle special cases like
          // <enum name="VK_COLORSPACE_SRGB_NONLINEAR_KHR"
          //       alias="VK_COLOR_SPACE_SRGB_NONLINEAR_KHR"
          //       comment="Backwards-compatible alias containing a typo"/>
          enumerant.value =
            enumerant_name(enum_node->attribute("alias"), enum_type->name);
        }
        else
          BOOST_ASSERT(false);

        if (enum_node->has_attribute("comment"))
          enumerant.comment = enum_node->attribute("comment");

        enum_type->enumerants.push_back(std::move(enumerant));
      }
    }
  }
}

void application::parse_commands(const parser::xml::node& commands_node)
{
  for (const auto& command_node : commands_node.children)
  {
    if (command_node->type != xml::node_type::element)
      continue;
    if (command_node->name == "command")
    {
      if (command_node->has_attribute("alias"))
      {
        // Skip alias definitions. We want to use the new names only.
        continue;
      }
      command_descriptor command;
      for (const auto& attribute : command_node->attributes)
      {
        if (attribute.first == "successcodes")
          command.success_codes = attribute.second;
        else if (attribute.first == "errorcodes")
          command.error_codes = attribute.second;
        else if (attribute.first == "queues")
          command.queues = attribute.second;
        else if (attribute.first == "renderpass")
          command.render_pass = attribute.second;
        else if (attribute.first == "cmdbufferlevel")
          command.command_buffer_level = attribute.second;
        else if (attribute.first == "pipeline")
          command.pipeline = attribute.second;
        else if (attribute.first == "comment")
          command.comment = attribute.second;
        else
          BOOST_ASSERT(false);
      }

      for (const auto& command_child_node : command_node->children)
      {
        if (command_child_node->type != xml::node_type::element)
          continue;
        if (command_child_node->name == "proto")
          parse_member(command.prototype, *command_child_node);
        else if (command_child_node->name == "param")
        {
          member_descriptor parameter;
          parse_member(parameter, *command_child_node);
          command.parameters.push_back(std::move(parameter));
        }
        else if (command_child_node->name == "validity")
        {
          for (const auto& validity_node : command_child_node->children)
          {
            if (validity_node->type != xml::node_type::element)
              continue;
            if (validity_node->name == "usage")
              command.validity.push_back(validity_node->inner_text());
            else
              BOOST_ASSERT(false);
          }
        }
        else if (command_child_node->name == "implicitexternsyncparams")
        {
          /// ToDo: Do something with this info.
        }
        else
          BOOST_ASSERT(false);
      }
      for (auto& parameter : command.parameters)
      {
        if (parameter.node->has_attribute("len"))
        {
          auto length_name =
            std::regex_replace(parameter.node->attribute("len"),
                               std::regex("[,]?null-terminated"), "");
          for (auto& other_parameter : command.parameters)
          {
            if (other_parameter.name == length_name)
            {
              parameter.length = &other_parameter;
              break;
            }
          }
        }
      }

      _commands.push_back(std::move(command));
    }
    else
      BOOST_ASSERT(false);
  }
}

void application::parse_feature(const parser::xml::node& feature_node)
{
  auto& feature = _features.emplace_back();
  for (const auto& feature_child_node : feature_node.children)
  {
    if (feature_child_node->type != xml::node_type::element)
      continue;
    if (feature_child_node->name == "require")
    {
      for (const auto& require_node : feature_child_node->children)
      {
        if (require_node->type != xml::node_type::element)
          continue;
        if (require_node->name == "comment")
          continue;
        if (!require_node->has_attribute("name"))
        {
          BOOST_THROW_EXCEPTION(
            core::runtime_error()
            << core::context_info("Missing 'name' attribute")
            << core::context_info(require_node->name));
        }
        if (require_node->name == "type")
        {
          auto* type = type_by_name(require_node->attribute("name"));
          if (type != nullptr)
            feature.types.push_back(type);
        }
        else if (require_node->name == "enum")
        {
          BOOST_ASSERT(require_node->has_attribute("name"));
          auto enum_name = require_node->attribute("name");

          std::stringstream value;
          if (require_node->has_attribute("dir"))
          {
            BOOST_ASSERT(require_node->attribute("dir") == "-");
            value << require_node->attribute("dir");
          }
          if (require_node->has_attribute("value"))
            value << require_node->attribute("value");
          else if (require_node->has_attribute("offset"))
          {
            auto offset = std::atoi(require_node->attribute("offset").c_str());
            auto extension_number =
              std::atoi(require_node->attribute("extnumber").c_str());
            value << (1000000000 + (extension_number - 1) * 1000 + offset);
          }
          else if (require_node->has_attribute("bitpos"))
            value << "1 << " << require_node->attribute("bitpos");
          else
          {
            log::warning() << "Skipping requirement of enum "
                           << require_node->attribute("name");
            // Simply ignore this enum for now.
            // Sample:
            // <feature api="vulkan" name="VK_VERSION_1_0" number="1.0">
            //   <require comment="API constants">
            //     <enum name="VK_TRUE"/>
          }

          if (require_node->has_attribute("extends"))
          {
            auto extends_name = require_node->attribute("extends");
            bool found = false;
            for (auto& extends_node : _types)
            {
              if (extends_node.name == extends_name)
              {
                BOOST_ASSERT(extends_node.category == type_category::enum_type);
                auto& enumerant = extends_node.enumerants.emplace_back();
                enumerant.node = require_node.get();
                enumerant.name = enum_name;
                enumerant.new_name =
                  enumerant_name(enum_name, extends_node.name);
                enumerant.value = value.str();
                if (require_node->has_attribute("comment"))
                  enumerant.comment = require_node->attribute("comment");
                found = true;
                break;
              }
            }
            BOOST_ASSERT(found);
          }
        }
        else if (require_node->name == "command")
        {
          feature.commands.push_back(
            command_by_name(require_node->attribute("name")));
        }
        else
          BOOST_ASSERT(false);
      }
    }
    else
      BOOST_ASSERT(false);
  }
}

void application::parse_extensions(const parser::xml::node& extensions_node)
{
  for (const auto& extension_node : extensions_node.children)
  {
    if (extension_node->type != xml::node_type::element)
      continue;
    if (extension_node->name == "extension")
    {
      // Skip disabled extensions.
      if (extension_node->has_attribute("supported") &&
          extension_node->attribute("supported") != "vulkan")
      {
        continue;
      }

      BOOST_ASSERT(extension_node->has_attribute("name") &&
                   extension_node->has_attribute("number"));
      if (!extension_node->has_attribute("name") ||
          !extension_node->has_attribute("number"))
      {
        continue;
      }

      bool skip_extension = false;
      extension_descriptor extension;
      extension.name = extension_node->attribute("name");
      extension.number = static_cast<std::uint32_t>(
        std::atoi(extension_node->attribute("number").c_str()));
      if (extension_node->has_attribute("platform"))
      {
        auto platform = extension_node->attribute("platform");
        if (_platforms.find(platform) == _platforms.end())
        {
          BOOST_THROW_EXCEPTION(core::runtime_error()
                                << core::context_info("Platform not found")
                                << core::context_info(platform));
        }
        extension.platform = &_platforms.at(platform);
      }

      for (const auto& extension_child_node : extension_node->children)
      {
        if (skip_extension)
          break;
        if (extension_child_node->type != xml::node_type::element)
          continue;
        if (extension_child_node->name == "require")
        {
          for (const auto& require_node : extension_child_node->children)
          {
            if (require_node->type != xml::node_type::element)
              continue;
            if (require_node->name == "comment")
            {
              /// ToDo: Process extension comments.
            }
            else if (require_node->name == "type")
            {
              if (!require_node->has_attribute("name"))
              {
                BOOST_THROW_EXCEPTION(
                  core::runtime_error()
                  << core::context_info("Missing 'name' attribute"));
              }
              extension.types.push_back(
                type_by_name(require_node->attribute("name")));
            }
            else if (require_node->name == "enum")
            {
              BOOST_ASSERT(require_node->has_attribute("name"));
              auto enum_name = require_node->attribute("name");

              std::stringstream value;
              if (require_node->has_attribute("dir"))
              {
                BOOST_ASSERT(require_node->attribute("dir") == "-");
                value << require_node->attribute("dir");
              }
              if (require_node->has_attribute("value"))
                value << require_node->attribute("value");
              else if (require_node->has_attribute("offset"))
              {
                auto offset = static_cast<std::uint32_t>(
                  std::atoi(require_node->attribute("offset").c_str()));
                value << (1000000000 + (extension.number - 1) * 1000 + offset);
              }
              else if (require_node->has_attribute("bitpos"))
                value << "1 << " << require_node->attribute("bitpos");
              else
              {
                // Skip alias definitions. We want to use the new names only.
                skip_extension = true;
                break;
              }

              if (require_node->has_attribute("extends"))
              {
                auto extends_name = require_node->attribute("extends");
                bool found = false;
                for (auto& extends_node : _types)
                {
                  if (extends_node.name == extends_name)
                  {
                    BOOST_ASSERT(extends_node.category ==
                                 type_category::enum_type);
                    enumerant_descriptor enumerant;
                    enumerant.node = require_node.get();
                    enumerant.name = enum_name;
                    enumerant.new_name =
                      enumerant_name(enum_name, extends_node.name);
                    enumerant.value = value.str();
                    if (require_node->has_attribute("comment"))
                      enumerant.comment = require_node->attribute("comment");
                    extends_node.enumerants.push_back(std::move(enumerant));
                    found = true;
                    break;
                  }
                }
                BOOST_ASSERT(found);
              }

              // extension.types.push_back(
              //  type_by_name(require_node->attribute("name")));
            }
            else if (require_node->name == "command")
            {
              if (!require_node->has_attribute("name"))
              {
                BOOST_THROW_EXCEPTION(
                  core::runtime_error()
                  << core::context_info("Missing 'name' attribute"));
              }
              auto* command = command_by_name(require_node->attribute("name"));
              if (command)
                extension.commands.push_back(command);
              else
              {
                // Skip this extensions because its commands are either missing,
                // or have been merged to Vulkan core.
                skip_extension = true;
                break;
              }
            }
            else if (require_node->name == "usage")
            {
              /// ToDo: Do something with the info.
            }
            else
              BOOST_ASSERT(false);
          }
        }
        else
          BOOST_ASSERT(false);
      }

      if (skip_extension)
      {
        log::info() << "Skipping extension " << extension.name << ".";
        continue;
      }
      _extensions.emplace_back(std::move(extension));
    }
    else
      BOOST_ASSERT(false);
  }
}

void application::fix_names()
{
  for (auto& type : _types)
  {
    if (type.name == "VkLogicOp")
    {
      for (auto& enumerant : type.enumerants)
      {
        if (!boost::ends_with(enumerant.new_name, "_op"))
          enumerant.new_name += "_op";
      }
    }
  }
}

void application::add_empty_bit_field_enumerants()
{
  for (auto& type : _types)
  {
    if (type.category == type_category::bit_mask)
    {
      auto flag_type_name =
        std::regex_replace(type.name, std::regex("Flags"), "FlagBits");
      type_descriptor* flag_type = nullptr;
      for (auto& other_type : _types)
      {
        if (other_type.name == flag_type_name)
        {
          flag_type = &other_type;
          break;
        }
      }
      if (flag_type)
      {
        if (flag_type->enumerants.empty() ||
            flag_type->enumerants.front().new_name != "none")
        {
          enumerant_descriptor enumerant;
          enumerant.name = "";
          enumerant.new_name = "none";
          enumerant.value = "0";
          enumerant.comment = "Custom enumerant not available in Vulkan.";
          flag_type->enumerants.insert(flag_type->enumerants.begin(),
                                       std::move(enumerant));
        }
      }
    }
  }
}

std::string application::cpp_default_value(const type_descriptor& parent_type,
                                           const member_descriptor& member)
{
  std::stringstream ss;
  if (member.name == "sType")
  {
    if (!member.default_value.empty())
    {
      ss << "vk::structure_type::"
         << enumerant_name(member.default_value, member.type->name);
    }
    else
      ss << "vk::structure_type::" << parent_type.new_name;
  }
  else if (member.array_size > 1 || member.array_size_reference)
  {
    ss << "{}";
  }
  else if (member.flags & member_type_flag::is_pointer ||
           member.flags & member_type_flag::is_pointer_to_pointer ||
           member.type->category == type_category::handle ||
           member.type->category == type_category::function)
  {
    ss << "nullptr";
  }
  else if (member.array_size > 1 || member.array_size_reference ||
           member.type->category == type_category::struct_type ||
           member.type->category == type_category::union_type)
  {
    ss << type_to_string(member) << "{}";
  }
  else if (member.type->category == type_category::enum_type)
  {
    ss << type_to_string(member)
       << "::" << member.type->enumerants.front().new_name;
  }
  else if (member.type->category == type_category::external)
  {
    if (member.type->name == "float")
      ss << "0.0f";
    else
    {
      // Either void, char, uint8_t, uint32_t, uint64_t, int32_t, or size_t.
      ss << "0";
    }
  }
  else if (member.type->category == type_category::base_type)
  {
    if (member.type->name == "VkBool32")
      ss << "VK_FALSE";
    else
    {
      // Either uint32_t, uint32_t, uint32_t, or uint64_t.
      ss << "0";
    }
  }
  else if (member.type->category == type_category::bit_mask)
  {
    auto flag_type_name =
      std::regex_replace(member.type->name, std::regex("Flags"), "FlagBits");
    const type_descriptor* flag_type = nullptr;
    for (const auto& other_type : _types)
    {
      if (other_type.name == flag_type_name)
      {
        flag_type = &other_type;
        break;
      }
    }
    if (flag_type)
    {
      ss << "vk::" << flag_type->new_name
         << "::" << flag_type->enumerants.front().new_name;
    }
    else
      ss << "0";
  }
  else
    BOOST_ASSERT(false);
  return ss.str();
}

void application::propagate_feature(feature_descriptor& feature)
{
  for (auto* type : feature.types)
  {
    if (auto success = type->features.insert(&feature).second; success)
    {
      // Continue propagating feature dependency.
      for (auto& member : type->members)
        propagate_feature(*member.type, *type);
    }
  }
}

void application::propagate_feature(type_descriptor& type,
                                    type_descriptor& other_type)
{
  auto propagate = false;
  for (auto* feature : other_type.features)
  {
    if (auto success = type.features.insert(feature).second; success)
      propagate = true;
  }
  if (propagate)
  {
    for (auto& member : type.members)
      propagate_feature(*member.type, type);
  }
}

void application::propagate_platform(extension_descriptor& extension)
{
  for (auto* type : extension.types)
  {
    if (auto success = type->platforms.insert(extension.platform).second;
        success)
    {
      // Continue propagating platform dependency.
      for (auto& member : type->members)
        propagate_platform(*member.type, *type);
    }
  }
}

void application::propagate_platform(type_descriptor& type,
                                     type_descriptor& other_type)
{
  auto propagate = false;
  for (auto* platform : other_type.platforms)
  {
    if (auto success = type.platforms.insert(platform).second; success)
      propagate = true;
  }
  if (propagate)
  {
    for (auto& member : type.members)
      propagate_platform(*member.type, type);
  }
}

void application::print_cpp(const type_descriptor& type)
{
  if (!_printed.emplace(&type).second != 0)
    return;

  // ToDo: We need to skip this specific type because it is referenced only in a
  // disabled extension and would otherwise be printed without #ifdef blocks.
  if (type.name == "VkNativeBufferANDROID")
    return;

  auto open_platform_guard = [&]() {
    if (type.platforms.empty())
      return false;
    for (auto* platform : type.platforms)
    {
      if (!platform)
        return false;
    }
    // Types that are not part of standard features but do have a set of
    // dependent platforms must be put into ifdef blocks.
    // Note that some types are required by platform specific extensions but
    // also by standard features and thus must not be put into ifdef blocks.
    if (!type.features.empty())
      return false;

    bool first = true;
    for (auto* platform : type.platforms)
    {
      if (first)
      {
        first = false;
        _file << "#if defined(" << platform->macro << ")";
      }
      else
        _file << " || defined(" << platform->macro << ")";
    }
    _file << br;
    return true;
  };
  auto close_platform_guard = [&](bool is_platform_specific) {
    if (is_platform_specific)
      _file << "#endif" br;
  };

  bool is_visible = false;
  auto update_visibility = [&](bool visible) {
    if (!is_visible && visible)
    {
      _file << dec_indent << indent << "public:" br << inc_indent;
      is_visible = true;
    }
    else if (is_visible && !visible)
    {
      _file << dec_indent << indent << "private:" br << inc_indent;
      is_visible = false;
    }
  };

  switch (type.category)
  {
  case type_category::base_type:
  {
    // NOP.
    break;
  }

  case type_category::bit_mask:
  {
    // Find xxxFlagBits type.
    auto flag_type_name =
      std::regex_replace(type.name, std::regex("Flags"), "FlagBits");
    const type_descriptor* flag_type = nullptr;
    for (const auto& other_type : _types)
    {
      if (other_type.name == flag_type_name)
      {
        flag_type = &other_type;
        break;
      }
    }
    if (flag_type)
    {
      // Print dependency.
      print_cpp(*flag_type);

      bool platform_guard = open_platform_guard();

      // Print flags type definition.
      _file << indent << "using " << type.new_name
            << " = shift::core::bit_field<" << flag_type->new_name << ", "
            << type.name << ">;" br;

      // Write support operator|(flag, flag) -> flags.
      _file << indent << "inline constexpr " << type.new_name << " operator|("
            << flag_type->new_name << " lhs, " << flag_type->new_name
            << " rhs)" br;
      _file << indent << "{" br << inc_indent;
      _file << indent << "return " << type.new_name << "{lhs} | rhs;" br;
      _file << dec_indent << indent << "}";

      close_platform_guard(platform_guard);
    }
    else
    {
      bool platform_guard = open_platform_guard();
      _file << indent << "using " << type.new_name << " = VkFlags;" br;
      close_platform_guard(platform_guard);
    }
    break;
  }

  case type_category::handle:
  {
    //_file << type.node->inner_text() << br;
    break;
  }

  case type_category::function:
  {
    break;
  }

  case type_category::enum_type:
  {
    if (type.is_fake_enum)
      break;  /// ToDo: make static consts?

    bool platform_guard = open_platform_guard();
    _file << indent << "enum class " << type.new_name << br;
    _file << indent << "{" br << inc_indent;
    for (const auto& enumerant : type.enumerants)
    {
      if (!enumerant.comment.empty())
        _file << indent << "/// " << enumerant.comment << br;
      if (!enumerant.name.empty())
        _file << indent << "/// @see " << enumerant.name << br;
      _file << indent << enumerant.new_name << " = " << enumerant.value
            << "," br;
    }
    _file << dec_indent << indent << "};" br;
    close_platform_guard(platform_guard);
    break;
  }

  case type_category::struct_type:
  {
    // Print dependencies.
    for (const auto& member : type.members)
      print_cpp(*member.type);

    BOOST_ASSERT(!type.new_name.empty());

    // Handle special case of alias types.
    if (type.alias != nullptr)
    {
      // Make sure alias type is already printed.
      print_cpp(*type.alias);
      _file << br << indent << "using " << type.new_name << " = "
            << type.alias->new_name << ";" br;
      break;
    }

    bool platform_guard = open_platform_guard();
    _file << br << indent << "/// Enhanced replacement type for " << type.name
          << "." << br;
    _file << indent << "class " << type.new_name << br;
    _file << indent << "{" br << inc_indent;
    update_visibility(true);

    // Constructors.
    _file << indent << "/// Default constructor." br;
    _file << indent << VS_CONSTEXPR << type.new_name << "() = default;" br;

#pragma region Constructor
    if (!type.members.empty())
    {
      _file << br << indent << "/// Constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "(";
      bool first_parameter = true;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        if (first_parameter)
          first_parameter = false;
        else
          _file << ", ";
        _file << type_to_string(member) << " initial_" << member.new_name;
      }
      _file << ") noexcept" br;
      first_parameter = true;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        if (first_parameter)
        {
          _file << indent << ": ";
          first_parameter = false;
        }
        else
          _file << indent << ", ";
        _file << "_" << member.new_name << "(std::move(initial_"
              << member.new_name << "))" br;
      }
      _file << indent << "{" br;
      _file << indent << "}" br;
    }
#pragma endregion

    if (type.has_read_only_members)
    {
#pragma region Copy constructor
      _file << br << indent << "/// Copy constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "(const "
            << type.new_name << "& other) noexcept" br;
      bool first_parameter = true;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        if (first_parameter)
        {
          _file << indent << ": ";
          first_parameter = false;
        }
        else
          _file << indent << ", ";
        _file << "_" << member.new_name << "(other._" << member.new_name
              << ")" br;
      }
      _file << indent << "{" br;
      _file << indent << "}" br;
#pragma endregion

#pragma region Move constructor
      _file << br << indent << "/// Move constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "(" << type.new_name
            << "&& other) noexcept" br;
      first_parameter = true;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        if (first_parameter)
        {
          _file << indent << ": ";
          first_parameter = false;
        }
        else
          _file << indent << ", ";
        _file << "_" << member.new_name << "(std::move(other._"
              << member.new_name << "))" br;
      }
      _file << indent << "{" br;
      _file << indent << "}" br;
#pragma endregion

#pragma region Copy assignment operator
      _file << br << indent << "/// Copy assignment operator." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "& operator=(const "
            << type.new_name << "& other) noexcept" br;
      _file << indent << "{" br << inc_indent;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        _file << indent << "_" << member.new_name << " = other._"
              << member.new_name << ";" br;
      }
      _file << indent << "return *this;" br;
      _file << dec_indent << indent << "}" br;
#pragma endregion

#pragma region Move assignment operator
      _file << br << indent << "/// Move assignment operator." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "& operator=("
            << type.new_name << "&& other) noexcept" br;
      _file << indent << "{" br << inc_indent;
      for (const auto& member : type.members)
      {
        if (member.read_only)
          continue;
        _file << indent << "_" << member.new_name << " = std::move(other._"
              << member.new_name << ");" br;
      }
      _file << indent << "return *this;" br;
      _file << dec_indent << indent << "}" br;
#pragma endregion
    }
    else
    {
      _file << br << indent << "/// Copy constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "(const "
            << type.new_name << "& other) = default;" br;

      _file << br << indent << "/// Move constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "(" << type.new_name
            << "&& other) = default;" br;

      _file << br << indent << "/// Copy assignment operator." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "& operator=(const "
            << type.new_name << "& other) = default;" br;

      _file << br << indent << "/// Move assignment operator." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "& operator=("
            << type.new_name << "&& other) = default;" br;
    }

#pragma region Conversion operator
    _file << br << indent
          << "/// Conversion operator to original Vulkan type." br;
    _file << indent << "operator const " << type.name << "&() const" br;
    _file << indent << "{" br << inc_indent;
    _file << indent << "return *reinterpret_cast<const " << type.name
          << "*>(this);" br;
    _file << dec_indent << indent << "}" br;
#pragma endregion

    // Getter and setter methods.
    for (const auto& member : type.members)
    {
      // if (member.flags & member_type_flag::is_optional)
      //  BOOST_ASSERT(member.flags & member_type_flag::is_pointer);

#pragma region Member getter methods
      // Getter methods.
      for (unsigned int const_getter = member.read_only ? 1 : 0;
           const_getter <= 1; ++const_getter)
      {
        _file << br << indent;
        if (const_getter)
          _file << VS_CONSTEXPR;
        if (!(member.flags & member_type_flag::is_pointer) &&
            !(member.flags & member_type_flag::is_pointer_to_pointer) &&
            const_getter)
        {
          _file << "const ";
        }
        _file << type_to_string(member);
        if (!(member.flags & member_type_flag::is_pointer) &&
            !(member.flags & member_type_flag::is_pointer_to_pointer))
        {
          _file << "& ";
        }
        else
          _file << " ";
        _file << member.new_name << "()";
        if (const_getter)
          _file << " const";
        _file << br;
        _file << indent << "{" br << inc_indent;
        _file << indent << "return _" << member.new_name << ";" br;
        _file << dec_indent << indent << "}" br;
      }
#pragma endregion

      // Don't write setter methods for private members.
      if (member.flags & member_type_flag::is_private)
        continue;

      // Setter method.
      if (!member.read_only)
      {
#pragma region Member setter method
        _file << br << indent << "void " << member.new_name << "("
              << type_to_string(member) << " new_" << member.new_name << ")" br;
        _file << indent << "{" br << inc_indent;
        _file << indent << "_" << member.new_name << " = new_"
              << member.new_name << ";" br;
        _file << dec_indent << indent << "}" br;
#pragma endregion

        if (member.length && member.type->name != "void")
        {
          BOOST_ASSERT(member.flags & member_type_flag::is_pointer ||
                       member.flags & member_type_flag::is_pointer_to_pointer);

#pragma region Member setter method for std::array
          // Add special setter for std::array.
          _file << br << indent << "template <std::size_t Count>" br;
          _file << indent << "void " << member.new_name << "(";
          if (member.flags & member_type_flag::is_const)
            _file << "const ";
          _file << "std::array<" << type_to_string(member, true)
                << ", Count>& new_" << member.new_name << ")" br;
          _file << indent << "{" br << inc_indent;
          _file << indent << "_" << member.length->new_name << " = static_cast<"
                << type_to_string(*member.length) << ">(new_" << member.new_name
                << ".size());" br;
          _file << indent << "_" << member.new_name << " = new_"
                << member.new_name << ".data();" br;
          _file << dec_indent << indent << "}" br;
#pragma endregion

#pragma region Member setter method for std::vector
          // Add special setter for std::vector.
          _file << br << indent << "void " << member.new_name << "(";
          if (member.flags & member_type_flag::is_const)
            _file << "const ";
          _file << "std::vector<" << type_to_string(member, true) << ">& new_"
                << member.new_name << ")" br;
          _file << indent << "{" br << inc_indent;
          _file << indent << "_" << member.length->new_name << " = static_cast<"
                << type_to_string(*member.length) << ">(new_" << member.new_name
                << ".size());" br;
          _file << indent << "_" << member.new_name << " = new_"
                << member.new_name << ".data();" br;
          _file << dec_indent << indent << "}" br;
#pragma endregion
        }
      }
    }

    // Member variable definitions.
    update_visibility(false);
    for (const auto& member : type.members)
    {
      print_cpp(member, "_");
      _file << " = " << cpp_default_value(type, member) << ";" br;
    }

    _file << dec_indent << indent << "};" br;
    // Add checks to make sure that our definition matches that of Vulkan.
    _file << indent << "static_assert(sizeof(" << type.new_name
          << ") == sizeof(::" << type.name
          << "), \"struct and wrapper have different size!\");" br2;
    close_platform_guard(platform_guard);
    break;
  }

  case type_category::union_type:
  {
    // Print dependencies.
    for (const auto& member : type.members)
      print_cpp(*member.type);

    bool platform_guard = open_platform_guard();
    BOOST_ASSERT(!type.new_name.empty());
    _file << br << indent << "union " << type.new_name << br;
    _file << indent << "{" br << inc_indent;

    _file << indent << "/// Default constructor." br;
    _file << indent << VS_CONSTEXPR << type.new_name << "() noexcept {}" br;
    for (const auto& member : type.members)
    {
      _file << indent << "/// Constructor." br;
      _file << indent << VS_CONSTEXPR << type.new_name << "("
            << type_to_string(member) << " initial_" << member.new_name
            << ") : " << member.new_name << "(initial_" << member.new_name
            << ") {}" br;
    }

    _file << indent << "/// Conversion operator to original Vulkan type." br;
    _file << indent << "operator const " << type.name << "&() const" br;
    _file << indent << "{" br << inc_indent;
    _file << indent << "return *reinterpret_cast<const " << type.name
          << "*>(this);" br;
    _file << dec_indent << indent << "}" br;

    // Member variable definitions.
    bool first_member = true;
    for (const auto& member : type.members)
    {
      print_cpp(member, "");
      if (first_member)
      {
        first_member = false;
        _file << " = " << cpp_default_value(type, member);
      }
      _file << ";" br;
    }

    _file << dec_indent << indent << "};" br;
    // Add checks to make sure that our definition matches that of Vulkan.
    _file << indent << "static_assert(sizeof(" << type.new_name
          << ") == sizeof(::" << type.name
          << "), \"struct and wrapper have different size!\");" br;
    close_platform_guard(platform_guard);
    break;
  }

  case type_category::include:
  {
    // NOP.
    break;
  }

  case type_category::define:
  {
    // NOP.
    break;
  }

  case type_category::external:
  {
    // NOP.
    break;
  }
  }
}

void application::print_cpp(const member_descriptor& member, std::string prefix)
{
  if (!member.comment.empty())
    _file << indent << "/// " << strip_comment(member.comment) << br;

  _file << indent << type_to_string(member, false, member.read_only) << " "
        << prefix << member.new_name;
}

void application::print_cpp(const command_descriptor& command)
{
  if (!_printed.emplace(&command).second)
    return;

  // Print dependencies.
  BOOST_ASSERT(command.prototype.type);
  print_cpp(*command.prototype.type);
  for (const auto& parameter : command.parameters)
  {
    BOOST_ASSERT(parameter.type);
    print_cpp(*parameter.type);
  }

  // Print command.
  _file << indent << "inline " << type_to_string(command.prototype) << " "
        << command.prototype.new_name << "(";
  bool first_parameter = true;
  for (const auto& parameter : command.parameters)
  {
    if (first_parameter)
      first_parameter = false;
    else
      _file << ", ";
    _file << type_to_string(parameter) << " " << parameter.new_name;
  }
  _file << ")" br;
  _file << indent << "{" br << inc_indent;
  _file << indent;
  bool has_return = command.prototype.type->name != "void";
  if (has_return)
  {
    _file << "return static_cast<" << type_to_string(command.prototype) << ">(";
  }
  _file << command.prototype.name << "(";
  first_parameter = true;
  for (const auto& parameter : command.parameters)
  {
    if (first_parameter)
      first_parameter = false;
    else
      _file << ", ";

    if ((parameter.flags & member_type_flag::is_pointer) ||
        (parameter.flags & member_type_flag::is_pointer_to_pointer) ||
        (parameter.array_size > 1) || parameter.array_size_reference)
      _file << "reinterpret_cast<";
    else
      _file << "static_cast<";

    if (parameter.flags & member_type_flag::is_const)
      _file << "const ";
    _file << parameter.type->name;
    if ((parameter.flags & member_type_flag::is_pointer) ||
        (parameter.array_size > 1) || parameter.array_size_reference)
    {
      _file << "*";
    }
    else if (parameter.flags & member_type_flag::is_pointer_to_pointer)
    {
      if (parameter.flags & member_type_flag::is_const)
        _file << "* const*";
      else
        _file << "**";
    }
    _file << ">(" << parameter.new_name;
    if ((parameter.array_size > 1) || parameter.array_size_reference)
      _file << ".data()";
    _file << ")";
  }
  if (has_return)
    _file << ")";
  _file << ");" br;
  _file << dec_indent << indent << "}" br;
}

void application::print_cpp(const extension_descriptor& extension)
{
  if (!_printed.emplace(&extension).second)
    return;

  if (extension.platform)
    _file << "#if defined(" << extension.platform->macro << ")" br;

  // Print dependencies.
  for (const auto& type : extension.types)
  {
    BOOST_ASSERT(type);
    print_cpp(*type);
  }
  for (const auto& command : extension.commands)
  {
    BOOST_ASSERT(command);
    print_cpp(*command);
  }

  if (extension.platform)
    _file << "#endif" br;
}

void application::print_snippets_vs()
{
  for (const auto& type : _types)
  {
    if (type.category != type_category::struct_type)
      continue;

    auto snippets_filename = snippets_vs_path / (type.new_name + ".snippet");
    log::info() << "Writing file "
                << fs::absolute(snippets_filename).generic_string();
    _file.open(snippets_filename.generic_string(), std::ios_base::out |
                                                     std::ios_base::binary |
                                                     std::ios_base::trunc);
    if (!_file)
    {
      BOOST_THROW_EXCEPTION(core::file_open_error() << core::context_info(
                              snippets_filename.generic_string()));
    }

    _file << "<?xml version=\"1.0\" encoding=\"utf-8\" ?>" br;
    _file << "<CodeSnippets  "
             "xmlns=\"http://schemas.microsoft.com/VisualStudio/2005/"
             "CodeSnippet\">" br;
    _file << "  <CodeSnippet Format=\"1.0.0\">" br;
    _file << "    <Header>" br;
    _file << "      <Title>" << type.new_name << "</Title>" br;
    _file << "      <Shortcut>" << boost::replace_all_copy(type.name, "_", "")
          << "</Shortcut>" br;
    _file << "      <Description>Code snippet for Vulkan type " << type.name
          << "</Description>" br;
    _file << "      <Author>Christian Maaser</Author>" br;
    _file << "      <SnippetTypes>" br;
    _file << "        <SnippetType>Expansion</SnippetType>" br;
    _file << "      </SnippetTypes>" br;
    _file << "    </Header>" br;
    _file << "    <Snippet>" br;
    bool first_parameter = true;
    for (const auto& member : type.members)
    {
      if (member.read_only)
        continue;
      if (first_parameter)
      {
        first_parameter = false;
        _file << "      <Declarations>" br;
      }
      _file << "        <Literal>" br;
      _file << "          <ID>" << member.new_name << "</ID>" br;
      _file << "          <ToolTip>Equivalent to Vulkan member " << type.name
            << "::" << member.name << ".</ToolTip>" br;
      _file << "          <Default>"
            << shift::parser::xml::encode_entities(
                 cpp_default_value(type, member))
            << "</Default>" br;
      _file << "        </Literal>" br;
    }

    if (!first_parameter)
    {
      first_parameter = true;
      _file << "      </Declarations>" br;
    }
    _file << "      <Code Language=\"cpp\"><![CDATA[vk::" << type.new_name
          << "(";
    for (const auto& member : type.members)
    {
      if (member.read_only)
        continue;
      if (first_parameter)
        first_parameter = false;
      else
        _file << ",";
      _file << br "  /* " << member.new_name << " */ $" << member.new_name
            << "$";
    }
    _file << ")$end$]]>" br;
    _file << "      </Code>" br;
    _file << "    </Snippet>" br;
    _file << "  </CodeSnippet>" br;
    _file << "</CodeSnippets>" br;
    _file.close();
  }
}

void application::print_snippets_qt()
{
  auto snippets_filename = snippets_qt_path / "snippets.xml";
  log::info() << "Writing file "
              << fs::absolute(snippets_filename).generic_string();
  _file.open(snippets_filename.generic_string(),
             std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
  if (!_file)
  {
    BOOST_THROW_EXCEPTION(core::file_open_error() << core::context_info(
                            snippets_filename.generic_string()));
  }
  _file << R"(<?xml version="1.0" encoding="UTF-8"?>)" br;
  _file << "<!-- merge content of this file with "
           "~/.config/QtProject/qtcreator/snippets/snippets.xml -->" br;
  _file << "<snippets>" br;
  for (const auto& type : _types)
  {
    if (type.category != type_category::struct_type)
      continue;

    _file << R"(  <snippet group="C++" trigger="_)"
          << boost::replace_all_copy(type.name, "_", "")
          << R"(" id="" complement="" removed="false" modified="false">)"
          << "vk::" << type.new_name << "(";
    bool first_parameter = true;
    for (const auto& member : type.members)
    {
      if (member.read_only)
        continue;
      if (first_parameter)
        first_parameter = false;
      else
        _file << ",";
      _file << br "  /* " << member.new_name << " */ $" << member.new_name
            << "$";
    }
    _file << ")" br "</snippet>" br;
  }

  _file << "</snippets>" br;
  _file.close();
}

void application::print_graph(const std::string& command_name,
                              const type_descriptor& type, bool input)
{
  if (type.category == type_category::handle)
  {
    if (_printed.emplace(&type).second)
    {
      _file << indent << "\"" << type.name
            << "\" [shape=box, style=filled, color=cyan];" br;
    }
  }

  switch (type.category)
  {
  case type_category::handle:
  {
    if (input)
    {
      _file << indent << "\"" << type.name << "\" -> \"" << command_name
            << "\";" br;
    }
    else
    {
      _file << indent << "\"" << command_name << "\" -> \"" << type.name
            << "\";" br;
    }
    break;
  }
  case type_category::struct_type:
  {
    for (const auto& member : type.members)
    {
      print_graph(command_name, *member.type,
                  input ||
                    !(member.flags &
                      (member_type_flags{0} | member_type_flag::is_pointer |
                       member_type_flag::is_pointer_to_pointer)) ||
                    member.flags & member_type_flag::is_const ||
                    member.flags & member_type_flag::is_optional);
    }
    break;
  }
  default:
    break;
  }
}

void application::print_graph(const command_descriptor& command)
{
  _file << indent << "\"" << command.prototype.name
        << "\" [shape=box, style=filled, color=greenyellow];" br;

  for (const auto& parameter : command.parameters)
  {
    print_graph(
      command.prototype.name, *parameter.type,
      !(parameter.flags & (member_type_flags{0} | member_type_flag::is_pointer |
                           member_type_flag::is_pointer_to_pointer)) ||
        parameter.flags & member_type_flag::is_const ||
        parameter.flags & member_type_flag::is_optional);
  }
}

void application::print_graph(const extension_descriptor& extension)
{
  if (!_printed.emplace(&extension).second)
    return;

  for (const auto& command : extension.commands)
  {
    BOOST_ASSERT(command);
    print_graph(*command);
  }
}

type_descriptor* application::type_by_name(const std::string& name)
{
  for (auto& type : _types)
  {
    if (type.name == name)
      return &type;
  }
  return nullptr;
}

command_descriptor* application::command_by_name(const std::string& name)
{
  for (auto& command : _commands)
  {
    if (command.prototype.name == name)
      return &command;
  }
  return nullptr;
}
}
