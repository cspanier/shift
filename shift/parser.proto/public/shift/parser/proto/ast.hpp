#ifndef SHIFT_PARSER_PROTO_AST_HPP
#define SHIFT_PARSER_PROTO_AST_HPP

#include <string>
#include <string_view>
#include <vector>
#include <unordered_set>
#include <variant>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/parser/proto/types.hpp"

namespace shift::parser::proto::ast
{
namespace bmi = boost::multi_index;

struct invalid_type
{
};

///
enum class built_in_type : std::uint8_t
{
  var_sint_t,
  var_uint_t,
  fixed_sint_t,
  fixed_uint_t,
  floating_point,
  repeated,
  selected,
  optional
};

using template_argument = std::variant<type_path_node, std::int64_t>;
using template_arguments = std::vector<template_argument>;

struct type_attribute_node
{
  std::string_view identifier;
  std::variant<std::string_view, std::uint64_t> value;
};
using type_attribute_nodes = std::vector<type_attribute_node>;

using type_variant = std::variant<invalid_type, built_in_type, alias_node*,
                                  enumeration_node*, structure_node*>;

///
struct type_path_node
{
  ///// Returns a pointer to a built-in type if this type represents one,
  ///// otherwise nullptr is returned.
  ///// @param recursive
  /////   Resolve the alias recursively, in case it points to another alias.
  // const proto::built_in_type* as_built_in_type(
  //  bool recursive = false) const;

  ///// Returns a pointer to an alias type if this type represents one,
  ///// otherwise nullptr is returned.
  // const alias* as_alias() const;

  ///// Returns a pointer to an enumeration type if this type represents
  /// one,
  ///// otherwise nullptr is returned.
  ///// @param recursive
  /////   Resolve the alias recursively, in case it points to another alias.
  // const enumeration* as_enumeration(bool recursive = false) const;

  ///// Returns a pointer to a structure type if this type represents one,
  ///// otherwise nullptr is returned.
  ///// @param recursive
  /////   Resolve the alias recursively, in case it points to another alias.
  // const structure* as_structure(bool recursive = false) const;

  const type_path_token* token = nullptr;
  type_variant type;
  template_arguments arguments;
};

using template_parameter_value =
  std::variant<type_path_node, std::int64_t, std::uint64_t, std::nullptr_t>;

/// Template parameters used in type definitions.
struct template_parameter_node
{
  const template_parameter_token* token = nullptr;
  bool is_typename;
  bool is_variadic;
  std::string_view identifier;
  template_parameter_value default_value = nullptr;
};
using template_parameter_nodes = std::vector<template_parameter_node>;

enum class attribute_assignment
{
  keep,
  overwrite
};

enum class attribute_visibility
{
  internal,
  published
};

/// Base class for all types, data fields, and enumerants.
struct type_node
{
  /// Destructor.
  virtual ~type_node();

  ///
  namescope_node* as_namescope();

  ///
  const namescope_node* as_namescope() const;

  ///
  alias_node* as_alias();

  ///
  const alias_node* as_alias() const;

  ///
  enumerant_node* as_enumerant();

  ///
  const enumerant_node* as_enumerant() const;

  ///
  enumeration_node* as_enumeration();

  ///
  const enumeration_node* as_enumeration() const;

  ///
  field_node* as_field();

  ///
  const field_node* as_field() const;

  ///
  structure_node* as_structure();

  ///
  const structure_node* as_structure() const;

  ///// Returns whether an attribute of given name exists.
  // bool has_attribute(const std::string& attribute_name) const;

  ///// Conditionally overwrites an attribute of given name.
  // bool attribute(
  //  const std::string& attribute_name, const std::string& value,
  //  attribute_assignment assignment = attribute_assignment::overwrite,
  //  attribute_visibility visibility = attribute_visibility::internal);

  // bool attribute(
  //  const std::string& attribute_name, std::uint64_t value,
  //  attribute_assignment assignment = attribute_assignment::overwrite,
  //  attribute_visibility visibility = attribute_visibility::internal);

  ///// Conditionally overwrites an attribute of given name.
  // bool attribute(
  //  attribute_pair attribute,
  //  attribute_assignment assignment = attribute_assignment::overwrite,
  //  attribute_visibility visibility = attribute_visibility::internal);

