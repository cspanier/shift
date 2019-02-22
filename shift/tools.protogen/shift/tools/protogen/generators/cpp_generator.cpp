#include "shift/tools/protogen/generators/cpp_generator.hpp"
#include "shift/tools/protogen/file_writer.hpp"
#include <shift/core/string_util.hpp>
#include <shift/core/stream_util.hpp>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <filesystem>
#include <fstream>

namespace shift::tools::protogen::generators
{
// using core::dec_indent;
// using core::inc_indent;
// using core::indent;
// using core::indent_width;
// using core::tab;

//#if defined(SHIFT_PLATFORM_WINDOWS)
//#define br "\r\n"
//#else
//#define br "\n"
//#endif
//#define br2 br br

// static const char* auto_generated_file_warning =
//  "////////////////////////////////////////////////////////////////" br
//  "// WARNING! Automatically generated file.                     //" br
//  "// Do not touch anything or your changes will be overwritten! //" br
//  "////////////////////////////////////////////////////////////////" br;

// bool node_comparator::operator()(const node* lhs, const node* rhs) const
//{
//  if (!rhs)
//    return true;
//  if (!lhs)
//    return false;
//  return cpp_generator::relative_name(*lhs, nullptr) <
//         cpp_generator::relative_name(*rhs, nullptr);
//}

// static file_writer& operator<<(file_writer& stream, const namescope&
// namescope)
//{
//  if (!namescope.has_attribute("cpp_name"))
//    return stream;
//  if (namescope.parent)
//    stream << *namescope.parent;
//  stream << "::" << namescope.attribute<std::string>("cpp_name");
//  return stream;
//}

// static file_writer& operator<<(file_writer& stream, const alias& alias)
//{
//  if (alias.parent)
//    stream << *alias.parent;
//  stream << "::" << alias.attribute<std::string>("cpp_name");
//  return stream;
//}

// static file_writer& operator<<(file_writer& stream,
//                               const enumeration& enumeration)
//{
//  if (enumeration.parent)
//    stream << *enumeration.parent;
//  stream << "::" << enumeration.attribute<std::string>("cpp_name");
//  return stream;
//}

// static file_writer& operator<<(file_writer& stream, const message& message)
//{
//  auto* parent = message.parent;
//  // Since C++ cannot forward declare inner types, messages defined within
//  // interfaces are moved to the surrounding name-scope. In this case we do
//  // not want to include the interface name in the name path as we already
//  // merged that name into the cs_name message attribute.
//  if (parent && parent->has_attribute("is_interface"))
//    parent = parent->parent;
//  if (parent)
//    stream << *parent;
//  stream << "::" << message.attribute<std::string>("cpp_name");
//  return stream;
//}

// static file_writer& operator<<(file_writer& stream,
//                               const type_reference& type_reference)
//{
//  if (auto* type = type_reference.as_built_in_type())
//  {
//    switch (*type)
//    {
//    case built_in_type::undefined:
//      stream << "<undefined built-in type>";
//      break;

//    case built_in_type::boolean:
//      stream << "bool";
//      break;

//    case built_in_type::char8:
//      stream << "char";
//      break;

//    case built_in_type::char16:
//      stream << "char16_t";
//      break;

//    case built_in_type::char32:
//      stream << "char32_t";
//      break;

//    case built_in_type::int8:
//      stream << "std::int8_t";
//      break;

//    case built_in_type::uint8:
//      stream << "std::uint8_t";
//      break;

//    case built_in_type::int16:
//      stream << "std::int16_t";
//      break;

//    case built_in_type::uint16:
//      stream << "std::uint16_t";
//      break;

//    case built_in_type::int32:
//      stream << "std::int32_t";
//      break;

//    case built_in_type::uint32:
//      stream << "std::uint32_t";
//      break;

//    case built_in_type::int64:
//      stream << "std::int64_t";
//      break;

//    case built_in_type::uint64:
//      stream << "std::uint64_t";
//      break;

//    case built_in_type::datetime:
//      stream << "std::chrono::system_clock::time_point";
//      break;

//    case built_in_type::float32:
//      stream << "float";
//      break;

//    case built_in_type::float64:
//      stream << "double";
//      break;

//    case built_in_type::string:
//      stream << "std::string";
//      break;

//    case built_in_type::raw_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "::shift::serialization::raw_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::unique_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "std::unique_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::shared_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "std::shared_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::weak_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "std::weak_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::group_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "::shift::core::group_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::tuple:
//    {
//      bool firstArgument = true;
//      stream << "std::tuple<";
//      for (const auto& template_argument : type_reference.arguments)
//      {
//        const auto* argument_type =
//          std::get_if<proto::type_reference>(&template_argument);
//        BOOST_ASSERT(argument_type);
//        if (firstArgument)
//          firstArgument = false;
//        else
//          stream << ", ";
//        stream << *argument_type;
//      }
//      stream << ">";
//      break;
//    }

//    case built_in_type::array:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() >= 1);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = nullptr;
//      if (type_reference.arguments.size() >= 2)
//        size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      if (size)
//      {
//        stream << "std::array<" << *value_type << ", " << std::dec << *size
//               << ">";
//      }
//      else
//      {
//        stream << "std::vector<" << *value_type << ">";
//      }
//      break;
//    }

//    case built_in_type::list:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      stream << "std::vector<" << *value_type << ">";
//      break;
//    }

//    case built_in_type::vector:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type && size);
//      if (!value_type || !size)
//        break;
//      stream << "::shift::math::vector<" << std::dec << *size << ", "
//             << *value_type << ">";
//      break;
//    }

//    case built_in_type::set:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      stream << "std::set<" << *value_type << ">";
//      break;
//    }

//    case built_in_type::matrix:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type && size);
//      if (!value_type || !size)
//        break;
//      stream << "::shift::math::matrix<" << std::dec << *size << ", "
//             << std::dec << *size << ", " << *value_type << ">";
//      break;
//    }

//    case built_in_type::map:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const auto* key_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[1]);
//      BOOST_ASSERT(key_type && value_type);
//      if (!key_type || !value_type)
//        break;
//      stream << "std::map<" << *key_type << ", " << *value_type << ">";
//      break;
//    }

//    case built_in_type::variant:
//    {
//      stream << "std::variant<";
//      bool firstType = true;
//      for (auto& template_argument : type_reference.arguments)
//      {
//        const auto* argumentReference =
//          std::get_if<proto::type_reference>(&template_argument);
//        BOOST_ASSERT(argumentReference);
//        if (!argumentReference)
//          break;
//        if (firstType)
//          firstType = false;
//        else
//          stream << ", ";
//        stream << *argumentReference;
//      }
//      stream << ">";
//      break;
//    }

//    case built_in_type::bitfield:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const auto* enumType =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(enumType);
//      auto* enumeration = enumType->as_enumeration();
//      if (!enumType || !enumeration)
//        break;
//      stream << "::shift::core::bit_field<" << *enumType << ">";
//      break;
//    }

//    default:
//      stream << "<unknown built-in type>";
//      break;
//    }
//  }
//  else if (auto* alias = type_reference.as_alias())
//    stream << *alias;
//  else if (auto* enumeration = type_reference.as_enumeration())
//    stream << *enumeration;
//  else if (auto* message = type_reference.as_message())
//    stream << *message;
//  else
//    stream << "<unknown-type>";

//  return stream;
//}

// static file_writer& operator<<(file_writer& stream, const field& field)
//{
//  stream << field.reference << " " <<
//  field.attribute<std::string>("cpp_name"); return stream;
//}

// bool cpp_generator::active() const
//{
//  return !cpp_source_output_path.empty() && !cpp_include_output_path.empty();
//}

// std::vector<std::filesystem::path> cpp_generator::output_paths() const
//{
//  return {cpp_source_output_path, cpp_include_output_path};
//}

// bool cpp_generator::generate(namescope& root_scope, namescope&
// /*limit_scope*/)
//{
//  if (!cpp_naming_convention_name.empty())
//  {
//    if (cpp_naming_convention_name == "lower-delimited")
//      _naming_convention = naming_convention::lower_delimited;
//    else if (cpp_naming_convention_name == "camel-case")
//      _naming_convention = naming_convention::camel_case;
//    else
//    {
//      /// ToDo: report error.
//      return false;
//    }
//  }

//  _use_services = false;
//  preprocess(root_scope);
//  gather_nodes(root_scope);
//  for (auto node = _nodes.begin(); node != _nodes.end(); ++node)
//    gather_dependencies(**node);

//  file_writer fileHeader;
//  file_writer fileSource;
//  fileHeader.global_scope(root_scope);
//  fileSource.global_scope(root_scope);
//  fileHeader.on_enter_namescope = fileSource.on_enter_namescope =
//    [](std::ostream& stream, const namescope& scope) {
//      if (scope.has_attribute("cpp_name") &&
//          !scope.has_attribute("is_interface"))
//      {
//        stream << indent << "namespace "
//               << scope.attribute<std::string>("cpp_name") << br;
//        stream << indent << "{" br << inc_indent;
//      }
//    };
//  fileHeader.on_leave_namescope = fileSource.on_leave_namescope =
//    [](std::ostream& stream, const namescope& scope) {
//      if (scope.has_attribute("cpp_name") &&
//          !scope.has_attribute("is_interface"))
//      {
//        stream << dec_indent << indent << "}" br;
//      }
//    };
//  _header = &fileHeader;
//  _source = &fileSource;

//  for (auto groupBegin = _node_groups.begin(), groupEnd =
//  _node_groups.begin();
//       groupBegin != _node_groups.end(); groupBegin = groupEnd)
//  {
//    const auto& groupName = groupBegin->first;
//    groupEnd = _node_groups.upper_bound(groupName);

//    // Collect all declarations and definitions which are required in the
//    // current header and source file.
//    node_set groupDeclarations;
//    std::set<std::string> includeFiles;
//    node_set groupDefinitions;
//    for (auto nodeIter = groupBegin; nodeIter != groupEnd; ++nodeIter)
//    {
//      const auto* node = nodeIter->second.first;
//      bool defineNode = nodeIter->second.second;
//      if (defineNode)
//      {
//        groupDefinitions.insert(node);

//        auto declarations = _declarations.find(node);
//        if (declarations != _declarations.end())
//        {
//          for (const auto* declaration : declarations->second)
//            groupDeclarations.insert(declaration);
//        }
//        auto node_definitions = _definitions.find(node);
//        if (node_definitions != _definitions.end())
//        {
//          for (const auto* definition : node_definitions->second)
//          {
//            if (groupName ==
//                definition->attribute<std::string>("cpp_def_group_name"))
//            {
//              groupDefinitions.insert(definition);
//            }
//            else
//            {
//              includeFiles.insert(
//                definition->attribute<std::string>("cpp_def_group_name"));
//            }
//          }
//        }
//      }
//      else
//        groupDeclarations.insert(node);
//    }

//    BOOST_ASSERT(&root_scope == &_header->current_scope());
//    std::string includeGuard = core::to_upper(groupName) + "_H";
//    boost::replace_all(includeGuard, "/", "_");
//    boost::replace_all(includeGuard, ".", "_");

//    _use_header = false;
//    _use_source = false;
//    *_header << auto_generated_file_warning <<
//    indent_width(static_cast<int>(translator::cpp_indent_width));
//    *_header << "#ifndef " << includeGuard << br;
//    *_header << "#define " << includeGuard << br2;

//    *_header << "#include <cstdint>" br;
//    *_header << "#include <chrono>" br;
//    *_header << "#include <memory>" br;
//    *_header << "#include <variant>" br;
//    *_header << "#include <array>" br;
//    *_header << "#include <vector>" br;
//    *_header << "#include <set>" br;
//    *_header << "#include <shift/core/bit_field.hpp>" br;
//    *_header << "#include <shift/math/vector.hpp>" br;
//    *_header << "#include <shift/math/matrix.hpp>" br;
//    *_header << "#include <shift/math/serialization.hpp>" br;
//    *_header << "#include <shift/serialization/message.hpp>" br;
//    *_header << "#include <shift/serialization/all.hpp>" br;
//    *_header << "#include <shift/serialization/compact/all.hpp>" br;
//#if defined(USE_REFLECTIONS)
//    *_header << "#include \"shift/proto/types.h\"" br;
//#endif
//    if (_use_services)
//      *_header << "#include <shift/service/basic_service.hpp>" br;

//    *_source << auto_generated_file_warning <<
//    indent_width(static_cast<int>(translator::cpp_indent_width));
//    *_source << "#include \"" << groupName << ".h\"" br;

//    // Definitions within this group need to be sorted topologically.
//    std::vector<const proto::node*> sortedGroupDefinitions;
//    node_set visitedGroupDefinitions;
//    std::function<void(const proto::node*)> visit =
//      [&](const proto::node* item) {
//        if (visitedGroupDefinitions.find(item) ==
//        visitedGroupDefinitions.end())
//        {
//          visitedGroupDefinitions.insert(item);
//          for (const auto* dependency : _definitions[item])
//          {
//            if (groupDefinitions.find(dependency) != groupDefinitions.end())
//              visit(dependency);
//          }
//          sortedGroupDefinitions.push_back(item);
//        }
//      };
//    for (const auto* node : groupDefinitions)
//      visit(node);

//    for (auto includeFile : includeFiles)
//      *_header << indent << "#include \"" << includeFile << ".h\"" br;
//    *_header << br;
//    for (const auto* node : groupDeclarations)
//    {
//      write_declaration(*node);
//      *_source << indent << "#include \""
//               << node->attribute<std::string>("cpp_def_group_name")
//               << ".h\"" br;
//    }
//    *_header << br;
//#if defined(USE_REFLECTIONS)
//    *_source << "#include \"shift/proto/service.h\"" br;
//    *_source << "#include \"shift/proto/interface.h\"" br;
//    *_source << "#include \"shift/proto/message.h\"" br;
//    *_source << "#include \"shift/proto/messagefield.h\"" br;
//    *_source << "#include \"shift/proto/enumeration.h\"" br;
//    *_source << "#include \"shift/proto/enumerant.h\"" br;
//    *_source << "#include \"shift/proto/attribute.h\"" br;
//    *_source << "#include \"shift/proto/type_reference.h\"" br;
//#endif
//    *_source << "#include <shift/serialization/archive.hpp>" br;
//    *_source << "#include <shift/serialization/all.hpp>" br;
//    *_source << "#include <functional>" br;
//    *_source << "#include <iomanip>" br2;

//    for (const auto* node : sortedGroupDefinitions)
//      write_definition(*node);

//    _header->switch_namescope(root_scope);
//    _source->switch_namescope(root_scope);
//    *_header << indent << br2 "#endif" br;
//    if (_use_header)
//      _header->write(cpp_include_output_path / (groupName + ".h"),
//                     cpp_clang_format);
//    if (_use_source)
//    {
//      _source->write(cpp_source_output_path / (groupName + ".cpp"),
//                     cpp_clang_format);
//    }

//    _header->clear();
//    _source->clear();
//  }

//  std::string includeGuardPrefix = core::to_upper(cpp_namespace);
//  boost::replace_all(includeGuardPrefix, "::", "_");
//  std::string includePath = cpp_folder.empty() ? cpp_namespace : cpp_folder;
//  boost::replace_all(includePath, "::", "/");

//  _header = nullptr;
//  _source = nullptr;
//  return true;
//}

// std::string cpp_generator::relative_name(const node& node,
//                                         const namescope* current_scope,
//                                         std::string delimiter)
//{
//  static auto name_lookup = [](const proto::node& node) -> std::string {
//    if (node.has_attribute("cpp_name"))
//      return node.attribute<std::string>("cpp_name");
//    else
//      return node.name;
//  };
//  auto relative_path =
//    base_generator::relative_path(name_lookup, &node, current_scope);
//  // Since C++ cannot forward declare inner types, messages defined within
//  // interfaces are moved to the surrounding name-scope. In this case we do
//  // not want to include the interface name in the name path as we already
//  // merged that name into the cs_name message attribute.
//  if (node.parent && node.parent->has_attribute("is_interface") &&
//      relative_path.size() > 1)
//  {
//    relative_path.erase(relative_path.begin() + relative_path.size() - 2);
//  }
//  return core::implode(relative_path, delimiter);
//}

// void cpp_generator::preprocess(namescope& scope)
//{
//  if (!scope.name.empty())
//  {
//    if (_naming_convention == naming_convention::lower_delimited)
//    {
//      scope.attribute("cpp_name", delimiter_case(scope.name),
//                      attribute_assignment::keep);
//    }
//    else if (_naming_convention == naming_convention::camel_case)
//    {
//      scope.attribute("cpp_name", camel_case(scope.name),
//                      attribute_assignment::keep);
//    }
//  }

//  for (auto* alias : scope.aliases)
//  {
//    if (_naming_convention == naming_convention::lower_delimited)
//    {
//      alias->attribute("cpp_name", delimiter_case(alias->name),
//                       attribute_assignment::keep);
//    }
//    else if (_naming_convention == naming_convention::camel_case)
//    {
//      alias->attribute("cpp_name", pascal_case(alias->name),
//                       attribute_assignment::keep);
//    }
//    alias->attribute("cpp_def_group_name", def_group_name(*alias));
//    alias->attribute("cpp_decl_group_name", decl_group_name(*alias));
//  }
//  for (auto* enumeration : scope.enumerations)
//  {
//    if (_naming_convention == naming_convention::lower_delimited)
//    {
//      enumeration->attribute("cpp_name", delimiter_case(enumeration->name),
//                             attribute_assignment::keep);
//    }
//    else if (_naming_convention == naming_convention::camel_case)
//    {
//      enumeration->attribute("cpp_name", pascal_case(enumeration->name),
//                             attribute_assignment::keep);
//    }
//    enumeration->attribute("cpp_def_group_name",
//    decl_group_name(*enumeration));
//    enumeration->attribute("cpp_decl_group_name",
//                           decl_group_name(*enumeration));
//    for (auto& member : enumeration->members)
//    {
//      if (_naming_convention == naming_convention::lower_delimited)
//      {
//        member.attribute("cpp_name", delimiter_case(member.name),
//                         attribute_assignment::keep);
//      }
//      else if (_naming_convention == naming_convention::camel_case)
//      {
//        member.attribute("cpp_name", camel_case(member.name),
//                         attribute_assignment::keep);
//      }
//    }
//  }
//  for (auto* message : scope.messages)
//  {
//    if (_naming_convention == naming_convention::lower_delimited)
//    {
//      message->attribute("cpp_name", delimiter_case(message->name),
//                         attribute_assignment::keep);
//    }
//    else if (_naming_convention == naming_convention::camel_case)
//    {
//      message->attribute("cpp_name", pascal_case(message->name),
//                         attribute_assignment::keep);
//    }
//    message->attribute("cpp_def_group_name", def_group_name(*message));
//    message->attribute("cpp_decl_group_name", decl_group_name(*message));
//    message->attribute("cpp_copy_constructable",
//                       is_copy_constructable(*message) ? 1 : 0);
//    for (auto& field : message->fields)
//    {
//      if (_naming_convention == naming_convention::lower_delimited)
//      {
//        field.attribute("cpp_name", delimiter_case(field.name),
//                        attribute_assignment::keep);
//      }
//      else if (_naming_convention == naming_convention::camel_case)
//      {
//        field.attribute("cpp_name", camel_case(field.name),
//                        attribute_assignment::keep);
//      }
//    }
//  }
//  for (auto* interface : scope.interfaces)
//  {
//    interface->attribute("cpp_name", interface->name,
//                         attribute_assignment::keep);
//    interface->attribute("cpp_def_group_name", def_group_name(*interface));
//    interface->attribute("cpp_decl_group_name", decl_group_name(*interface));
//    auto interfaceName =
//      interface->attribute<std::string>("cpp_name").substr(1);

//    for (auto* message : interface->messages)
//    {
//      // C++ does not support predeclarations of nested classes. Thus we
//      // move the message type to the surrounding name scope and merge the
//      // two names together to avoid name collisions.
//      message->attribute("cpp_name", interfaceName + "_" + message->name,
//                         attribute_assignment::keep);
//      message->attribute("cpp_def_group_name", def_group_name(*message));
//      message->attribute("cpp_decl_group_name", decl_group_name(*message));
//      message->attribute("cppEventName", "on" + message->name,
//                         attribute_assignment::keep);
//      for (auto& field : message->fields)
//        field.attribute("cpp_name", field.name, attribute_assignment::keep);
//    }
//  }
//  _use_services |= !scope.services.empty();
//  for (auto* service : scope.services)
//  {
//    service->attribute("cpp_name", service->name, attribute_assignment::keep);
//    service->attribute("cpp_def_group_name", def_group_name(*service));
//    service->attribute("cpp_decl_group_name", decl_group_name(*service));
//  }

//  for (auto sub_namescope : scope.namescopes)
//    preprocess(*sub_namescope);
//}

// bool cpp_generator::is_copy_constructable(const message& message)
//{
//  for (const auto& field : message.fields)
//  {
//    if (!field.has_attribute("ignore") &&
//        !is_copy_constructable(field.reference))
//    {
//      return false;
//    }
//  }
//  return true;
//}

// bool cpp_generator::is_copy_constructable(const type_reference& reference)
//{
//  if (const auto* alias = reference.as_alias())
//    return is_copy_constructable(alias->reference);
//  else if (const auto* type = reference.as_built_in_type())
//  {
//    switch (*type)
//    {
//    case built_in_type::raw_ptr:
//    case built_in_type::unique_ptr:
//      return false;

//    case built_in_type::tuple:
//    case built_in_type::variant:
//      for (const auto& template_argument : reference.arguments)
//      {
//        const auto* argument_type =
//          std::get_if<proto::type_reference>(&template_argument);
//        if (!argument_type || !is_copy_constructable(*argument_type))
//          return false;
//      }
//      break;

//    case built_in_type::array:
//    case built_in_type::list:
//    case built_in_type::vector:
//    case built_in_type::set:
//    case built_in_type::matrix:
//    {
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&reference.arguments[0]);
//      if (!value_type || !is_copy_constructable(*value_type))
//        return false;
//      break;
//    }

