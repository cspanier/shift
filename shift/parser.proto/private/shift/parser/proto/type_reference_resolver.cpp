#include "shift/parser/proto/type_reference_resolver.h"
#include "shift/parser/proto/document.h"
#include "shift/parser/proto/document_impl.h"
#include "shift/parser/proto/error.h"

namespace shift::parser::proto
{
type_reference_resolver::type_reference_resolver(document_impl& impl)
: _impl(impl)
{
}

type_reference_resolver::return_type type_reference_resolver::operator()(
  namescope_token& token)
{
  // Recursively walk through all children.
  for (auto& child : token.children)
  {
    if (!std::visit(*this, child))
      return false;
  }
  return true;
}

type_reference_resolver::return_type type_reference_resolver::operator()(
  alias_token& token)
{
  auto* alias_node = token.ast_node->as_alias();
  BOOST_ASSERT(alias_node);
  if (!alias_node)
  {
    _impl.error_stream << error_internal{token} << std::endl;
    return false;
  }

  alias_node->reference.type =
    find_type(alias_node->parent_namescope, token.reference);
  if (std::get_if<ast::invalid_type>(&alias_node->reference.type))
  {
    _impl.error_stream << error_type_lookup{token.reference} << std::endl;
    return false;
  }
  // alias_node->reference.arguments = token.reference.template_arguments;
  return true;
}

type_reference_resolver::return_type type_reference_resolver::operator()(
  enumeration_token& /*token*/)
{
  return true;
}

type_reference_resolver::return_type type_reference_resolver::operator()(
  structure_token& token)
{
  auto* structure_node = token.ast_node->as_structure();
  BOOST_ASSERT(structure_node);
  if (!structure_node)
  {
    _impl.error_stream << error_internal{token} << std::endl;
    return false;
  }

  if (!token.base.empty())
  {
    auto base_type = find_type(structure_node->parent_namescope, token.base);
    if (!std::get_if<ast::structure_node*>(&base_type))
    {
      _impl.error_stream << error_type_lookup{token.base} << std::endl;
      return false;
    }
    structure_node->base = std::get<ast::structure_node*>(base_type);
  }

  auto result = true;

  for (auto* field_node : structure_node->fields)
  {

    BOOST_ASSERT(field_node->token);
    if (!field_node->token)
    {
      _impl.error_stream << error_internal{token} << std::endl;
      return false;
    }
    auto& field = *field_node->token;

    field_node->reference.type =
      find_type(structure_node->parent_namescope, field.reference);

    if (std::get_if<ast::invalid_type>(&field_node->reference.type))
    {
      _impl.error_stream << error_type_lookup{field.reference} << std::endl;
      result = false;
    }
    // field_node->reference.arguments = field.reference.template_arguments;
  }

  return result;
}

ast::type_variant type_reference_resolver::find_type(
  ast::namescope_node* namescope, const type_path_token& path)
{
  if (path.size() == 1)
  {
    const auto& type_name = path[0];
    if (type_name.name.value == "var_sint_t")
      return ast::built_in_type::var_sint_t;
    if (type_name.name.value == "var_uint_t")
      return ast::built_in_type::var_uint_t;
    if (type_name.name.value == "fixed_sint_t")
      return ast::built_in_type::fixed_sint_t;
    if (type_name.name.value == "fixed_uint_t")
      return ast::built_in_type::fixed_uint_t;
    if (type_name.name.value == "floating_point_t")
      return ast::built_in_type::floating_point;
    if (type_name.name.value == "repeated_t")
      return ast::built_in_type::repeated;
  }
  return find_type_node(namescope, path);
}

ast::type_variant type_reference_resolver::find_type_node(
  ast::namescope_node* namescope, const type_path_token& path,
  std::size_t current_depth)
{
  if (!namescope)
    return ast::invalid_type{};
  const std::string_view name = path[current_depth].name.value;
  if (current_depth == path.size() - 1)
  {
    if (auto* alias = namescope->alias(name))
      return alias;
    else if (auto* enumeration = namescope->enumeration(name))
      return enumeration;
    else if (auto* structure = namescope->structure(name))
      return structure;
  }
  else if (auto* sub_namescope = namescope->namescope(name))
  {
    auto result = find_type_node(sub_namescope, path, current_depth + 1);
    if (!std::get_if<ast::invalid_type>(&result))
      return result;
  }

  if (current_depth == 0)
    return find_type_node(namescope->parent_namescope, path);
  else
    return ast::invalid_type{};
}
}
