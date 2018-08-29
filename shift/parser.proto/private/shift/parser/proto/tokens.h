#ifndef SHIFT_PARSER_PROTO_TOKENS_H
#define SHIFT_PARSER_PROTO_TOKENS_H

#include <cstdint>
#include <string>
#include <vector>
#include <variant>
#include "shift/parser/proto/types.h"
#include "shift/parser/proto/annotation_info.h"

namespace shift::parser::proto
{
/// An empty placeholder type used to signal that certain variants below don't
/// hold any value.
struct no_value_t
{
};

///
struct string_token : public annotation_info
{
  std::string value;
};

using template_argument_token = std::variant<type_path_token, std::int64_t>;
using template_argument_tokens = std::vector<template_argument_token>;

///
struct type_path_element_token : public annotation_info
{
  string_token name;
  template_argument_tokens template_arguments;
};

///
struct type_path_token : public std::vector<type_path_element_token>,
                         public annotation_info
{
};

///
struct comment_token : public std::string, public annotation_info
{
};
using comments_token = std::vector<comment_token>;

///
struct type_attribute_token : public annotation_info
{
  string_token identifier;
  std::variant<string_token, std::uint64_t> value;
};

///
struct type_attribute_tokens : public std::vector<type_attribute_token>,
                               public annotation_info
{
};

///
struct enumerant_reference_token : public annotation_info
{
  type_path_token name;
  string_token enumerant;
};

/// Template parameters used in type definitions.
struct template_parameter_token : public annotation_info
{
  type_path_token type;
  bool is_typename;
  bool is_variadic;
  string_token identifier;
  std::variant<type_path_token, std::int64_t, std::uint64_t, std::nullptr_t>
    default_value;
};

///
struct template_parameter_tokens : public std::vector<template_parameter_token>,
                                   public annotation_info
{
};

///
struct meta_token : public annotation_info
{
  comments_token comments;
  type_attribute_tokens attributes;
};

/// Base class for all types, data fields, and enumerants.
struct node_token : public annotation_info
{
  node_token* parent = nullptr;
  meta_token meta;
  string_token identifier;
  template_parameter_tokens template_parameters;

  /// A direct reference to the assiciated public AST node which was created
  /// from this token node. This speeds up AST building.
  ast::type_node* ast_node = nullptr;
};

using definition_token = std::variant<namescope_token, alias_token,
                                      enumeration_token, structure_token>;
using definition_tokens = std::vector<definition_token>;

///
struct namescope_token : public node_token
{
  definition_tokens children;
};

///
struct alias_token : public node_token
{
  type_path_token reference;
};

///
using enumerant_value =
  std::variant<no_value_t, string_token, enumerant_reference_token,
               std::uint64_t, std::int64_t>;

///
struct enumerant_token : public node_token
{
  enumerant_value value;
};
using enumerant_tokens = std::vector<enumerant_token>;

///
struct enumeration_token : public node_token
{
  type_path_token base;
  enumerant_tokens enumerants;
};

using field_value =
  std::variant<no_value_t, std::nullptr_t, string_token,
               enumerant_reference_token, std::uint64_t, std::int64_t>;

///
struct field_token : public node_token
{
  bool is_const;
  type_path_token reference;
  field_value default_value;
};
using field_tokens = std::vector<field_token>;

///
struct structure_token : public node_token
{
  type_path_token base;
  field_tokens fields;
};

///
std::ostream& operator<<(std::ostream& stream, const string_token& value);

///
std::ostream& operator<<(std::ostream& stream, const type_path_token& value);

///
std::ostream& operator<<(std::ostream& stream, const namescope_token& value);

///
std::ostream& operator<<(std::ostream& stream, const alias_token& value);

///
std::ostream& operator<<(std::ostream& stream, const enumeration_token& value);

///
std::ostream& operator<<(std::ostream& stream, const structure_token& value);
}

#endif
