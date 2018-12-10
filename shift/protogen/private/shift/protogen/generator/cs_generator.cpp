#include "shift/protogen/generator/cs_generator.hpp"
#include <shift/core/stream_util.hpp>
#include <shift/core/string_util.hpp>

namespace shift::proto::generator
{
using core::dec_indent;
using core::inc_indent;
using core::indent;
using core::indent_width;
using core::tab;

#if defined(SHIFT_PLATFORM_WINDOWS)
#define br "\r\n"
#else
#define br "\n"
#endif
#define br2 br br

static const char* auto_generated_file_warning =
  "////////////////////////////////////////////////////////////////" br
  "// WARNING! Automatically generated file.                     //" br
  "// Do not touch anything or your changes will be overwritten! //" br
  "////////////////////////////////////////////////////////////////" br;

enum class ptr_type
{
  no_ptr,
  raw_ptr,
  unique_ptr,
  shared_ptr,
  weak_ptr,
  group_ptr
};

enum class print_mode
{
  type,
  proto_type
};

using print_tuple =
  std::tuple<const namescope&, const type_reference&, print_mode, ptr_type>;

static inline print_tuple make_tuple(const namescope& scope,
                                     const type_reference& reference,
                                     print_mode mode,
                                     ptr_type ptrType = ptr_type::no_ptr)
{
  return std::make_tuple(std::cref(scope), std::cref(reference), mode, ptrType);
}

static file_writer& operator<<(file_writer& stream, print_tuple arguments)
{
  const auto& current_scope = std::get<0>(arguments);
  const auto& reference = std::get<1>(arguments);
  print_mode mode = std::get<2>(arguments);
  ptr_type ptrType = std::get<3>(arguments);

  if (mode == print_mode::proto_type)
  {
    switch (ptrType)
    {
    case ptr_type::no_ptr:
      break;
    case ptr_type::raw_ptr:
      stream << "RawPtr<";
      break;
    case ptr_type::unique_ptr:
      stream << "UniquePtr<";
      break;
    case ptr_type::shared_ptr:
      stream << "SharedPtr<";
      break;
    case ptr_type::weak_ptr:
      stream << "WeakPtr<";
      break;
    case ptr_type::group_ptr:
      stream << "GroupPtr<";
      break;
    default:
      BOOST_ASSERT(false);
    }
  }

  if (auto* type = reference.as_built_in_type())
  {
    switch (*type)
    {
    case built_in_type::undefined:
      stream << "<undefined built-in type>";
      break;

    case built_in_type::boolean:
      stream << "bool";
      break;

    case built_in_type::char8:
      stream << "sbyte";
      break;

    case built_in_type::char16:
      stream << "char";
      break;

    case built_in_type::char32:
      stream << "int";
      break;

    case built_in_type::int8:
      stream << "sbyte";
      break;

    case built_in_type::uint8:
      stream << "byte";
      break;

    case built_in_type::int16:
      stream << "short";
      break;

    case built_in_type::uint16:
      stream << "ushort";
      break;

    case built_in_type::int32:
      stream << "int";
      break;

    case built_in_type::uint32:
      stream << "uint";
      break;

    case built_in_type::int64:
      stream << "long";
      break;

    case built_in_type::uint64:
      stream << "ulong";
      break;

    case built_in_type::datetime:
      stream << "DateTime";
      break;

    case built_in_type::float32:
      stream << "float";
      break;

    case built_in_type::float64:
      stream << "double";
      break;

    case built_in_type::string:
      stream << "string";
      break;

    case built_in_type::raw_ptr:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* argument_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(argument_type);
      stream << make_tuple(current_scope, *argument_type, mode,
                           ptr_type::raw_ptr);
      break;
    }

    case built_in_type::unique_ptr:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* argument_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(argument_type);
      stream << make_tuple(current_scope, *argument_type, mode,
                           ptr_type::unique_ptr);
      break;
    }

    case built_in_type::shared_ptr:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* argument_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(argument_type);
      stream << make_tuple(current_scope, *argument_type, mode,
                           ptr_type::shared_ptr);
      break;
    }

