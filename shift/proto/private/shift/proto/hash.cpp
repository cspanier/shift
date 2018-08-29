#include "shift/proto/hash.h"
#include "shift/proto/proto.h"

namespace shift::crypto
{
sha256& operator<<(sha256& context,
                   const ::shift::proto::type_reference& type_reference)
{
  if (const auto* built_in_type = type_reference.as_built_in_type())
    context << static_cast<unsigned char>(*built_in_type);
  else if (const auto* alias = type_reference.as_alias())
    context << *alias;
  else if (const auto* enumeration = type_reference.as_enumeration())
    context << *enumeration;
  else if (const auto* message = type_reference.as_message())
    context << *message;
  else
    BOOST_ASSERT(false);
  return context;
}

sha256& operator<<(sha256& context, const ::shift::proto::alias& alias)
{
  return context << alias.name;
  /// ToDo: Also consider the alias.type.
}

sha256& operator<<(sha256& context, const ::shift::proto::enumerator& member)
{
  return context << member.name << "=" << member.value;
}

sha256& operator<<(sha256& context,
                   const ::shift::proto::enumeration& enumeration)
{
  context << enumeration.name;

  // Take the name scope path into account.
  int index = 0;
  auto parent_namescope = enumeration.parent;
  while (parent_namescope && !parent_namescope->name.empty())
  {
    context << parent_namescope->name << index++;
    parent_namescope = parent_namescope->parent;
  }

  // Take all names and values into account.
  for (const auto& member : enumeration.members)
    context << member;

  return context;
}

sha256& operator<<(sha256& context, const ::shift::proto::field& field)
{
  return context << field.name << field.reference;
}

sha256& operator<<(sha256& context, const ::shift::proto::message& message)
{
  context << message.name;

  // Take the name scope path into account.
  int index = 0;
  auto parent_namescope = message.parent;
  while (parent_namescope && !parent_namescope->name.empty())
  {
    context << parent_namescope->name << index++;
    parent_namescope = parent_namescope->parent;
  }

  // Tage the base type into account.
  if (message.base)
    context << *message.base;

  // Take all member fields into account.
  index = 0;
  for (const auto& field : message.fields)
    context << field << index++;

  return context;
}

sha256& operator<<(sha256& context, const ::shift::proto::interface& interface)
{
  context << interface.name;

  // Take the name scope path into account.
  int index = 0;
  auto parent_namescope = interface.parent;
  while (parent_namescope && !parent_namescope->name.empty())
  {
    context << parent_namescope->name << index++;
    parent_namescope = parent_namescope->parent;
  }

  // Take all messages into account.
  for (const auto* message : interface.messages)
    context << *message;

  return context;
}

sha256& operator<<(sha256& context, const ::shift::proto::service& service)
{
  // Take full service interface and callback interface names into account.
  if (service.service_interface)
  {
    context << service.service_interface->name;

    int index = 0;
    auto parent_namescope = service.service_interface->parent;
    while (parent_namescope && !parent_namescope->name.empty())
    {
      context << parent_namescope->name << index++;
      parent_namescope = parent_namescope->parent;
    }
  }
  if (service.callback_interface)
  {
    context << service.callback_interface->name;

    int index = 0;
    auto parent_namescope = service.callback_interface->parent;
    while (parent_namescope && !parent_namescope->name.empty())
    {
      context << parent_namescope->name << index++;
      parent_namescope = parent_namescope->parent;
    }
  }

  return context;
}

sha256& operator<<(sha256& context, const ::shift::proto::namescope& namescope)
{
  context << namescope.name;

  // Take the name scope path into account.
  int index = 0;
  auto parent_namescope = namescope.parent;
  while (parent_namescope && !parent_namescope->name.empty())
  {
    context << parent_namescope->name << index++;
    parent_namescope = parent_namescope->parent;
  }

  return context;
}
}
