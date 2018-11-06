#include "shift/proto/semanticanalyzer.hpp"
#include "shift/proto/proto.hpp"
#include <shift/log/log.hpp>
#include <shift/core/string_util.hpp>
#include <set>

namespace std
{
bool operator<(std::reference_wrapper<const std::string> lhs,
               std::reference_wrapper<const std::string> rhs)
{
  return lhs.get() < rhs.get();
}
}

namespace shift::proto
{
/// A helper visitor class that collects names from all types except namescope
/// instances, and returns whether the name is unique among all previously
/// checked names.
struct ambiguous_name_checker
{
  using return_type = bool;

  ///
  bool operator()(const namescope& /*namescope*/)
  {
    return true;
  }

  ///
  bool operator()(const node& alias)
  {
    return register_name(alias.name);
  }

  bool register_name(std::string_view name)
  {
    if (!names.insert(name).second)
    {
      log::error() << "Type name \"" << name << "\" is ambiguous.";
      return false;
    }
    return true;
  }

  std::set<std::string_view> names;
};

bool semantic_analyzer::analyze_namescope(namescope& namescope) const
{
  // Check whether there are any colliding type names within the current name
  // scope.
  bool result = true;
  ambiguous_name_checker name_checker;
  for (auto& child : namescope.dynamic_children)
    result &= std::visit(name_checker, *child);

  for (auto* alias : namescope.aliases)
    resolve_type(namescope, alias->reference);

  for (auto* enumeration : namescope.enumerations)
  {
    if (!resolve_type(namescope, enumeration->base))
    {
      log::error() << "Cannot find base type of enumeration \""
                   << enumeration->name << "\".";
      result = false;
      continue;
    }

    auto* built_in_type = enumeration->base.as_built_in_type(true);
    if ((built_in_type == nullptr) ||
        (!built_in_type_traits::is_int(*built_in_type) &&
         !built_in_type_traits::is_char(*built_in_type)))
    {
      log::error() << "Enumeration \"" << enumeration->name
                   << "\" can only be of integer or character type.";
      result = false;
      continue;
    }
  }

  for (auto* message : namescope.messages)
  {
    if (message->base_name.empty())
      message->base = nullptr;
    else
    {
      // Check if the base type name exists.
      auto* baseNode = namescope.find_type(message->base_name);
      if (baseNode == nullptr)
      {
        log::error() << "Cannot find base type \""
                     << core::implode(message->base_name, ".")
                     << "\" of type \"" << message->name << "\".";
        result = false;
      }
      else if (auto* baseMessage = std::get_if<proto::message>(baseNode))
        message->base = baseMessage;
      else
      {
        log::error() << "Base type \"" << core::implode(message->base_name, ".")
                     << "\" of type \"" << message->name
                     << "\" is not a message.";
        result = false;
      }
    }

    for (auto& field : message->fields)
      result &= resolve_type(namescope, field.reference);
  }

  for (auto* service : namescope.services)
  {
    auto* serviceNode = namescope.find_type(service->service_name);
    if (serviceNode == nullptr)
    {
      log::error() << "Cannot find service interface type \""
                   << core::implode(service->service_name, ".")
                   << "\" used for definition of service \"" << service->name
                   << "\".";
      result = false;
    }
    else if (auto* interface = std::get_if<proto::interface>(serviceNode))
      service->service_interface = interface;
    else
    {
      log::error()
        << "Type \"" << core::implode(service->service_name, ".")
        << "\" used for service interface for definition of service \""
        << service->name << "\" is not supported.";
      result = false;
    }

    auto* callback_node = namescope.find_type(service->callback_name);
    if (callback_node == nullptr)
    {
      log::error() << "Cannot find callback interface type \""
                   << core::implode(service->callback_name, ".")
                   << "\" used for definition of service \"" << service->name
                   << "\".";
      result = false;
    }
    else if (auto* interface = std::get_if<proto::interface>(callback_node))
      service->callback_interface = interface;
    else
    {
      log::error()
        << "Type \"" << core::implode(service->callback_name, ".")
        << "\" used for service interface for definition of service \""
        << service->name << "\" is not supported.";
      result = false;
    }
  }

  for (auto* sub_namescope : namescope.namescopes)
    result &= analyze_namescope(*sub_namescope);
  return result;
}

bool semantic_analyzer::resolve_type(namescope& namescope,
                                     type_reference& type_reference) const
{
  bool result = true;
  for (auto& templateArgument : type_reference.arguments)
  {
    if (auto* templateType =
          std::get_if<proto::type_reference>(&templateArgument))
    {
      result &= resolve_type(namescope, *templateType);
    }
  }

  if (const auto* type_path =
        std::get_if<proto::type_path>(&type_reference.name))
  {
    auto* field_node = namescope.find_type(*type_path);
    if (field_node == nullptr)
    {
      log::error() << "Unknown type \"" << core::implode(*type_path, ".")
                   << "\".";
      return false;
    }
    if (const auto* alias = std::get_if<proto::alias>(field_node))
      type_reference.variant = alias;
    else if (const auto* enumeration =
               std::get_if<proto::enumeration>(field_node))
    {
      type_reference.variant = enumeration;
    }
    else if (const auto* message = std::get_if<proto::message>(field_node))
      type_reference.variant = message;
    else
    {
      log::error() << "Use of unsupported type \""
                   << core::implode(*type_path, ".") << "\".";
      return false;
    }
  }
  else if (const auto* built_in_type =
             std::get_if<proto::built_in_type>(&type_reference.name))
  {
    type_reference.variant = *built_in_type;
  }
  else
  {
    BOOST_ASSERT(false);  // Can't happen if the grammar is well defined.
    return false;
  }

  return result;
}
}