    case built_in_type::weak_ptr:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* argument_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(argument_type);
      stream << make_tuple(current_scope, *argument_type, mode,
                           ptr_type::weak_ptr);
      break;
    }

    case built_in_type::group_ptr:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* argument_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(argument_type);
      stream << make_tuple(current_scope, *argument_type, mode,
                           ptr_type::group_ptr);
      break;
    }

    case built_in_type::tuple:
    {
      bool firstArgument = true;
      stream << "Tuple<";
      for (const auto& template_argument : reference.arguments)
      {
        const type_reference* argument_type =
          std::get_if<type_reference>(&template_argument);
        BOOST_ASSERT(argument_type);
        if (firstArgument)
          firstArgument = false;
        else
          stream << ", ";
        stream << make_tuple(current_scope, *argument_type, mode);
      }
      stream << ">";
      break;
    }

    case built_in_type::array:
    {
      BOOST_ASSERT(!reference.arguments.empty());
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      // The array size is not needed at this point in C#.
      // const int* size = nullptr;
      // if (reference.arguments.size() >= 2)
      //   size = std::get_if<int>(&reference.arguments[1]);
      BOOST_ASSERT(value_type);
      if (value_type == nullptr)
        break;

      stream << make_tuple(current_scope, *value_type, mode) << "[]";
      break;
    }

    case built_in_type::list:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(value_type);
      if (value_type == nullptr)
        break;
      stream << "List<" << make_tuple(current_scope, *value_type, mode) << ">";
      break;
    }

    case built_in_type::vector:
    {
      BOOST_ASSERT(reference.arguments.size() == 2);
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      const int* size = std::get_if<int>(&reference.arguments[1]);
      BOOST_ASSERT(value_type && size);
      if ((value_type == nullptr) || (size == nullptr))
        break;

      auto* builtInValueType =
        std::get_if<proto::built_in_type>(&value_type->variant);
      if ((builtInValueType != nullptr) &&
          (*builtInValueType == built_in_type::float32 ||
           *builtInValueType == built_in_type::float64))
      {
        if (*size == 2)
          stream << "System.Windows.Point";
        else if (*size == 3)
          stream << "Thales.Sagittarius.Data.Media3D.Point3D";
        else if (*size == 4)
          stream << "Thales.Sagittarius.Data.Media3D.Point4D";
        else
          stream << "<<unsupported vector type>>";
      }
      else
      {
        stream << make_tuple(current_scope, *value_type, mode) << "[" << *size
               << "]";
      }
      break;
    }

    case built_in_type::set:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(value_type);
      if (value_type == nullptr)
        break;
      stream << "HashSet<" << make_tuple(current_scope, *value_type, mode)
             << ">";
      break;
    }

    case built_in_type::matrix:
    {
      BOOST_ASSERT(reference.arguments.size() == 2);
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      const int* size = std::get_if<int>(&reference.arguments[1]);
      BOOST_ASSERT(value_type && size);
      if ((value_type == nullptr) || (size == nullptr))
        break;

      auto* builtInValueType =
        std::get_if<proto::built_in_type>(&value_type->variant);
      if ((builtInValueType != nullptr) &&
          *builtInValueType == built_in_type::float32 && *size == 4)
      {
        stream << "Thales.Sagittarius.Data.Media3D.Matrix3D";
      }
      else
        stream << make_tuple(current_scope, *value_type, mode) << "[" << *size
               << "][" << *size << "]";
      break;
    }

    case built_in_type::map:
    {
      BOOST_ASSERT(reference.arguments.size() == 2);
      const type_reference* key_type =
        std::get_if<type_reference>(&reference.arguments[0]);
      const type_reference* value_type =
        std::get_if<type_reference>(&reference.arguments[1]);
      BOOST_ASSERT(key_type && value_type);
      stream << "Dictionary<" << make_tuple(current_scope, *key_type, mode)
             << ", " << make_tuple(current_scope, *value_type, mode) << ">";
      break;
    }

    case built_in_type::variant:
    {
      int i = 0;
      bool firstType = true;
      for (auto& template_argument : reference.arguments)
      {
        const type_reference* argumentReference =
          std::get_if<type_reference>(&template_argument);
        BOOST_ASSERT(argumentReference);
        if (argumentReference == nullptr)
          break;
        if (firstType)
          firstType = false;
        else
          stream << ", ";
        if (i++ % 7 == 0)
          stream << "Variant<";
        stream << make_tuple(current_scope, *argumentReference, mode);
      }
      stream << std::string((i - 1) / 7 + 1, '>');
      break;
    }

    case built_in_type::bitfield:
    {
      BOOST_ASSERT(reference.arguments.size() == 1);
      const type_reference* enumReference =
        std::get_if<type_reference>(&reference.arguments[0]);
      BOOST_ASSERT(enumReference);
      auto* enumeration = enumReference->as_enumeration();
      if ((enumReference == nullptr) || (enumeration == nullptr))
        break;
      stream << make_tuple(current_scope, *enumReference, mode);
      break;
    }

    default:
      stream << "<unknown built-in type>";
      break;
    }
  }
  else if (auto* alias = reference.as_alias())
  {
    const auto& aliasTypeReference = alias->actual_type_reference();
    auto* aliasBuiltInType = aliasTypeReference.as_built_in_type();
    if (((aliasBuiltInType != nullptr) &&
         (built_in_type_traits::is_char(*aliasBuiltInType) ||
          built_in_type_traits::is_int(*aliasBuiltInType) ||
          built_in_type_traits::is_float(*aliasBuiltInType) ||
          *aliasBuiltInType == built_in_type::bitfield)) ||
        (mode == print_mode::proto_type))
    {
      stream << make_tuple(current_scope, aliasTypeReference, mode, ptrType);
    }
    else
      stream << cs_generator::relative_name(*alias, current_scope);
  }
  else if (auto* enumeration = reference.as_enumeration())
    stream << cs_generator::relative_name(*enumeration, current_scope);
  else if (auto* message = reference.as_message())
    stream << cs_generator::relative_name(*message, current_scope);
  else
    stream << "<unknown-type>";

  if (mode == print_mode::type)
  {
    switch (ptrType)
    {
    case ptr_type::no_ptr:
      break;
    case ptr_type::raw_ptr:
    case ptr_type::unique_ptr:
    case ptr_type::shared_ptr:
    case ptr_type::weak_ptr:
    case ptr_type::group_ptr:
    {
      auto* built_in_type = reference.as_built_in_type();
      if ((built_in_type != nullptr) &&
          !built_in_type_traits::is_template(*built_in_type) &&
          !built_in_type_traits::is_string(*built_in_type))
      {
        stream << "?";
      }
      break;
    }
    default:
      BOOST_ASSERT(false);
    }
  }
  else if (mode == print_mode::proto_type)
  {
    switch (ptrType)
    {
    case ptr_type::no_ptr:
      break;
    case ptr_type::raw_ptr:
    case ptr_type::unique_ptr:
    case ptr_type::shared_ptr:
    case ptr_type::weak_ptr:
    case ptr_type::group_ptr:
      stream << ">";
      break;
    default:
      BOOST_ASSERT(false);
    }
  }

  return stream;
}

