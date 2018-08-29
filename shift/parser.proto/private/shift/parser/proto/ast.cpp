#include "shift/parser/proto/ast.h"

namespace shift::parser::proto::ast
{
type_node::~type_node() = default;

namescope_node* type_node::as_namescope()
{
  return dynamic_cast<namescope_node*>(this);
}

const namescope_node* type_node::as_namescope() const
{
  return dynamic_cast<const namescope_node*>(this);
}

alias_node* type_node::as_alias()
{
  return dynamic_cast<alias_node*>(this);
}

const alias_node* type_node::as_alias() const
{
  return dynamic_cast<const alias_node*>(this);
}

enumerant_node* type_node::as_enumerant()
{
  return dynamic_cast<enumerant_node*>(this);
}

const enumerant_node* type_node::as_enumerant() const
{
  return dynamic_cast<const enumerant_node*>(this);
}

enumeration_node* type_node::as_enumeration()
{
  return dynamic_cast<enumeration_node*>(this);
}

const enumeration_node* type_node::as_enumeration() const
{
  return dynamic_cast<const enumeration_node*>(this);
}

field_node* type_node::as_field()
{
  return dynamic_cast<field_node*>(this);
}

const field_node* type_node::as_field() const
{
  return dynamic_cast<const field_node*>(this);
}

structure_node* type_node::as_structure()
{
  return dynamic_cast<structure_node*>(this);
}

const structure_node* type_node::as_structure() const
{
  return dynamic_cast<const structure_node*>(this);
}

namescope_node* namescope_node::namescope(std::string_view name) const
{
  auto result = namescopes.find(name);
  if (result != namescopes.end())
    return result->second;
  else
    return nullptr;
}

alias_node* namescope_node::alias(std::string_view name) const
{
  auto result = aliases.find(name);
  if (result != aliases.end())
    return result->second;
  else
    return nullptr;
}

enumeration_node* namescope_node::enumeration(std::string_view name) const
{
  auto result = enumerations.find(name);
  if (result != enumerations.end())
    return result->second;
  else
    return nullptr;
}

structure_node* namescope_node::structure(std::string_view name) const
{
  auto result = structures.find(name);
  if (result != structures.end())
    return result->second;
  else
    return nullptr;
}

const field_node* structure_node::field(std::string_view name) const
{
  auto result =
    std::find_if(fields.begin(), fields.end(),
                 [&](const auto* field) { return field->identifier == name; });
  if (result != fields.end())
    return *result;
  else if (base)
    return base->field(name);
  else
    return nullptr;
}
}
