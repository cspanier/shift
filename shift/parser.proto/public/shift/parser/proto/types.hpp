#ifndef SHIFT_PARSER_PROTO_TYPES_HPP
#define SHIFT_PARSER_PROTO_TYPES_HPP

namespace shift::parser::proto
{
struct template_parameter_token;
struct namescope_token;
struct alias_token;
struct enumerant_token;
struct enumeration_token;
struct field_token;
struct structure_token;
struct type_path_token;

namespace ast
{
  struct type_node;
  struct template_parameter_node;
  struct namescope_node;
  struct alias_node;
  struct enumerant_node;
  struct enumeration_node;
  struct field_node;
  struct structure_node;
  struct type_path_node;
}
}

#endif