bool cs_generator::active() const
{
  return !cs_output_path.empty();
}

std::vector<boost::filesystem::path> cs_generator::output_paths() const
{
  return {cs_output_path};
}

bool cs_generator::generate(namescope& root_scope, namescope& limit_scope)
{
  file_writer fileSource;
  fileSource.global_scope(root_scope);
  fileSource.on_enter_namescope = [](std::ostream& stream,
                                     const namescope& scope) {
    if (scope.has_attribute("cs_name") && !scope.has_attribute("is_interface"))
    {
      stream << indent << "namespace "
             << scope.attribute<std::string>("cs_name") << br;
      stream << indent << "{" br << inc_indent;
    }
  };
  fileSource.on_leave_namescope = [](std::ostream& stream,
                                     const namescope& scope) {
    if (scope.has_attribute("cs_name") && !scope.has_attribute("is_interface"))
    {
      stream << dec_indent << indent << "}" br;
    }
  };
  _source = &fileSource;
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Network;" br;
    *_source << "using Shift.Protogen;" br2;

    preprocess(root_scope);
    writeEnumDefinitions(limit_scope);
    _source->switch_namescope(root_scope);
    _source->write(cs_output_path / "Enumerations.cs");
    _source->clear();
  }
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Network;" br;
    *_source << "using Shift.Protogen;" br2;

    preprocess(root_scope);
    writeAliasDefinitions(limit_scope);
    _source->switch_namescope(root_scope);
    _source->write(cs_output_path / "Aliases.cs");
    _source->clear();
  }
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Network;" br;
    *_source << "using Shift.Protogen;" br2;

    preprocess(root_scope);
    writeMessageDefinitions(limit_scope);
    _source->switch_namescope(root_scope);
    _source->write(cs_output_path / "Messages.cs");
    _source->clear();
  }
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Network;" br;
    *_source << "using Shift.Protogen;" br2;

    preprocess(root_scope);
    writeInterfaceDefinitions(limit_scope);
    _source->switch_namescope(root_scope);
    _source->write(cs_output_path / "Interfaces.cs");
    _source->clear();
  }

  if (requiresInterfaceDeserializers(limit_scope))
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Service;" br;
    *_source << "using Shift.Network;" br;
    for (const auto& path : cs_usings)
      *_source << "using " << path << ";" br;
    *_source << br;

    *_source << "namespace Shift.Service" br;
    *_source << "{" br << inc_indent;
    writeInterfaceDeserializers(limit_scope);
    *_source << dec_indent << indent << "}" br;

    _source->write(cs_output_path / "Deserializer.cs");
    _source->clear();
  }

  if (requiresServiceDefinitions(limit_scope))
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Network;" br;
    *_source << "using Shift.Service;" br;
    for (const auto& path : cs_usings)
      *_source << "using " << path << ";" br;
    *_source << br;

    writeServices(limit_scope);
    _source->switch_namescope(root_scope);

    _source->write(cs_output_path / "Services.cs");
    _source->clear();
  }

  if (requiresTypeRegistry(limit_scope))
  {
    *_source << auto_generated_file_warning
             << indent_width(
                  static_cast<int>(program_options::cs_indent_width));
    *_source << "using System;" br;
    *_source << "using System.Collections.Generic;" br;
    *_source << "using Shift.Service;" br;
    for (const auto& path : cs_usings)
      *_source << "using " << path << ";" br;
    *_source << br;

    *_source << indent << "namespace " << cs_namespace << br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent << "public static class TypeRegistry" br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent << "public static void RegisterInterfaces()" br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent << "var host = ServiceHost.Instance;" br;
    writeTypeRegistry(limit_scope);
    *_source << dec_indent << indent << "}" br;
    *_source << dec_indent << indent << "}" br;
    *_source << dec_indent << indent << "}" br;

    _source->write(cs_output_path / "TypeRegistry.cs");
    _source->clear();
  }

  return true;
}