//    case built_in_type::map:
//    {
//      const auto* key_type =
//        std::get_if<proto::type_reference>(&reference.arguments[0]);
//      const auto* value_type =
//        std::get_if<proto::type_reference>(&reference.arguments[1]);
//      if (!key_type || !is_copy_constructable(*key_type) || !value_type ||
//          !is_copy_constructable(*value_type))
//      {
//        return false;
//      }
//      break;
//    }

//    case built_in_type::bitfield:
//    default:
//      break;
//    }
//  }
//  else if (const auto* message = reference.as_message())
//    return is_copy_constructable(*message);

//  return true;
//}

// void cpp_generator::write_copy_ctor_or_copy_assignment_body(
//  const message& message)
//{
//  for (const auto& field : message.fields)
//  {
//    *_source << indent << field.attribute<std::string>("cpp_name")
//             << " = other." << field.attribute<std::string>("cpp_name")
//             << ";" br;
//  }
//}

// void cpp_generator::write_move_ctor_or_move_assignment_body(
//  const message& message)
//{
//  for (const auto& field : message.fields)
//  {
//    *_source << indent << field.attribute<std::string>("cpp_name")
//             << " = std::move(other."
//             << field.attribute<std::string>("cpp_name") << ");" br;
//  }
//}

// void cpp_generator::gather_nodes(const namescope& scope)
//{
//  for (const auto* alias : scope.aliases)
//    gather_nodes(*alias);
//  for (const auto* enumeration : scope.enumerations)
//    gather_nodes(*enumeration);
//  for (const auto* message : scope.messages)
//    gather_nodes(*message);
//  for (const auto* interface : scope.interfaces)
//  {
//    gather_nodes(*interface);
//    for (const auto* message : interface->messages)
//      gather_nodes(*message);
//  }
//  for (const auto* service : scope.services)
//    gather_nodes(*service);
//  for (const auto sub_namescope : scope.namescopes)
//    gather_nodes(*sub_namescope);
//}

