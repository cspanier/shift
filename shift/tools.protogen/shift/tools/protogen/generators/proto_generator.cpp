#include "shift/tools/protogen/generators/proto_generator.h"
#include <shift/log/log.h>
#include <shift/core/string_util.h>
#include <shift/core/stream_util.h>
#include <shift/core/boost_disable_warnings.h>
#include <boost/filesystem.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <set>

namespace shift::tools::protogen::generators
{
// using core::inc_indent;
// using core::dec_indent;
// using core::indent;
// using core::indent_width;

//#if defined(SHIFT_PLATFORM_WINDOWS)
//#define br "\r\n"
//#else
//#define br "\n"
//#endif
//#define br2 br br

// struct source_gatherer
//{
//  using return_type = void;

//  void operator()(const namescope& scope)
//  {
//    for (const auto& child : scope.dynamic_children)
//      std::visit(*this, *child);
//  }

//  void operator()(const node& node)
//  {
//    if (node.has_attribute("source_filename"))
//      filenames.insert(node.attribute<std::string>("source_filename"));
//  }

//  std::set<std::string> filenames;
//};

// static file_writer& operator<<(file_writer& stream, const type_path&
// typePath)
//{
//  bool first = true;
//  for (const auto& typeName : typePath)
//  {
//    if (first)
//      first = false;
//    else
//      stream << ".";
//    stream << typeName;
//  }
//  return stream;
//}

// static file_writer& operator<<(file_writer& stream, const built_in_type&
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

// static inline std::tuple<const namescope&, const type_reference&> make_tuple(
//  const namescope& scope, const type_reference& type_reference)
//{
//  return std::make_tuple(std::cref(scope), std::cref(type_reference));
//}

// static file_writer& operator<<(
//  file_writer& stream,
//  std::tuple<const namescope&, const type_reference&> arguments)
//{
//  const namescope& current_scope = std::get<0>(arguments);
//  const type_reference& type_reference = std::get<1>(arguments);
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
//      stream << "raw_ptr<" << make_tuple(current_scope, *argument_type) <<
//      ">"; break;
//    }

//    case built_in_type::unique_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "unique_ptr<" << make_tuple(current_scope, *argument_type)
//             << ">";
//      break;
//    }

//    case built_in_type::shared_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "shared_ptr<" << make_tuple(current_scope, *argument_type)
//             << ">";
//      break;
//    }

//    case built_in_type::weak_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "weak_ptr<" << make_tuple(current_scope, *argument_type) <<
//      ">"; break;
//    }

//    case built_in_type::group_ptr:
//    {
//      BOOST_ASSERT(type_reference.arguments.size() == 1);
//      const proto::type_reference* argument_type =
//        std::get_if<proto::type_reference>(&type_reference.arguments[0]);
//      BOOST_ASSERT(argument_type);
//      stream << "group_ptr<" << make_tuple(current_scope, *argument_type)
//             << ">";
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
//        stream << make_tuple(current_scope, *argument_type);
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
//      stream << "array<" << make_tuple(current_scope, *value_type);
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
//      stream << "list<" << make_tuple(current_scope, *value_type) << ">";
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
//      stream << "vector<" << make_tuple(current_scope, *value_type) << ", "
//             << *size << ">";
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
//      stream << "set<" << make_tuple(current_scope, *value_type) << ">";
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
//      stream << "matrix<" << make_tuple(current_scope, *value_type) << ", "
//             << *size << ">";
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
//      stream << "map<" << make_tuple(current_scope, *key_type) << ", "
//             << make_tuple(current_scope, *value_type) << ">";
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
//        stream << make_tuple(current_scope, *option);
//      }
//      stream << ">";
//      break;
//    }
//    default:
//      stream << *type;
//    }
//  }
//  else if (auto* alias = type_reference.as_alias())
//    stream << proto_generator::relative_name(alias, &current_scope);
//  else if (auto* enumeration = type_reference.as_enumeration())
//    stream << proto_generator::relative_name(enumeration, &current_scope);
//  else if (auto* message = type_reference.as_message())
//    stream << proto_generator::relative_name(message, &current_scope);
//  else
//    stream << "<unknown-type>";

//  return stream;
//}

// bool proto_generator::active() const
//{
//  return !proto_output_path.empty();
//}

// std::vector<boost::filesystem::path> proto_generator::output_paths() const
//{
//  return {proto_output_path};
//}

// bool proto_generator::generate(namescope& root_scope, namescope& limit_scope)
//{
//  _source << indent_width(2);
//  _source.on_enter_namescope = [&](std::ostream& stream,
//                                   const namescope& scope) {
//    write_attributes(scope);
//    stream << indent << "namescope " << scope.name << br;
//    stream << indent << "{" br << inc_indent;
//  };
//  _source.on_leave_namescope = [](std::ostream& stream,
//                                  const namescope& /*scope*/) {
//    stream << dec_indent << indent << "}" br;
//  };

//  // Gather files to write.
//  source_gatherer gatherer;
//  for (const auto& child : limit_scope.dynamic_children)
//    std::visit(gatherer, *child);

//  // Write all files which were found to be among the sources of the AST.
//  for (const auto& filename : gatherer.filenames)
//  {
//    log::info() << "Rewriting source file " << filename << "...";
//    _source.global_scope(root_scope);
//    write(filename, limit_scope);
//    _source.switch_namescope(root_scope);
//    _source.write(filename);
//    _source.clear();
//  }
//  return true;
//}

// std::string proto_generator::relative_name(const node* node,
//                                           const namescope* current_scope)
//{
//  static auto name_lookup = [](const proto::node& node) { return node.name; };
//  return core::implode(
//    base_generator::relative_path(name_lookup, node, current_scope), ".");
//}

// void proto_generator::write(std::string_view source_filename,
//                            const namescope& scope)
//{
//  for (auto* aliases : scope.aliases)
//  {
//    if (aliases->attribute<std::string>("source_filename") != source_filename)
//      continue;
//    write(*aliases);
//  }
//  for (auto* enumeration : scope.enumerations)
//  {
//    if (enumeration->attribute<std::string>("source_filename") !=
//        source_filename)
//      continue;
//    write(*enumeration);
//  }
//  for (auto* message : scope.messages)
//  {
//    if (message->attribute<std::string>("source_filename") != source_filename)
//      continue;
//    write(*message);
//  }
//  for (auto* interface : scope.interfaces)
//  {
//    if (interface->attribute<std::string>("source_filename") !=
//    source_filename)
//      continue;
//    write(source_filename, *interface);
//  }
//  for (auto* service : scope.services)
//  {
//    if (service->attribute<std::string>("source_filename") != source_filename)
//      continue;
//    write(*service);
//  }
//  for (auto* sub_namescope : scope.namescopes)
//    write(source_filename, *sub_namescope);
//}

// void proto_generator::write(const alias& alias)
//{
//  if (!_source.switch_namescope(*alias.parent))
//    _source << br;

//  write_attributes(alias);
//  _source << indent << "alias " << alias.name << " = "
//          << make_tuple(_source.current_scope(), alias.reference) << br;
//}

// void proto_generator::write(const enumeration& enumeration)
//{
//  if (!_source.switch_namescope(*enumeration.parent))
//    _source << br;

//  write_attributes(enumeration);
//  _source << indent << "enum " << enumeration.name << " : "
//          << make_tuple(_source.current_scope(), enumeration.base) << br;
//  _source << indent << "{" << inc_indent;
//  bool first = true;
//  std::int64_t nextValue = 0;
//  for (auto& member : enumeration.members)
//  {
//    if (first)
//      first = false;
//    else
//      _source << ",";
//    _source << br << indent << camel_case(member.name);
//    if (member.value == nextValue)
//      ++nextValue;
//    else
//    {
//      _source << " = " << member.value;
//      nextValue = member.value + 1;
//    }
//  }
//  _source << br << dec_indent << indent << "}" br;
//}

// void proto_generator::write(const message& message)
//{
//  if (!_source.switch_namescope(*message.parent))
//    _source << br;

//  write_attributes(message);
//  _source << indent << "message " << message.name;
//  if (message.base)
//    _source << " : " << message.base_name;
//  _source << br << indent << "{" br << inc_indent;
//  for (const auto& field : message.fields)
//  {
//    write_attributes(field);
//    _source << indent << make_tuple(_source.current_scope(), field.reference)
//            << " " << field.name << ";" br;
//  }
//  _source << dec_indent << indent << "}" br;
//}

// void proto_generator::write(std::string_view source_filename,
//                            const interface& interface)
//{
//  if (!_source.switch_namescope(*interface.parent))
//    _source << br;

//  _source << indent << "interface " << interface.name << br;
//  _source << indent << "{" br << inc_indent;
//  bool first = true;
//  for (const auto* message : interface.messages)
//  {
//    if (message->attribute<std::string>("source_filename") != source_filename)
//      continue;
//    if (first)
//      first = false;
//    else
//      _source << br;
//    write(*message);
//  }
//  _source << dec_indent << indent << "}" br;

//  write_attributes(interface);
//}

// void proto_generator::write(const service& service)
//{
//  _source.switch_namescope(*service.parent);

//  write_attributes(service);
//  _source << indent;
//  if (service.is_provider)
//    _source << "service_provider<";
//  else
//    _source << "service_client<";
//  _source << service.service_name << ", " << service.callback_name << "> "
//          << service.name << ";" br;
//}

// void proto_generator::write_attributes(const node& node)
//{
//  bool hasAttributes = false;
//  for (const auto& attribute : node.public_attributes)
//  {
//    if (attribute.first == "source_filename")
//      continue;
//    if (!hasAttributes)
//    {
//      _source << indent << "[";
//      hasAttributes = true;
//    }
//    else
//      _source << ", ";
//    _source << attribute.first << " = ";
//    if (auto* string = std::get_if<std::string>(&attribute.second))
//      _source << '"' << *string << '"';
//    else if (auto* value = std::get_if<std::uint64_t>(&attribute.second))
//      _source << "0x" << std::hex << std::setw(8) << value;
//    else
//      BOOST_ASSERT(false);
//  }
//  if (hasAttributes)
//    _source << "]" br;
//}
}