std::string cs_generator::relative_name(const node& node,
                                        const namescope& current_scope,
                                        std::string delimiter)
{
  static auto name_lookup = [](const proto::node& node) -> std::string {
    if (node.has_attribute("cs_name"))
      return node.attribute<std::string>("cs_name");
    else
      return node.name;
  };
  auto relative_path =
    base_generator::relative_path(name_lookup, &node, &current_scope);
  // Since C# interfaces cannot declare inner types, messages defined within
  // interfaces are moved to the surrounding name-scope. In this case we do
  // not want to include the interface name in the name path as we already
  // merged that name into the cs_name message attribute.
  if ((node.parent != nullptr) && node.parent->has_attribute("is_interface") &&
      relative_path.size() > 1)
  {
    relative_path.erase(relative_path.begin() + relative_path.size() - 2);
  }
  return core::implode(relative_path, delimiter);
}

void cs_generator::preprocess(namescope& scope)
{
  if (!scope.name.empty())
  {
    scope.attribute("cs_name", pascal_case(scope.name),
                    attribute_assignment::keep);
  }

  for (auto* enumeration : scope.enumerations)
  {
    enumeration->attribute("cs_name", enumeration->name,
                           attribute_assignment::keep);
    for (auto& member : enumeration->members)
    {
      member.attribute("cs_name", pascal_case(member.name),
                       attribute_assignment::keep);
    }
  }
  for (auto* message : scope.messages)
  {
    message->attribute("cs_name", message->name, attribute_assignment::keep);
    for (auto& field : message->fields)
    {
      field.attribute("cs_name", pascal_case(field.name),
                      attribute_assignment::keep);
      field.attribute("csArgumentName",
                      camel_case(field.attribute<std::string>("cs_name")),
                      attribute_assignment::keep);
    }
  }
  for (auto* interface : scope.interfaces)
  {
    interface->attribute("cs_name", interface->name,
                         attribute_assignment::keep);
    interface->attribute(
      "csDeserializer",
      "Deserializer_" + relative_name(*interface, _source->global_scope(), "_"),
      attribute_assignment::overwrite);
    auto interfaceName = interface->attribute<std::string>("cs_name").substr(1);
    for (auto* message : interface->messages)
    {
      // C# does not support types being declared within interfaces. Thus we
      // move the message type to the surrounding name scope and merge the
      // two names together to avoid name collisions.
      message->attribute("cs_name", interfaceName + "_" + message->name,
                         attribute_assignment::keep);
      message->attribute("csEventName", "On" + message->name,
                         attribute_assignment::keep);
      for (auto& field : message->fields)
      {
        field.attribute("cs_name", pascal_case(field.name),
                        attribute_assignment::keep);
        field.attribute("csArgumentName",
                        camel_case(field.attribute<std::string>("cs_name")),
                        attribute_assignment::keep);
      }
    }
  }
  for (auto* service : scope.services)
    service->attribute("cs_name", service->name, attribute_assignment::keep);

  for (auto sub_namescope : scope.namescopes)
    preprocess(*sub_namescope);
}

void cs_generator::writeEnumDefinitions(namescope& scope)
{
  bool hasEnumerations = false;
  for (auto enumeration : scope.enumerations)
  {
    if (!enumeration->has_attribute("csExported") ||
        enumeration->attribute<std::string>("csExported") != "true")
    {
      hasEnumerations = true;
      break;
    }
  }

  if (hasEnumerations)
  {
    _source->switch_namescope(scope);

    bool first = true;
    for (auto enumeration : scope.enumerations)
    {
      if (enumeration->has_attribute("csExported") &&
          enumeration->attribute<std::string>("csExported") == "true")
      {
        continue;
      }
      if (first)
        first = false;
      else
        *_source << br;

      if (enumeration->has_attribute("csFlags") &&
          enumeration->attribute<std::uint64_t>("csFlags") == 1)
      {
        *_source << indent << "[Flags]" br;
      }
      if (enumeration->has_attribute("csCustomAttribute"))
      {
        *_source << indent << "["
                 << enumeration->attribute<std::string>("csCustomAttribute")
                 << "]" br;
      }
      *_source << indent << "public enum " << enumeration->name << " : "
               << make_tuple(scope, enumeration->base, print_mode::type) << br;
      *_source << indent << "{" << inc_indent;
      std::int64_t nextAutoValue = 0;
      bool firstMember = true;
      for (auto& member : enumeration->members)
      {
        if (firstMember)
          firstMember = false;
        else
          *_source << ",";
        *_source << br;
        if (member.has_attribute("csCustomAttribute"))
        {
          *_source << indent << "["
                   << member.attribute<std::string>("csCustomAttribute")
                   << "]" br;
        }
        *_source << indent << member.attribute<std::string>("cs_name");
        if (member.value != nextAutoValue)
          *_source << " = " << member.value;
        nextAutoValue = member.value + 1;
      }
      *_source << br << dec_indent << indent << "};" br;
    }
  }

  for (auto sub_namescope : scope.namescopes)
    writeEnumDefinitions(*sub_namescope);
}