// void cpp_generator::gather_nodes(const type_reference& reference)
//{
//  if (const auto* built_in_type = reference.as_built_in_type())
//  {
//    for (const auto& template_argument : reference.arguments)
//    {
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&template_argument);
//      if (!argument_type)
//        continue;
//      gather_nodes(*argument_type);
//    }
//  }
//  else if (const auto* alias = reference.as_alias())
//    gather_nodes(*alias);
//  else if (const auto* enumeration = reference.as_enumeration())
//    gather_nodes(*enumeration);
//  else if (const auto* message = reference.as_message())
//    gather_nodes(*message);
//}

// void cpp_generator::gather_nodes(const alias& alias)
//{
//  if (_nodes.find(&alias) != _nodes.end())
//    return;
//  _nodes.insert(&alias);
//  _node_groups.insert(
//    std::make_pair(alias.attribute<std::string>("cpp_def_group_name"),
//                   std::make_pair(&alias, true)));
//  gather_nodes(alias.reference);
//}

// void cpp_generator::gather_nodes(const enumeration& enumeration)
//{
//  if (_nodes.find(&enumeration) != _nodes.end())
//    return;
//  _nodes.insert(&enumeration);
//  _node_groups.insert(
//    std::make_pair(enumeration.attribute<std::string>("cpp_def_group_name"),
//                   std::make_pair(&enumeration, true)));
//  gather_nodes(enumeration.base);
//}

