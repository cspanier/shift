#include "shift/tools/protogen/generators/dot_generator.h"
#include "shift/tools/protogen/file_writer.h"
#include <shift/core/string_util.h>
#include <shift/core/stream_util.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <fstream>

namespace shift::tools::protogen::generators
{
// using core::inc_indent;
// using core::dec_indent;
// using core::tab;
// using core::indent;
// using core::indent_width;

//#if defined(SHIFT_PLATFORM_WINDOWS)
//#define br "\r\n"
//#else
//#define br "\n"
//#endif
//#define br2 br br

// file_writer& operator<<(file_writer& stream, const node& any_node)
//{
//  if (any_node.parent && any_node.parent->has_attribute("dot_name"))
//    stream << static_cast<const node&>(*any_node.parent) << "::";
//  stream << any_node.attribute<std::string>("dot_name");
//  return stream;
//}

// std::ostream& operator<<(std::ostream& stream, const node& any_node)
//{
//  stream << any_node.attribute<std::string>("dot_name");
//  return stream;
//}

// static std::ostream& operator<<(std::ostream& stream, const built_in_type&
// type)
//{
//  switch (type)
//  {
//  case built_in_type::boolean:
//    stream << "bool";
//    break;

//  case built_in_type::char8:
//    stream << "char8";
//    break;

//  case built_in_type::char16:
//    stream << "char16";
//    break;

//  case built_in_type::char32:
//    stream << "char32";
//    break;

//  case built_in_type::int8:
//    stream << "int8";
//    break;

//  case built_in_type::uint8:
//    stream << "uint8";
//    break;

//  case built_in_type::int16:
//    stream << "int16";
//    break;

//  case built_in_type::uint16:
//    stream << "uint16";
//    break;

//  case built_in_type::int32:
//    stream << "int32";
//    break;

//  case built_in_type::uint32:
//    stream << "uint32";
//    break;

//  case built_in_type::int64:
//    stream << "int64";
//    break;

//  case built_in_type::uint64:
//    stream << "uint64";
//    break;

//  case built_in_type::datetime:
//    stream << "datetime";
//    break;

//  case built_in_type::float32:
//    stream << "float32";
//    break;

//  case built_in_type::float64:
//    stream << "float64";
//    break;

//  case built_in_type::string:
//    stream << "string";
//    break;

//  case built_in_type::raw_ptr:
//  case built_in_type::unique_ptr:
//  case built_in_type::shared_ptr:
//  case built_in_type::weak_ptr:
//  case built_in_type::group_ptr:
//  case built_in_type::tuple:
//  case built_in_type::array:
//  case built_in_type::vector:
//  case built_in_type::list:
//  case built_in_type::matrix:
//  case built_in_type::map:
//  case built_in_type::variant:
//    // NOP.
//    break;

//  case built_in_type::undefined:
//    stream << "<undefined built-in type>";
//    break;

//  default:
//    stream << "<unknown built-in type>";
//    break;
//  }
//  return stream;
//}

// static std::ostream& operator<<(std::ostream& stream,
//                                const type_reference& type_reference)
//{
//  if (auto* type = type_reference.as_built_in_type())
//  {
//    switch (*type)
//    {
//    case built_in_type::raw_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "raw_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::unique_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "unique_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::shared_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "shared_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::weak_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "weak_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::group_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "group_ptr<" << *argument_type << ">";
//      break;
//    }

//    case built_in_type::tuple:
//    {
//      bool firstArgument = true;
//      stream << "tuple<";
//      for (const auto& template_argument : type_reference.arguments)
//      {
//        const proto::type_reference* argument_type =
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
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = nullptr;
//      if (type_reference.arguments.size() >= 2)
//        size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      stream << "array<" << *value_type;
//      if (size)
//        stream << ", " << *size;
//      stream << ">";
//      break;
//    }

//    case built_in_type::list:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      stream << "list<" << *value_type << ">";
//      break;
//    }

//    case built_in_type::vector:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type && size);
//      if (!value_type || !size)
//        break;
//      stream << "vector<" << *value_type << ", " << *size << ">";
//      break;
//    }

//    case built_in_type::set:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(value_type);
//      if (!value_type)
//        break;
//      stream << "set<" << *value_type << ">";
//      break;
//    }

//    case built_in_type::matrix:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const int* size = std::get_if<int>(&type_reference.arguments[1]);
//      BOOST_ASSERT(value_type && size);
//      if (!value_type || !size)
//        break;
//      stream << "matrix<" << *value_type << ", " << *size << ">";
//      break;
//    }

//    case built_in_type::map:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 2);
//      const proto::type_reference* key_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      const proto::type_reference* value_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[1]);
//      BOOST_ASSERT(key_type && value_type);
//      if (!key_type || !value_type)
//        break;
//      stream << "map<" << *key_type << ", " << *value_type << ">";
//      break;
//    }

//    case built_in_type::variant:
//    {
//      stream << "variant<";
//      bool firstType = true;
//      for (auto& template_argument : type_reference.arguments)
//      {
//        const proto::type_reference* option =
//          std::get_if<proto::type_reference>(&template_argument);
//        BOOST_ASSERT(option);
//        if (!option)
//          break;
//        if (firstType)
//          firstType = false;
//        else
//          stream << ", ";
//        stream << *option;
//      }
//      stream << ">";
//      break;
//    }
//    default:
//      stream << *type;
//    }
//  }
//  else if (auto* alias = type_reference.as_alias())
//    stream << static_cast<const node&>(*alias);
//  else if (auto* enumeration = type_reference.as_enumeration())
//    stream << static_cast<const node&>(*enumeration);
//  else if (auto* message = type_reference.as_message())
//    stream << static_cast<const node&>(*message);
//  else
//    stream << "<unknown-type>";

//  return stream;
//}

// static const char* auto_generated_file_warning =
//  "////////////////////////////////////////////////////////////////" br
//  "// WARNING! Automatically generated file.                     //" br
//  "// Do not touch anything or your changes will be overwritten! //" br
//  "////////////////////////////////////////////////////////////////" br;

// bool dot_generator::active() const
//{
//  return !dot_output_path.empty();
//}

// std::vector<boost::filesystem::path> dot_generator::output_paths() const
//{
//  return {dot_output_path};
//}

// bool dot_generator::generate(namescope& root_scope, namescope&
// /*limit_scope*/)
//{
//  _use_services = false;
//  preprocess(root_scope);

//  file_writer fileSource;
//  _source = &fileSource;
//  *_source << auto_generated_file_warning << indent_width(2);
//  *_source << "digraph G {" br << inc_indent;
//  gather_nodes(root_scope);
//  *_source << br;
//  gather_edges(root_scope);
//  std::set<edge, edge_name_comparator> sorted_edges;
//  for (const auto& edge : _edges)
//    sorted_edges.insert(edge);
//  for (const auto& edge : sorted_edges)
//  {
//    *_source << indent << "\"" << static_cast<const node&>(*edge.first)
//             << "\" -> \"" << static_cast<const node&>(*edge.second) << "\""
//             br;
//  }
//  *_source << dec_indent << indent << "}" br;
//  _source->write(dot_output_path / "graph.dot");

//  std::string includeGuardPrefix = core::to_upper(cpp_namespace);
//  boost::replace_all(includeGuardPrefix, "::", "_");
//  std::string includePath = cpp_folder.empty() ? cpp_namespace : cpp_folder;
//  boost::replace_all(includePath, "::", "/");

//  _source = nullptr;
//  return true;
//}

// void dot_generator::preprocess(namescope& scope)
//{
//  if (!scope.name.empty())
//  {
//    scope.attribute("dot_name", delimiter_case(scope.name),
//                    attribute_assignment::keep);
//  }

//  for (auto* alias : scope.aliases)
//  {
//    alias->attribute("dot_name", delimiter_case(alias->name),
//                     attribute_assignment::keep);
//  }
//  for (auto* enumeration : scope.enumerations)
//  {
//    enumeration->attribute("dot_name", delimiter_case(enumeration->name),
//                           attribute_assignment::keep);
//    for (auto& member : enumeration->members)
//    {
//      member.attribute("dot_name", delimiter_case(member.name),
//                       attribute_assignment::keep);
//    }
//  }
//  for (auto* message : scope.messages)
//  {
//    message->attribute("dot_name", delimiter_case(message->name),
//                       attribute_assignment::keep);
//    for (auto& field : message->fields)
//    {
//      field.attribute("dot_name", delimiter_case(field.name),
//                      attribute_assignment::keep);
//    }
//  }
//  for (auto* interface : scope.interfaces)
//  {
//    interface->attribute("dot_name", interface->name,
//                         attribute_assignment::keep);

//    for (auto* message : interface->messages)
//    {
//      // C++ does not support predeclarations of nested classes. Thus we
//      // move the message type to the surrounding name scope and merge the
//      // two names together to avoid name collisions.
//      message->attribute("dot_name",
//                         interface->attribute<std::string>("dot_name") + "_" +
//                           delimiter_case(message->name),
//                         attribute_assignment::keep);
//      for (auto& field : message->fields)
//        field.attribute("dot_name", field.name, attribute_assignment::keep);
//    }
//  }
//  _use_services |= !scope.services.empty();
//  for (auto* service : scope.services)
//  {
//    service->attribute("dot_name", service->name, attribute_assignment::keep);
//  }

//  for (auto sub_namescope : scope.namescopes)
//    preprocess(*sub_namescope);
//}

// void dot_generator::gather_nodes(const namescope& scope)
//{
//  if (scope.has_attribute("dot_name"))
//  {
//    *_source << indent << "subgraph \""
//             << scope.attribute<std::string>("dot_name") << "\" {" br
//             << inc_indent;
//    *_source << indent << "label = \"" << static_cast<const node&>(scope)
//             << "\";" br;
//  }

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

//  if (scope.has_attribute("dot_name"))
//    *_source << dec_indent << indent << "}" br;
//}

// void dot_generator::gather_nodes(const type_reference& reference)
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

// void dot_generator::gather_nodes(const alias& alias)
//{
//  if (_nodes.find(&alias) != _nodes.end())
//    return;
//  _nodes.insert(&alias);
//  *_source << indent << "\"" << static_cast<const node&>(alias)
//           << "\" [label=\"{" << alias.attribute<std::string>("dot_name");

//  std::stringstream field_stream;
//  field_stream << alias.reference;
//  auto field_type = boost::replace_all_copy(
//    boost::replace_all_copy(
//      boost::replace_all_copy(field_stream.str(), " ", "\\ "), "<", "\\<"),
//    ">", "\\>");
//  if (field_type.length() > 48)
//    field_type = field_type.substr(0, 45) + "...";
//  *_source << "|" << field_type << " "
//           << alias.attribute<std::string>("dot_name");

//  *_source << "}\", shape=record];" br;
//  gather_nodes(alias.reference);
//}

// void dot_generator::gather_nodes(const enumeration& enumeration)
//{
//  if (_nodes.find(&enumeration) != _nodes.end())
//    return;
//  _nodes.insert(&enumeration);
//  *_source << indent << "\"" << static_cast<const node&>(enumeration)
//           << "\" [label=\"" << enumeration.attribute<std::string>("dot_name")
//           << "\", shape=box style=rounded, color=greenyellow];" br;
//  gather_nodes(enumeration.base);
//}

// void dot_generator::gather_nodes(const message& message)
//{
//  if (_nodes.find(&message) != _nodes.end())
//    return;
//  _nodes.insert(&message);
//  *_source << indent << "\"" << static_cast<const node&>(message)
//           << "\" [label=\"{" << message.attribute<std::string>("dot_name");
//  for (const auto& field : message.fields)
//  {
//    std::stringstream field_stream;
//    field_stream << field.reference;
//    auto field_type = boost::replace_all_copy(
//      boost::replace_all_copy(
//        boost::replace_all_copy(field_stream.str(), " ", "\\ "), "<", "\\<"),
//      ">", "\\>");
//    if (field_type.length() > 48)
//      field_type = field_type.substr(0, 45) + "...";
//    *_source << "|" << field_type << " "
//             << field.attribute<std::string>("dot_name");
//  }
//  *_source << "}\", shape=record];" br;
//  if (message.base)
//    gather_nodes(*message.base);
//  for (const auto& field : message.fields)
//    gather_nodes(field.reference);
//}

// void dot_generator::gather_nodes(const interface& interface)
//{
//  if (_nodes.find(&interface) != _nodes.end())
//    return;
//  _nodes.insert(&interface);
//  *_source << indent << "\"" << static_cast<const node&>(interface)
//           << "\" [label=\"" << interface.attribute<std::string>("dot_name")
//           << "\", shape=box, style=filled, color=lightblue];" br;
//  gather_nodes(static_cast<const namescope&>(interface));
//}

// void dot_generator::gather_nodes(const service& service)
//{
//  if (_nodes.find(&service) != _nodes.end())
//    return;
//  _nodes.insert(&service);
//  *_source << indent << "\"" << static_cast<const node&>(service)
//           << "\" [label=\"" << service.attribute<std::string>("dot_name")
//           << "\", shape=box, style=filled, color=yellow];" br;
//  gather_nodes(*service.service_interface);
//  gather_nodes(*service.callback_interface);
//}

// void dot_generator::gather_edges(const namescope& scope)
//{
//  for (const auto* alias : scope.aliases)
//    gather_edges(alias, alias->reference);
//  for (const auto* message : scope.messages)
//  {
//    if (message->base)
//      _edges.insert(std::make_pair(message, message->base));
//    for (const auto& field : message->fields)
//      gather_edges(message, field.reference);
//  }
//  for (const auto* interface : scope.interfaces)
//  {
//    for (const auto* message : interface->messages)
//    {
//      _edges.insert(std::make_pair(interface, message));
//      for (const auto& field : message->fields)
//        gather_edges(message, field.reference);
//    }
//  }
//  for (const auto* service : scope.services)
//  {
//    _edges.insert(std::make_pair(service, service->service_interface));
//    _edges.insert(std::make_pair(service, service->callback_interface));
//  }
//  for (const auto sub_namescope : scope.namescopes)
//    gather_edges(*sub_namescope);
//}

// void dot_generator::gather_edges(const node* source,
//                                 const type_reference& reference)
//{
//  if (const auto* built_in_type = reference.as_built_in_type())
//  {
//    for (const auto& template_argument : reference.arguments)
//    {
//      const auto* argument_type =
//        std::get_if<proto::type_reference>(&template_argument);
//      if (!argument_type)
//        continue;
//      gather_edges(source, *argument_type);
//    }
//  }
//  else if (const auto* alias = reference.as_alias())
//    _edges.insert(std::make_pair(source, alias));
//  else if (const auto* enumeration = reference.as_enumeration())
//    _edges.insert(std::make_pair(source, enumeration));
//  else if (const auto* message = reference.as_message())
//    _edges.insert(std::make_pair(source, message));
//}
}