void cs_generator::writeAliasDefinitions(namescope& scope)
{
  bool has_aliases = false;
  for (auto alias : scope.aliases)
  {
    auto* type = alias->actual_type_reference().as_built_in_type();
    if (type == nullptr)
      continue;
    if (!alias->has_attribute("csExported") ||
        alias->attribute<std::string>("csExported") != "true")
    {
      if (!built_in_type_traits::is_char(*type) &&
          !built_in_type_traits::is_int(*type) &&
          !built_in_type_traits::is_float(*type) &&
          *type != built_in_type::bitfield)
      {
        has_aliases = true;
        break;
      }
    }
  }

  if (has_aliases)
  {
    for (auto alias : scope.aliases)
    {
      auto* type = alias->actual_type_reference().as_built_in_type();
      if (type == nullptr)
        continue;
      if (built_in_type_traits::is_char(*type) ||
          built_in_type_traits::is_int(*type) ||
          built_in_type_traits::is_float(*type) ||
          *type == built_in_type::bitfield)
      {
        continue;
      }
      if (alias->has_attribute("csExported") &&
          alias->attribute<std::string>("csExported") == "true")
      {
        continue;
      }
      if (!_source->switch_namescope(scope))
        *_source << br;

      *_source << indent << "[Alias(typeof("
               << make_tuple(scope, alias->reference, print_mode::type)
               << "))]" br;
      *_source << indent << "public class " << alias->name << " : "
               << make_tuple(scope, alias->reference, print_mode::type) << br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "public " << alias->name << "()" br;
      *_source << indent << "{" br;
      *_source << indent << "}" br2;
      *_source << indent << "public " << alias->name << "("
               << make_tuple(scope, alias->reference, print_mode::type)
               << " other) : base(other)" br;
      *_source << indent << "{" br;
      *_source << indent << "}" br;
      if (auto* alias_type = alias->reference.as_built_in_type())
      {
        if (*alias_type == built_in_type::variant)
        {
          // Variant aliases get implicit conversion operators for each
          // variant type.
          for (auto& template_argument : alias->reference.arguments)
          {
            const type_reference* argumentReference =
              std::get_if<type_reference>(&template_argument);
            BOOST_ASSERT(argumentReference);
            if (argumentReference == nullptr)
              break;
            *_source << br << indent << "public static implicit operator "
                     << alias->name << "("
                     << make_tuple(scope, *argumentReference, print_mode::type)
                     << " value)" br;
            *_source << indent << "{" br;
            *_source << indent << tab << "return new " << alias->name
                     << "() { Value = value };" br;
            *_source << indent << "}" br;
          }
        }
      }
      *_source << dec_indent << indent << "}" br;
    }
  }

  for (auto sub_namescope : scope.namescopes)
    writeAliasDefinitions(*sub_namescope);
}