// void cpp_generator::gather_nodes(const message& message)
//{
//  if (_nodes.find(&message) != _nodes.end())
//    return;
//  _nodes.insert(&message);
//  _node_groups.insert(
//    std::make_pair(message.attribute<std::string>("cpp_decl_group_name"),
//                   std::make_pair(&message, false)));
//  _node_groups.insert(
//    std::make_pair(message.attribute<std::string>("cpp_def_group_name"),
//                   std::make_pair(&message, true)));
//  if (message.base)
//    gather_nodes(*message.base);
//  for (const auto& field : message.fields)
//    gather_nodes(field.reference);
//}

// void cpp_generator::gather_nodes(const interface& interface)
//{
//  if (_nodes.find(&interface) != _nodes.end())
//    return;
//  _nodes.insert(&interface);
//  _node_groups.insert(
//    std::make_pair(interface.attribute<std::string>("cpp_decl_group_name"),
//                   std::make_pair(&interface, false)));
//  _node_groups.insert(
//    std::make_pair(interface.attribute<std::string>("cpp_def_group_name"),
//                   std::make_pair(&interface, true)));
//  gather_nodes(static_cast<const namescope&>(interface));
//}

// void cpp_generator::gather_nodes(const service& service)
//{
//  if (_nodes.find(&service) != _nodes.end())
//    return;
//  _nodes.insert(&service);
//  _node_groups.insert(
//    std::make_pair(service.attribute<std::string>("cpp_decl_group_name"),
//                   std::make_pair(&service, false)));
//  _node_groups.insert(
//    std::make_pair(service.attribute<std::string>("cpp_def_group_name"),
//                   std::make_pair(&service, true)));
//  gather_nodes(*service.service_interface);
//  gather_nodes(*service.callback_interface);
//}

// void cpp_generator::gather_dependencies(const node& target)
//{
//  if (const auto* alias = dynamic_cast<const proto::alias*>(&target))
//  {
//    gather_dependencies(target, *alias, _declarations[&target],
//                        _definitions[&target], true);
//  }
//  else if (const auto* enumeration =
//             dynamic_cast<const proto::enumeration*>(&target))
//  {
//    gather_dependencies(target, *enumeration, _declarations[&target],
//                        _definitions[&target], true);
//  }
//  else if (const auto* message = dynamic_cast<const proto::message*>(&target))
//  {
//    gather_dependencies(target, *message, _declarations[&target],
//                        _definitions[&target], true);
//  }
//  else if (const auto* interface =
//             dynamic_cast<const proto::interface*>(&target))
//  {
//    gather_dependencies(target, *interface, _declarations[&target],
//                        _definitions[&target], true);
//  }
//  else if (const auto* service = dynamic_cast<const proto::service*>(&target))
//  {
//    gather_dependencies(target, *service, _declarations[&target],
//                        _definitions[&target], true);
//  }
//  _declarations[&target].erase(&target);
//  _definitions[&target].erase(&target);
//}