  ///// Returns the value of the attribute of given name.
  ///// @exception std::invalid_argument
  /////   There is no attribute of given name.
  // template <typename T>
  // T attribute(const std::string& attribute_name) const
  //{
  //  auto attribute = public_attributes.find(attribute_name);
  //  if (attribute == public_attributes.end())
  //  {
  //    attribute = internal_attributes.find(attribute_name);
  //    if (attribute == internal_attributes.end())
  //    {
  //      throw std::invalid_argument("attribute \"" + attribute_name +
  //                                  "\" does not exist in type \"" +
  //                                  identifier.name + "\".");
  //    }
  //  }
  //  if (auto* string = core::get<std::string>(&attribute->second))
  //    return boost::lexical_cast<T>(*string);
  //  else if (auto* number = core::get<std::uint64_t>(&attribute->second))
  //    return boost::lexical_cast<T>(*number);
  //  else
  //  {
  //    throw std::invalid_argument("attribute \"" + attribute_name +
  //                                "\" is invalid in type \"" +
  //                                identifier.name + "\".");
  //  }
  //}

  ///// Recursively traverses the AST and generates uid attributes for each
  ///// type which doesn't already have one.
  // virtual void generate_uids() = 0;

  type_node* this_type = nullptr;
  std::string_view identifier;
  template_parameter_nodes template_parameters;
  namescope_node* parent_namescope = nullptr;
  std::uint32_t depth = 0;
  type_attribute_nodes attributes;
  std::string comments;
};

using definition_node =
  std::variant<namescope_node, alias_node, enumeration_node, structure_node>;
using definition_nodes = std::vector<definition_node>;

struct by_identifier
{
};

///
struct namescope_node : public type_node
{
  /// Returns a namescope by name, or nullptr if no such namescope exists.
  namescope_node* namescope(std::string_view name) const;

  /// Returns an alias by name, or nullptr if no such alias exists.
  alias_node* alias(std::string_view name) const;

  /// Returns an enumeration by name, or nullptr if no such enumeration exists.
  enumeration_node* enumeration(std::string_view name) const;

  /// Returns a structure by name, or nullptr if no such structure exists.
  structure_node* structure(std::string_view name) const;

  using children_t = boost::multi_index_container<
    type_node*,
    bmi::indexed_by<bmi::ordered_unique<
      bmi::tag<by_identifier>,
      bmi::member<type_node, std::string_view, &type_node::identifier>>>>;

  ///// @see type_node::generate_uids.
  // virtual void generate_uids() override;

  const namescope_token* token = nullptr;
  children_t children;
  std::unordered_map<std::string_view, namescope_node*> namescopes;
  std::unordered_map<std::string_view, alias_node*> aliases;
  std::unordered_map<std::string_view, enumeration_node*> enumerations;
  std::unordered_map<std::string_view, structure_node*> structures;
};

struct alias_node : public type_node
{
  ///// @see type_node::generate_uids.
  // virtual void generate_uids() override;

  ///// Returns the actual type referenced, which is different from
  ///// type_path_node when chaining aliases.
  // const type_path_node& actual_type_reference() const;

  const alias_token* token = nullptr;
  type_path_node reference;
};

/// ToDo: It makes no sense to derive enumerant_node from type_node.
struct enumerant_node : public type_node
{
  ///// @see type_node::generate_uids.
  // void generate_uids() override;

  const enumerant_token* token = nullptr;
  std::int64_t value;
};

using enumerant_nodes = std::vector<enumerant_node>;

///
struct enumeration_node : public type_node
{
  ///// @see type_node::generate_uids.
  // virtual void generate_uids() override;

  const enumeration_token* token = nullptr;
  type_path_node base;
  enumerant_nodes enumerants;
};

/// An empty type used in a field's value variant to signal that it does not
/// hold any value.
struct no_value_t
{
};

using field_value =
  std::variant<no_value_t, enumerant_node*, std::int64_t, std::nullptr_t>;

/// ToDo: It doesn't make much sense to derive field from type_node.
struct field_node : public type_node
{
  ///// @see type_node::generate_uids.
  // virtual void generate_uids() override;

  const field_token* token = nullptr;
  const structure_node* parent_structure = nullptr;
  bool is_const = false;
  type_path_node reference;
  field_value default_value;
};

///
struct structure_node : public type_node
{
  ///// @see type_node::generate_uids.
  // virtual void generate_uids() override;

  ///// Returns whether this structure or its base type have any fields.
  // bool has_fields() const;

  /// Returns a field by name, or nullptr of no such field exists.
  const field_node* field(std::string_view name) const;

  const structure_token* token = nullptr;
  structure_node* base = nullptr;
  std::vector<field_node*> fields;
};
}

#endif