void cs_generator::writeMessageDefinitions(namescope& scope)
{
  bool hasMessages = false;
  for (auto message : scope.messages)
  {
    if (!message->has_attribute("csExported") ||
        message->attribute<std::string>("csExported") != "true")
    {
      hasMessages = true;
      break;
    }
  }

  if (hasMessages)
  {
    for (auto message : scope.messages)
    {
      if (message->has_attribute("csExported") &&
          message->attribute<std::string>("csExported") == "true")
      {
        continue;
      }
      if (!_source->switch_namescope(scope))
        *_source << br;

      if (message->has_attribute("csCustomAttribute"))
      {
        *_source << indent << "["
                 << message->attribute<std::string>("csCustomAttribute")
                 << "]" br;
      }
      *_source << indent << "public partial class "
               << message->attribute<std::string>("cs_name");
      if (message->base != nullptr)
      {
        *_source << " : " << message->base->attribute<std::string>("cs_name")
                 << br;
      }
      else
        *_source << " : Shift.Network.Message" br;

      *_source << indent << "{" << inc_indent << br;
      *_source << indent << "public";
      // Eventually hide base member UID.
      if (message->base != nullptr)
        *_source << " new";
      *_source << " const uint Uid = 0x" << std::hex << std::setfill('0')
               << std::setw(8) << message->attribute<std::uint32_t>("uid")
               << ";" br2;
      *_source << indent << "/// <summary>Default constructor.</summary>" br;
      *_source << indent << "public "
               << message->attribute<std::string>("cs_name") << "()" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "MessageUid = Uid;" br;
      for (auto& field : message->fields)
      {
        if (field.has_attribute("csEntityCollection"))
        {
          *_source << indent << field.attribute<std::string>("cs_name")
                   << " = new "
                   << make_tuple(scope, field.reference, print_mode::type)
                   << "();" br;
        }
      }
      *_source << dec_indent << indent << "}" br2;

      if (message->has_fields())
      {
        *_source << indent << "/// <summary>Constructor.</summary>" br;
        *_source << indent << "public "
                 << message->attribute<std::string>("cs_name") << "(";
        writeMessageAsParameterList(scope, *message, true);
        *_source << ")" br;
        if (message->base != nullptr)
        {
          *_source << inc_indent;
          *_source << indent << ": base(";
          writeMessageAsArgumentList(scope, *message->base, true);
          *_source << ")" br << dec_indent;
        }
        *_source << indent << "{" br << inc_indent;
        *_source << indent << "MessageUid = Uid;" br;
        for (auto& field : message->fields)
        {
          *_source << indent << field.attribute<std::string>("cs_name") << " = "
                   << field.attribute<std::string>("csArgumentName") << ";" br;
        }
        *_source << dec_indent << indent << "}" br2;
      }

      std::size_t order = 1;
      /// If this message derives from another one, we must not start
      /// counting at 1!
      for (auto* base = message->base; base != nullptr; base = base->base)
        order += base->fields.size();

      for (auto& field : message->fields)
      {
        *_source << indent;
        if (field.has_attribute("csCustomAttribute"))
        {
          *_source << "[" << field.attribute<std::string>("csCustomAttribute")
                   << "]" br;
        }
        if (!field.has_attribute("ignore"))
        {
          *_source << "[Order(" << std::dec << order++ << "), ProtoType(typeof("
                   << make_tuple(scope, field.reference, print_mode::proto_type)
                   << "))] public ";
        }
        if (field.has_attribute("csEntityCollection"))
          *_source << "virtual ";
        *_source << make_tuple(scope, field.reference, print_mode::type) << " "
                 << field.attribute<std::string>("cs_name")
                 << " { get; set; }" br;
      }
      *_source << dec_indent << indent << "};" br;
    }
  }

  for (auto sub_namescope : scope.namescopes)
    writeMessageDefinitions(*sub_namescope);
}

bool cs_generator::writeMessageAsArgumentList(namescope& scope,
                                              message& message, bool firstField)
{
  if (message.base != nullptr)
  {
    firstField = writeMessageAsArgumentList(scope, *message.base, firstField);
  }
  for (auto& field : message.fields)
  {
    if (firstField)
      firstField = false;
    else
      *_source << ", ";
    *_source << field.attribute<std::string>("csArgumentName");
  }
  return firstField;
}

bool cs_generator::writeMessageAsParameterList(namescope& scope,
                                               message& message,
                                               bool firstField)
{
  if (message.base != nullptr)
  {
    firstField = writeMessageAsParameterList(scope, *message.base, firstField);
  }
  for (auto& field : message.fields)
  {
    if (firstField)
      firstField = false;
    else
      *_source << ", ";
    *_source << make_tuple(scope, field.reference, print_mode::type) << " "
             << field.attribute<std::string>("csArgumentName");
  }
  return firstField;
}

void cs_generator::writeInterfaceDefinitions(namescope& scope)
{
  bool hasInterfaces = false;
  for (auto* interface : scope.interfaces)
  {
    if (!interface->has_attribute("csExported") ||
        interface->attribute<std::string>("csExported") != "true")
    {
      hasInterfaces = true;
      break;
    }
  }

  if (hasInterfaces)
  {
    _source->switch_namescope(scope);

    bool firstInterface = true;
    for (auto* interface : scope.interfaces)
    {
      if (interface->has_attribute("csExported") &&
          interface->attribute<std::string>("csExported") == "true")
      {
        continue;
      }
      if (firstInterface)
        firstInterface = false;
      else
        *_source << br;

      if (interface->has_attribute("csCustomAttribute"))
      {
        *_source << indent << "["
                 << interface->attribute<std::string>("csCustomAttribute")
                 << "]" br;
      }
      *_source << indent << "public interface "
               << interface->attribute<std::string>("cs_name") << br;
      *_source << indent << "{" br << inc_indent;
      bool firstMessage = true;
      for (auto* message : interface->messages)
      {
        if (firstMessage)
          firstMessage = false;
        else
          *_source << br;
        *_source << indent << "void "
                 << message->attribute<std::string>("csEventName") << "(";
        bool firstParameter = true;
        for (auto& parameter : message->fields)
        {
          if (firstParameter)
            firstParameter = false;
          else
            *_source << ", ";
          *_source << make_tuple(scope, parameter.reference, print_mode::type)
                   << " " << parameter.attribute<std::string>("cs_name");
        }
        *_source << ");" br;
      }
      *_source << dec_indent << indent << "};" br;
    }
  }

  for (auto sub_namescope : scope.namescopes)
    writeInterfaceDefinitions(*sub_namescope);
}