// void cpp_generator::gather_dependencies(const node& target,
//                                        const type_reference& reference,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if (const auto* type = reference.as_built_in_type())
//  {
//    switch (*type)
//    {
//    case built_in_type::raw_ptr:
//    case built_in_type::unique_ptr:
//    case built_in_type::shared_ptr:
//    case built_in_type::weak_ptr:
//    case built_in_type::group_ptr:
//    {
//      BOOST_ASSERT(reference.arguments.size() == 1);
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      if (!argument_type)
//        break;
//      gather_dependencies(target, *argument_type, required_declarations,
//                          required_definitions, false);
//      break;
//    }

//    default:
//    {
//      for (const auto& template_argument : reference.arguments)
//      {
//        const auto* argument_type =
//          std::get_if<proto::type_reference>(&template_argument);
//        if (!argument_type)
//          continue;
//        gather_dependencies(target, *argument_type, required_declarations,
//                            required_definitions, definition);
//      }
//      break;
//    }
//    }
//  }
//  else if (const auto* alias = reference.as_alias())
//  {
//    gather_dependencies(target, *alias, required_declarations,
//                        required_definitions, definition);
//  }
//  else if (const auto* enumeration = reference.as_enumeration())
//  {
//    gather_dependencies(target, *enumeration, required_declarations,
//                        required_definitions, definition);
//  }
//  else if (const auto* message = reference.as_message())
//  {
//    gather_dependencies(target, *message, required_declarations,
//                        required_definitions, definition);
//  }
//}

// void cpp_generator::gather_dependencies(const node& target, const alias&
// alias,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if ((definition && !required_definitions.insert(&alias).second) ||
//      (!definition && !required_declarations.insert(&alias).second))
//  {
//    return;
//  }
//  gather_dependencies(target, alias.reference, required_declarations,
//                      required_definitions, true);
//}

// void cpp_generator::gather_dependencies(const node& target,
//                                        const enumeration& enumeration,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if ((definition && !required_definitions.insert(&enumeration).second) ||
//      (!definition && !required_declarations.insert(&enumeration).second))
//  {
//    return;
//  }
//  gather_dependencies(target, enumeration.base, required_declarations,
//                      required_definitions, true);
//}

// void cpp_generator::gather_dependencies(const node& target,
//                                        const message& message,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if ((definition && !required_definitions.insert(&message).second) ||
//      (!definition && !required_declarations.insert(&message).second))
//  {
//    return;
//  }
//  if (definition && message.base)
//  {
//    gather_dependencies(target, *message.base, required_declarations,
//                        required_definitions, true);
//  }
//  for (const auto& field : message.fields)
//  {
//    gather_dependencies(target, field.reference, required_declarations,
//                        required_definitions, definition);
//  }
//}

// void cpp_generator::gather_dependencies(const node& target,
//                                        const interface& interface,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if ((definition && !required_definitions.insert(&interface).second) ||
//      (!definition && !required_declarations.insert(&interface).second))
//  {
//    return;
//  }
//  if (definition)
//  {
//    for (const auto* message : interface.messages)
//    {
//      gather_dependencies(target, *message, required_declarations,
//                          required_definitions, true);
//    }
//  }
//}

// void cpp_generator::gather_dependencies(const node& target,
//                                        const service& service,
//                                        node_set& required_declarations,
//                                        node_set& required_definitions,
//                                        bool definition)
//{
//  if (&service != &target)
//  {
//    // Nothing should depend on a service definition.
//    BOOST_ASSERT(false);
//  }
//  if (definition)
//  {
//    gather_dependencies(target, *service.service_interface,
//                        required_declarations, required_definitions, true);
//    gather_dependencies(target, *service.callback_interface,
//                        required_declarations, required_definitions, true);
//  }
//}

// void cpp_generator::write_declaration(const node& node)
//{
//  if (const auto* alias = dynamic_cast<const proto::alias*>(&node))
//    write_declaration(*alias);
//  else if (const auto* enumeration =
//             dynamic_cast<const proto::enumeration*>(&node))
//  {
//    write_declaration(*enumeration);
//  }
//  else if (const auto* message = dynamic_cast<const proto::message*>(&node))
//    write_declaration(*message);
//  else if (const auto* interface = dynamic_cast<const
//  proto::interface*>(&node))
//  {
//    write_declaration(*interface);
//  }
//  else if (const auto* service = dynamic_cast<const proto::service*>(&node))
//    write_declaration(*service);
//}

// void cpp_generator::write_declaration(const alias& alias)
//{
//  // There is no pre-declaration syntax for alias types.
//  write_definition(alias);
//}

// void cpp_generator::write_declaration(const enumeration& enumeration)
//{
//  _header->switch_namescope(*enumeration.parent);
//  _use_header = true;
//  *_header << indent << "enum class "
//           << enumeration.attribute<std::string>("cpp_name") << " : "
//           << enumeration.base << ";" br;
//}

// void cpp_generator::write_declaration(const message& message)
//{
//  _header->switch_namescope(*message.parent);
//  _use_header = true;
//  *_header << indent << "struct " <<
//  message.attribute<std::string>("cpp_name")
//           << ";" << br;
//}

// void cpp_generator::write_declaration(const interface& /*interface*/)
//{
//  // Interfaces are not translated in C++ code.
//}

// void cpp_generator::write_declaration(const service& service)
//{
//  _header->switch_namescope(*service.parent);
//  _use_header = true;
//  *_header << indent << "class " << service.attribute<std::string>("cpp_name")
//           << ";" br;
//}

// void cpp_generator::write_definition(const node& node)
//{
//  if (const auto* alias = dynamic_cast<const proto::alias*>(&node))
//    write_definition(*alias);
//  else if (const auto* enumeration =
//             dynamic_cast<const proto::enumeration*>(&node))
//  {
//    write_definition(*enumeration);
//  }
//  else if (const auto* message = dynamic_cast<const proto::message*>(&node))
//    write_definition(*message);
//  else if (const auto* interface = dynamic_cast<const
//  proto::interface*>(&node))
//  {
//    write_definition(*interface);
//  }
//  else if (const auto* service = dynamic_cast<const proto::service*>(&node))
//    write_definition(*service);
//}

// void cpp_generator::write_definition(const alias& alias)
//{
//  _header->switch_namescope(*alias.parent);
//  _definitions[&alias];
//  _use_header = true;
//  *_header << indent << "using " << alias.attribute<std::string>("cpp_name")
//           << " = " << alias.reference << ";" br;
//}

// void cpp_generator::write_definition(const enumeration& enumeration)
//{
//  _header->switch_namescope(*enumeration.parent);
//  _source->switch_namescope(*enumeration.parent);

//  _use_header = true;
//  _use_source = true;
//  *_header << indent << "enum class "
//           << enumeration.attribute<std::string>("cpp_name") << " : "
//           << enumeration.base << br;
//  *_header << indent << "{" << inc_indent;
//  std::int64_t nextAutoValue = 0;
//  bool firstMember = true;
//  for (auto member : enumeration.members)
//  {
//    if (firstMember)
//      firstMember = false;
//    else
//      *_header << ",";
//    *_header << br << indent << member.attribute<std::string>("cpp_name");
//    if (member.value != nextAutoValue)
//      *_header << " = 0x" << std::hex << member.value;
//    nextAutoValue = member.value + 1;
//  }
//  *_header << br << dec_indent << indent << "};" br2;