bool cs_generator::requiresInterfaceSerializers(namescope& scope)
{
  for (auto* interface : scope.interfaces)
  {
    if (!interface->messages.empty())
      return true;
  }

  for (auto sub_namescope : scope.namescopes)
  {
    if (requiresInterfaceSerializers(*sub_namescope))
      return true;
  }
  return false;
}

bool cs_generator::requiresInterfaceDeserializers(namescope& scope)
{
  if (!scope.interfaces.empty())
    return true;

  for (auto sub_namescope : scope.namescopes)
  {
    if (requiresInterfaceDeserializers(*sub_namescope))
      return true;
  }
  return false;
}

void cs_generator::writeInterfaceDeserializers(namescope& scope)
{
  for (auto* interface : scope.interfaces)
  {
    auto deserializerName = interface->attribute<std::string>("csDeserializer");
    *_source << indent << "public sealed class " << deserializerName
             << " : IDeserializer" br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent
             << "public void Receive(MessageStream stream, "
                "object implementationBase)" br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent << "if (implementationBase == null)" br << inc_indent;
    *_source << indent << "return;" br << dec_indent;
    *_source << indent << "var implementation = ("
             << relative_name(*interface, _source->global_scope())
             << ")implementationBase;" br;

    *_source << indent << "stream.Read(out uint messageId);" br;
    *_source << indent << "switch (messageId)" br;
    *_source << indent << "{" br << inc_indent;
    for (auto* message : interface->messages)
    {
      BOOST_ASSERT(message->parent == interface);

      *_source << indent << "case 0x" << std::hex << std::setfill('0')
               << std::setw(8) << message->attribute<std::uint32_t>("uid")
               << ":" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "stream.Read(out "
               << relative_name(*message, _source->global_scope())
               << " _message, null, new Group());" br;

      *_source << indent << "implementation."
               << message->attribute<std::string>("csEventName") << "(";
      bool firstParameter = true;
      for (auto& field : message->fields)
      {
        if (!field.has_attribute("ignore"))
        {
          if (firstParameter)
            firstParameter = false;
          else
            *_source << ", ";
          *_source << "_message." << field.attribute<std::string>("cs_name");
        }
      }
      *_source << ");" br;

      *_source << indent << "break;" br;
      *_source << dec_indent << indent << "}" br;
    }
    *_source << indent << "default:" br << inc_indent;
    *_source << indent
             << "throw new NotImplementedException(\"Unknown message "
                "id \" + messageId + \".\");" br
             << dec_indent;
    *_source << dec_indent << indent << "}" br;
    *_source << dec_indent << indent << "}" br;

    *_source << dec_indent << indent << "}" br2;
  }

  for (auto sub_namescope : scope.namescopes)
    writeInterfaceDeserializers(*sub_namescope);
}

bool cs_generator::requiresServiceDefinitions(namescope& scope)
{
  if (!scope.services.empty())
    return true;

  for (auto sub_namescope : scope.namescopes)
  {
    if (requiresServiceDefinitions(*sub_namescope))
      return true;
  }
  return false;
}