//  auto typeName = enumeration.attribute<std::string>("cpp_name");
//  *_header << indent
//           << "std::basic_ostream<char>& "
//              "operator<<(std::basic_ostream<char>& stream, "
//           << typeName << " value);" br2;
//  *_source << indent
//           << "std::basic_ostream<char>& "
//              "operator<<(std::basic_ostream<char>& stream, "
//           << typeName << " value)" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "switch (value)" br;
//  *_source << indent << "{" br;

//  using MemberMap = std::multimap<int64_t, std::string>;
//  MemberMap memberMap;
//  for (const auto& member : enumeration.members)
//  {
//    auto memberName = member.attribute<std::string>("cpp_name");
//    memberMap.insert(std::make_pair(member.value, memberName));
//  }

//  for (auto selector = memberMap.begin(); selector != memberMap.end();)
//  {
//    auto range = memberMap.equal_range(selector->first);
//    for (auto current = range.first; current != range.second; ++current)
//    {
//      *_source << indent << (current == range.first ? "case " : "// case ")
//               << typeName << "::" << current->second << ":" br;
//    }
//    *_source << inc_indent;

//    *_source << indent << "stream << \"";
//    for (auto current = range.first; current != range.second; ++current)
//    {
//      if (current != range.first)
//        *_source << "/";
//      *_source << current->second;
//    }
//    *_source << "\";" br;
//    *_source << indent << "break;" br << dec_indent;
//    selector = range.second;
//  }
//  *_source << indent << "default:" br << inc_indent;
//  *_source << indent << "stream << \"<unknown enum>\";" br;
//  *_source << indent << "break;" br << dec_indent;
//  *_source << indent << "}" br;
//  *_source << indent << "return stream;" br << dec_indent;
//  *_source << indent << "}" br2;
//}

// void cpp_generator::write_definition(const message& message)
//{
//  _header->switch_namescope(*message.parent);
//  _source->switch_namescope(*message.parent);
//  auto name = message.attribute<std::string>("cpp_name");

//  _use_header = true;
//  _use_source = true;
//  *_header << indent << "struct " << name << " : public ";
//  if (message.base)
//    *_header << *message.base << br;
//  else
//    *_header << "::shift::serialization::message" << br;

//  // Write message Uid.
//  *_header << indent << "{" br << inc_indent;
//  *_header << indent
//           << "static constexpr ::shift::serialization::message_uid_t uid =
//           0x"
//           << std::hex << std::setfill('0') << std::setw(8)
//           << message.attribute<std::uint32_t>("uid") << ";" br2;

//  // Write default constructors.
//  *_header << indent << "/// Default constructor." br;
//  *_header << indent << name << "();" br2;
//  *_source << indent << name << "::" << name << "()" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "_uid = uid;" br << dec_indent;
//  *_source << indent << "}" br2;

//  //// Write copy constructor.
//  *_header << indent << "/// Copy constructor." br;
//  if (message.attribute<std::uint32_t>("cpp_copy_constructable"))
//  {
//    *_header << indent << name << "(const " << name << "& other);" br2;
//    *_source << indent << name << "::" << name << "(const " << name
//             << "& other)" br;
//    *_source << indent << "{" br << inc_indent;
//    *_source << indent << "_uid = other._uid;" br;
//    /// ToDo: Members of potential base type are missing!
//    write_copy_ctor_or_copy_assignment_body(message);
//    *_source << dec_indent << indent << "}" br2;
//  }
//  else
//  {
//    *_header << indent << name << "(const " << name << "& other) = delete;"
//    br2;
//  }

//  // Write move constructor.
//  *_header << indent << "/// Move constructor." br;
//  *_header << indent << name << "(" << name << "&& other) noexcept;" br2;
//  *_source << indent << name << "::" << name << "(" << name
//           << "&& other) noexcept" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "_uid = other._uid;" br;
//  /// ToDo: Members of potential base type are missing!
//  write_move_ctor_or_move_assignment_body(message);
//  *_source << dec_indent << indent << "}" br2;

//  // Write custom constructor.
//  if (!message.fields.empty())
//  {
//    *_header << indent << "/// Constructor." br;
//    *_header << indent << name << "(";
//    *_source << indent << name << "::" << name << "(";
//    bool firstMember = true;
//    for (const auto& field : message.fields)
//    {
//      if (firstMember)
//        firstMember = false;
//      else
//      {
//        *_header << ", ";
//        *_source << ", ";
//      }
//      *_header << field;
//      *_source << field;
//    }
//    *_header << ");" br2;
//    *_source << ") :" br << inc_indent;
//    firstMember = true;
//    for (const auto& field : message.fields)
//    {
//      if (firstMember)
//        firstMember = false;
//      else
//        *_source << "," br;
//      *_source << indent << field.attribute<std::string>("cpp_name")
//               << "(std::move(" << field.attribute<std::string>("cpp_name")
//               << "))";
//    }
//    *_source << br << dec_indent;
//    *_source << indent << "{" br << inc_indent;
//    /// ToDo: Members of potential base type are missing!
//    *_source << indent << "_uid = uid;" br << dec_indent;
//    *_source << indent << "}" br2;
//  }

//  // Write copy assignment operator.
//  *_header << indent << "/// Copy assignment operator." br;
//  if (message.attribute<std::uint32_t>("cpp_copy_constructable"))
//  {
//    *_header << indent << name << "& operator=(const " << name
//             << "& other);" br2;
//    *_source << indent << name << "& " << name << "::operator="
//             << "(const " << name << "& other)" br;
//    *_source << indent << "{" br << inc_indent;
//    *_source << indent << "_uid = other._uid;" br;
//    /// ToDo: Members of potential base type are missing!
//    write_copy_ctor_or_copy_assignment_body(message);
//    *_source << indent << "return *this;" br << dec_indent;
//    *_source << indent << "}" br2;
//  }
//  else
//  {
//    *_header << indent << name << "& operator=(const " << name
//             << "& other) = delete;" br2;
//  }

//  // Write move assignment operator.
//  *_header << indent << "/// Move assignment operator." br;
//  *_header << indent << name << "& operator=(" << name
//           << "&& other) noexcept;" br2;
//  *_source << indent << name << "& " << name << "::operator="
//           << "(" << name << "&& other) noexcept" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "_uid = other._uid;" br;
//  /// ToDo: Members of potential base type are missing!
//  write_move_ctor_or_move_assignment_body(message);
//  *_source << indent << "return *this;" br << dec_indent;
//  *_source << indent << "}" br2;

//#if defined(USE_REFLECTIONS)
//  // struct meta_type_generator_base
//  //{
//  //  std::shared_ptr<shift::core::group> group()
//  //  {
//  //    static auto result = std::make_shared<shift::core::group>();
//  //    return result;
//  //  }
//  //};

//  // template<typename T>
//  // struct meta_type_generator;