void cs_generator::writeServices(namescope& scope)
{
  for (auto service : scope.services)
  {
    if (!_source->switch_namescope(scope))
      *_source << br;

    auto* service_interface = service->is_provider ? service->callback_interface
                                                   : service->service_interface;
    auto* callback_interface = service->is_provider
                                 ? service->service_interface
                                 : service->callback_interface;

    *_source << indent << "public sealed class "
             << service->attribute<std::string>("cs_name") << " :" br
             << inc_indent;
    *_source << indent << "Shift.Service.Service<"
             << relative_name(*service_interface, scope) << ", "
             << relative_name(*callback_interface, scope) << ">," br;
    *_source << indent << "IDeserializer" br << dec_indent;
    *_source << indent << "{" br << inc_indent;

    // Constructor.
    *_source << indent << "public "
             << service->attribute<std::string>("cs_name")
             << "(ushort service_tag = 0) :" br;
    *_source << indent << "    base("
             << (service->is_provider ? "true" : "false") << ", 0x" << std::hex
             << std::setfill('0') << std::setw(8)
             << service->attribute<std::uint32_t>("uid") << ", service_tag)" br;
    *_source << indent << "{" br << inc_indent;
    *_source << indent << "Deserializer = this;" br;
    *_source << dec_indent << indent << "}" br2;

    // Write post methods for each outgoing message.
    for (auto* message : service_interface->messages)
    {
      *_source << indent << "public void Broadcast("
               << relative_name(*message, scope) << " message)" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "base.Broadcast(message);" br;
      *_source << dec_indent << indent << "}" br2;

      *_source << indent << "public void Reply("
               << relative_name(*message, scope) << " message)" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "base.Reply(message);" br;
      *_source << dec_indent << indent << "}" br2;

      *_source << indent << "public void SendTo(ulong remote_service_guid, "
               << relative_name(*message, scope) << " message)" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "base.SendTo(remote_service_guid, message);" br;
      *_source << dec_indent << indent << "}" br2;
    }

    // Implement IDeserializer.Receive.
    *_source << indent
             << "public void Receive(MessageStream stream, "
                "object implementationBase)" br;
    *_source << indent << "{" br << inc_indent;

    *_source << indent << "stream.Read(out uint messageId);" br;
    *_source << indent << "switch (messageId)" br;
    *_source << indent << "{" br << inc_indent;
    for (auto* message : callback_interface->messages)
    {
      BOOST_ASSERT(message->parent == callback_interface);

      *_source << indent << "case 0x" << std::hex << std::setfill('0')
               << std::setw(8) << message->attribute<std::uint32_t>("uid")
               << ":" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent << "var serializerStartTime = DateTime.Now;" br;
      *_source << indent << "stream.Read(out " << relative_name(*message, scope)
               << " message, null, new Group());" br;
      *_source << indent
               << "if (ServiceHost.Instance.DebugSerializationPerformance)" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent
               << "var deserializationTime = DateTime.Now - "
                  "serializerStartTime;" br;
      *_source << indent
               << "if (deserializationTime > TimeSpan.FromMilliseconds(50))" br;
      *_source << indent
               << "   Console.WriteLine(string.Format(\"Performance "
                  "Warning: Deserialization of message {0} took {1}ms.\", \""
               << service->attribute<std::string>("cs_name") << "."
               << message->attribute<std::string>("csEventName")
               << "()\", deserializationTime.TotalMilliseconds));" br;
      *_source << dec_indent << indent << "}" br;
      *_source << indent << "if (ServiceHost.Instance.DebugMessages)" br;
      *_source << indent << "   Console.WriteLine(\""
               << service->attribute<std::string>("cs_name") << "."
               << message->attribute<std::string>("csEventName") << "()\");" br;
      *_source << indent << "var handler = "
               << message->attribute<std::string>("csEventName") << ";" br;

      *_source << indent << "var handlerStartTime = DateTime.Now;" br;
      *_source << indent << "handler?.Invoke(this, message);" br;
      *_source << indent
               << "if (ServiceHost.Instance.DebugMessagePerformance)" br;
      *_source << indent << "{" br << inc_indent;
      *_source << indent
               << "var handlerTime = DateTime.Now - handlerStartTime;" br;
      *_source << indent
               << "if (handlerTime > TimeSpan.FromMilliseconds(50))" br;
      *_source << indent
               << "   Console.WriteLine(string.Format(\"Performance "
                  "Warning: Processing of message {0} took {1}ms.\", \""
               << service->attribute<std::string>("cs_name") << "."
               << message->attribute<std::string>("csEventName")
               << "()\", handlerTime.TotalMilliseconds));" br;
      *_source << dec_indent << indent << "}" br;
      *_source << indent << "break;" br;
      *_source << dec_indent << indent << "}" br;
    }
    *_source << indent << "default:" br << inc_indent;
    *_source << indent
             << "throw new NotImplementedException(\"Unknown message "
                "id \" + messageId + \".\");" br
             << dec_indent;
    *_source << dec_indent << indent << "}" br;
    *_source << dec_indent << indent << "}" br;

    // Write message events.
    bool firstEvent = true;
    for (auto* message : callback_interface->messages)
    {
      if (firstEvent)
      {
        firstEvent = false;
        *_source << br;
      }
      *_source << indent << "public event EventHandler<"
               << relative_name(*message, scope) << "> "
               << message->attribute<std::string>("csEventName") << ";" br;
    }
    *_source << dec_indent << indent << "}" br;
  }

  for (auto sub_namescope : scope.namescopes)
    writeServices(*sub_namescope);
}

bool cs_generator::requiresTypeRegistry(namescope& scope)
{
  if (!scope.interfaces.empty())
    return true;

  for (auto sub_namescope : scope.namescopes)
  {
    if (requiresTypeRegistry(*sub_namescope))
      return true;
  }
  return false;
}

void cs_generator::writeTypeRegistry(namescope& scope)
{
  for (auto* interface : scope.interfaces)
  {
    *_source << indent << "host.RegisterInterface<"
             << relative_name(*interface, _source->global_scope()) << ">(" br
             << inc_indent;
    *_source << indent << "0x" << std::hex << std::setfill('0') << std::setw(8)
             << interface->attribute<std::uint32_t>("uid") << ", new "
             << interface->attribute<std::string>("csDeserializer") << "());" br
             << dec_indent;
  }

  for (auto sub_namescope : scope.namescopes)
    writeTypeRegistry(*sub_namescope);
}
}