//  // template<>
//  // struct meta_type_generator<Farewell> : private meta_type_generator_base
//  //{
//  //  shift::core::group_ptr<shift::proto::message> meta_type;
//  //};

//  // shift::core::group_ptr<shift::proto::message> Farewell::meta_type(
//  //  std::shared_ptr<shift::core::group> group)
//  //{
//  //  static meta_type_generator<Farewell> generator;
//  //  return generator.meta_type;

//  //  auto type = shift::core::make_group<shift::proto::message>(group);
//  //  // type->fields.push_back(shift::proto::MessageField({}, name, type);
//  //  return type;
//  //}

//  // Write meta_type method.
//  *_header << indent
//           << "/// Returns a type descriptor providing more "
//              "information than std::type_info." br;
//  *_header << indent
//           << "static shift::core::group_ptr<shift::proto::message> "
//              "meta_type(std::shared_ptr<shift::core::group> group);" br2;
//  *_source << indent << "shift::core::group_ptr<shift::proto::message> " <<
//  name
//           << "::meta_type(std::shared_ptr<shift::core::group> group)" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent
//           << "auto type = "
//              "shift::core::make_group<shift::proto::message>("
//              "group);" br;
//  for (const auto& field : message.fields)
//  {
//    *_header << indent << "type->fields.push_back();" br;  // field
//  }
//  *_source << indent << "return type;" br;
//  *_source << dec_indent << indent << "}" br2;
//#endif

//  // Write read_virtual methods.
//  *_header << indent << "///" br;
//  *_header << indent
//           << "virtual void "
//              "read_virtual(::shift::serialization::compact_"
//              "input_archive<boost::endian::order::big>& "
//              "archive) override;" br2;
//  *_source << indent << "void " << name
//           << "::read_virtual(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::big>& archive)" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "archive >> *this;" br;
//  *_source << dec_indent << indent << "};" br2;

//  *_header << indent << "///" br;
//  *_header << indent
//           << "virtual void "
//              "read_virtual(::shift::serialization::compact_"
//              "input_archive<boost::endian::order::little>& "
//              "archive) override;" br2;
//  *_source << indent << "void " << name
//           << "::read_virtual(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::little>& archive)" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "archive >> *this;" br;
//  *_source << dec_indent << indent << "};" br2;

//  // Write write_virtual methods.
//  *_header << indent << "///" br;
//  *_header << indent
//           << "virtual void "
//              "write_virtual(::shift::serialization::compact_"
//              "output_archive<boost::endian::order::big>& "
//              "archive) const override;" br;

//  *_source << indent << "void " << name
//           << "::write_virtual(::shift::serialization::compact_output_"
//              "archive<boost::endian::order::big>& archive) const" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "archive << *this;" br;
//  *_source << dec_indent << indent << "};" br2;

//  *_header << indent << "///" br;
//  *_header << indent
//           << "virtual void "
//              "write_virtual(::shift::serialization::compact_"
//              "output_archive<boost::endian::order::little>& "
//              "archive) const override;" br;

//  *_source << indent << "void " << name
//           << "::write_virtual(::shift::serialization::compact_output_"
//              "archive<boost::endian::order::little>& archive) const" br;
//  *_source << indent << "{" br << inc_indent;
//  *_source << indent << "archive << *this;" br;
//  *_source << dec_indent << indent << "};" br2;

//  // Write members.
//  if (!message.fields.empty())
//  {
//    *_header << br;
//    for (const auto& field : message.fields)
//      *_header << indent << field << ";" br;
//  }
//  *_header << dec_indent << indent << "};" br2;

//  *_header << indent
//           << "::shift::serialization::compact_input_archive<"
//              "boost::endian::order::big>& "
//              "operator>>(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::big>& archive, "
//           << message << "& message);" br;
//  *_header << indent
//           << "::shift::serialization::compact_output_archive<boost::"
//              "endian::order::big>& "
//              "operator<<(::shift::serialization::compact_output_archive<"
//              "boost::endian::order::big>& archive, const "
//           << message << "& message);" br;
//  *_header << indent
//           << "::shift::serialization::compact_input_archive<"
//              "boost::endian::order::little>& "
//              "operator>>(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::little>& archive, "
//           << message << "& message);" br;
//  *_header << indent
//           << "::shift::serialization::compact_output_archive<boost::"
//              "endian::order::little>& "
//              "operator<<(::shift::serialization::compact_output_archive<"
//              "boost::endian::order::little>& archive, const "
//           << message << "& message);" br;

//  *_source << indent
//           << "::shift::serialization::compact_input_archive<"
//              "boost::endian::order::big>& "
//              "operator>>(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::big>& archive, "
//           << message << "& message)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (message.base)
//  {
//    *_source << indent << "archive >> static_cast<" << *message.base
//             << "&>(message);" br;
//  }
//  for (auto& field : message.fields)
//  {
//    if (!field.has_attribute("ignore"))
//    {
//      *_source << indent << "archive >> message."
//               << field.attribute<std::string>("cpp_name") << ";" br;
//    }
//  }
//  *_source << indent << "return archive;" br;
//  *_source << dec_indent << indent << "}" br2;

//  *_source << indent
//           << "::shift::serialization::compact_output_archive<boost::"
//              "endian::order::big>& "
//              "operator<<(::shift::serialization::compact_output_archive<"
//              "boost::endian::order::big>& archive, const "
//           << message << "& message)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (message.base)
//  {
//    *_source << indent << "archive << static_cast<const " << *message.base
//             << "&>(message);" br;
//  }
//  for (auto& field : message.fields)
//  {
//    if (!field.has_attribute("ignore"))
//    {
//      *_source << indent << "archive << message."
//               << field.attribute<std::string>("cpp_name") << ";" br;
//    }
//  }
//  *_source << indent << "return archive;" br;
//  *_source << dec_indent << indent << "}" br2;

//  *_source << indent
//           << "::shift::serialization::compact_input_archive<"
//              "boost::endian::order::little>& "
//              "operator>>(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::little>& archive, "
//           << message << "& message)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (message.base)
//  {
//    *_source << indent << "archive >> static_cast<" << *message.base
//             << "&>(message);" br;
//  }
//  for (auto& field : message.fields)
//  {
//    if (!field.has_attribute("ignore"))
//    {
//      *_source << indent << "archive >> message."
//               << field.attribute<std::string>("cpp_name") << ";" br;
//    }
//  }
//  *_source << indent << "return archive;" br;
//  *_source << dec_indent << indent << "}" br2;

//  *_source << indent
//           << "::shift::serialization::compact_output_archive<boost::"
//              "endian::order::little>& "
//              "operator<<(::shift::serialization::compact_output_archive<"
//              "boost::endian::order::little>& archive, const "
//           << message << "& message)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (message.base)
//  {
//    *_source << indent << "archive << static_cast<const " << *message.base
//             << "&>(message);" br;
//  }
//  for (auto& field : message.fields)
//  {
//    if (!field.has_attribute("ignore"))
//    {
//      *_source << indent << "archive << message."
//               << field.attribute<std::string>("cpp_name") << ";" br;
//    }
//  }
//  *_source << indent << "return archive;" br;
//  *_source << dec_indent << indent << "}" br2;
//}

// void cpp_generator::write_definition(const interface& /*interface*/)
//{
//  _use_header = true;
//  // Interfaces are not translated in C++ code.
//}

// void cpp_generator::write_definition(const service& service)
//{
//  _header->switch_namescope(*service.parent);
//  _source->switch_namescope(*service.parent);

//  auto service_name = service.attribute<std::string>("cpp_name");
//  auto* service_interface = service.is_provider ? service.callback_interface
//                                                : service.service_interface;
//  auto* callback_interface = service.is_provider ? service.service_interface
//                                                 : service.callback_interface;

//  _use_header = true;
//  _use_source = true;
//  *_header << indent << "class " << service_name << " final :" br <<
//  inc_indent;
//  *_header << indent << "public ::shift::service::basic_service" br
//           << dec_indent;
//  *_header << indent << "{" br;
//  *_header << indent << "public:" br << inc_indent;

//  // Write constructor.
//  *_header << indent << "/// Constructor." br;
//  *_header << indent << service_name
//           << "(::shift::service::service_tag_t service_tag = 0, "
//              "::shift::service::service_protocol protocol = "
//              "::shift::service::service_protocol::tcp);" br2;
//  *_source << indent << service_name << "::" << service_name
//           << "(::shift::service::service_tag_t service_tag, "
//              "::shift::service::service_protocol protocol) :" br
//           << inc_indent;
//  *_source << indent << "::shift::service::basic_service"
//           << "(0x" << std::hex << std::setfill('0') << std::setw(8)
//           << service.attribute<std::uint32_t>("uid") << ", std::make_pair(0x"
//           << std::hex << std::setfill('0') << std::setw(8)
//           << service_interface->attribute<std::uint32_t>("uid") << ", 0x"
//           << std::hex << std::setfill('0') << std::setw(8)
//           << callback_interface->attribute<std::uint32_t>("uid")
//           << "), protocol, " << std::boolalpha << service.is_provider
//           << ", service_tag)" br << dec_indent;
//  *_source << indent << "{" br;
//  *_source << indent << "}" br2;

//  // Write destructor.
//  *_header << indent << "/// Destructor." br;
//  *_header << indent << "virtual ~" << service_name << "() override;" br2;
//  *_source << indent << service_name << "::~" << service_name << "()" br;
//  *_source << indent << "{" br;
//  *_source << indent << "}" br2;

//  // Override service::read_message.
//  *_header << indent
//           << "/// @see ::shift::service::basic_service::read_message." br;
//  *_header << indent
//           << "virtual void "
//              "read_message(::shift::serialization::compact_"
//              "input_archive<boost::endian::order::big>& "
//              "archive, ::shift::serialization::message_uid_t "
//              "uid) override;" br;
//  *_header << indent
//           << "/// @see ::shift::service::basic_service::read_message." br;
//  *_header << indent
//           << "virtual void "
//              "read_message(::shift::serialization::compact_"
//              "input_archive<boost::endian::order::little>& "
//              "archive, ::shift::serialization::message_uid_t "
//              "uid) override;" br;

//  *_source << indent << "void " << service_name
//           << "::read_message(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::big>& archive, "
//              "::shift::serialization::message_uid_t uid)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (!callback_interface->messages.empty())
//  {
//    *_source << indent << "switch (uid)" br;
//    *_source << indent << "{" br;
//    for (const auto* message : callback_interface->messages)
//    {
//      BOOST_ASSERT(message->parent == callback_interface);
//      *_source << indent << "case " << *message << "::uid:" br << inc_indent;
//      *_source << indent << "{" br << inc_indent;
//      *_source << indent << *message << " message;" br;
//      *_source << indent << "archive >> message;" br;
//      *_source << indent << "if ("
//               << message->attribute<std::string>("cppEventName") << ")" br;
//      *_source << indent << tab
//               << message->attribute<std::string>("cppEventName")
//               << "(*this, message);" br;
//      *_source << indent << "else if (on_receive)" br;
//      *_source << indent << tab << "on_receive(*this, message);" br;
//      *_source << indent << "return;" br;
//      *_source << dec_indent << indent << "}" br2;
//      *_source << dec_indent;
//    }
//    *_source << indent << "default:" br << inc_indent;
//  }
//  *_source << indent
//           << "shift::log::warning() << \"Received unknown "
//              "message uid 0x\" << std::hex << std::setfill('0') "
//              "<< std::setw(8) << uid << \".\";" br;
//  *_source << indent << "return;" br << dec_indent;
//  if (!callback_interface->messages.empty())
//    *_source << indent << "}" br;
//  *_source << dec_indent;
//  *_source << indent << "}" br2;

//  *_source << indent << "void " << service_name
//           << "::read_message(::shift::serialization::compact_input_"
//              "archive<boost::endian::order::little>& archive, "
//              "::shift::serialization::message_uid_t uid)" br;
//  *_source << indent << "{" br << inc_indent;
//  if (!callback_interface->messages.empty())
//  {
//    *_source << indent << "switch (uid)" br;
//    *_source << indent << "{" br;
//    for (const auto* message : callback_interface->messages)
//    {
//      BOOST_ASSERT(message->parent == callback_interface);
//      *_source << indent << "case " << *message << "::uid:" br << inc_indent;
//      *_source << indent << "{" br << inc_indent;
//      *_source << indent << *message << " message;" br;
//      *_source << indent << "archive >> message;" br;
//      *_source << indent << "if ("
//               << message->attribute<std::string>("cppEventName") << ")" br;
//      *_source << indent << tab
//               << message->attribute<std::string>("cppEventName")
//               << "(*this, message);" br;
//      *_source << indent << "else if (on_receive)" br;
//      *_source << indent << tab << "on_receive(*this, message);" br;
//      *_source << indent << "return;" br;
//      *_source << dec_indent << indent << "}" br2;
//      *_source << dec_indent;
//    }
//    *_source << indent << "default:" br << inc_indent;
//  }
//  *_source << indent
//           << "shift::log::warning() << \"Received unknown "
//              "message uid 0x\" << std::hex << std::setfill('0') "
//              "<< std::setw(8) << uid << \".\";" br;
//  *_source << indent << "return;" br << dec_indent;
//  if (!callback_interface->messages.empty())
//    *_source << indent << "}" br;
//  *_source << dec_indent;
//  *_source << indent << "}" br;

//  // Write callback functors.
//  if (!callback_interface->messages.empty())
//    *_header << br;
//  for (const auto* message : callback_interface->messages)
//  {
//    *_header << indent
//             << "std::function<void(::shift::service::basic_service& service,
//             "
//             << *message << "& message)> "
//             << message->attribute<std::string>("cppEventName") << ";" br;
//  }

//  *_header << dec_indent << indent << "};" br;
//}

// std::string cpp_generator::def_group_name(const node& node)
//{
//  return lower_case(relative_name(node, nullptr, "/"));
//}

// std::string cpp_generator::decl_group_name(const node& node)
//{
//  const auto* parent = node.parent;
//  while (dynamic_cast<const proto::interface*>(parent))
//    parent = parent->parent;
//  return lower_case(relative_name(*parent, nullptr, "/")) + "/types";
//}
}
