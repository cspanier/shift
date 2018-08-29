#include "shift/parser/proto/token_to_ast_visitor.h"
#include "shift/parser/proto/document.h"
#include "shift/parser/proto/document_impl.h"
#include "shift/parser/proto/error.h"

namespace shift::parser::proto
{
token_to_ast_visitor::token_to_ast_visitor(namescope_token& parent_token,
                                           ast::namescope_node& parent_node,
                                           document_impl& impl)
: _parent_token(parent_token), _parent_node(parent_node), _impl(impl)
{
}

token_to_ast_visitor::return_type token_to_ast_visitor::operator()(
  namescope_token& token)
{
  token.parent = &_parent_token;

  ast::namescope_node* node = nullptr;
  auto& children_by_identifier =
    _parent_node.children.get<ast::by_identifier>();
  auto node_iterator = children_by_identifier.find(token.identifier.value);
  if (node_iterator == children_by_identifier.end())
  {
    // Create new namespace_node.
    node = _impl.namescope_pool.create();
    node->parent_namescope = &_parent_node;
    node->identifier = token.identifier.value;
    // node->template_parameters =
    // node->attributes =
    node->token = &token;
    token.ast_node = node;
    _parent_node.children.insert(node);
    bool success =
      _parent_node.namescopes.insert_or_assign(node->identifier, node).second;
    BOOST_ASSERT(success);
    (void)success;
  }
  else if (auto* other_namescope = (*node_iterator)->as_namescope())
  {
    // There is a different namescope with the same name, which is
    // allowed. All elements of this namescope_token will be merged into
    // the existing ast::namescope_node.
    node = other_namescope;
    token.ast_node = other_namescope;
    /// ToDo: We lose information that this namescope was merged.
  }
  else
  {
    handle_name_collision(token.identifier, *node_iterator);
    return false;
  }
  BOOST_ASSERT(node);

  // Comments on different namescope definition locations will be merged
  // together.
  for (const auto& comment : token.meta.comments)
  {
    if (node->comments.empty())
      node->comments = comment;
    else
    {
      node->comments = node->comments + '\n' + comment;
    }
  }

  // Recursively translate all children.
  for (auto& child : token.children)
  {
    if (!std::visit(token_to_ast_visitor{token, *node, _impl}, child))
      return false;
  }
  return true;
}

token_to_ast_visitor::return_type token_to_ast_visitor::operator()(
  alias_token& token)
{
  token.parent = &_parent_token;

  ast::alias_node* node = nullptr;
  auto& children_by_identifier =
    _parent_node.children.get<ast::by_identifier>();
  auto node_iterator = children_by_identifier.find(token.identifier.value);
  if (node_iterator == children_by_identifier.end())
  {
    // Create new ast::alias_node.
    node = _impl.alias_pool.create();
    node->parent_namescope = &_parent_node;
    node->identifier = token.identifier.value;
    translate_template_parameters(*node, token);
    // node->attributes =
    node->token = &token;
    token.ast_node = node;
    _parent_node.children.insert(node);
    bool success =
      _parent_node.aliases.insert_or_assign(node->identifier, node).second;
    BOOST_ASSERT(success);
    (void)success;
  }
  else
  {
    handle_name_collision(token.identifier, *node_iterator);
    return false;
  }
  BOOST_ASSERT(node);
  return true;
}

token_to_ast_visitor::return_type token_to_ast_visitor::operator()(
  enumeration_token& token)
{
  token.parent = &_parent_token;
  for (auto& enumerant : token.enumerants)
    enumerant.parent = &token;

  ast::enumeration_node* node = nullptr;
  auto& children_by_identifier =
    _parent_node.children.get<ast::by_identifier>();
  auto node_iterator = children_by_identifier.find(token.identifier.value);
  if (node_iterator == children_by_identifier.end())
  {
    // Create new enumeration_node.
    node = _impl.enumeration_pool.create();
    node->parent_namescope = &_parent_node;
    node->identifier = token.identifier.value;
    translate_template_parameters(*node, token);
    // node->attributes =
    node->token = &token;
    token.ast_node = node;
    _parent_node.children.insert(node);
    bool success =
      _parent_node.enumerations.insert_or_assign(node->identifier, node).second;
    BOOST_ASSERT(success);
    (void)success;
  }
  else
  {
    handle_name_collision(token.identifier, *node_iterator);
    return false;
  }

  BOOST_ASSERT(node);
  return true;
}

token_to_ast_visitor::return_type token_to_ast_visitor::operator()(
  structure_token& token)
{
  token.parent = &_parent_token;

  ast::structure_node* structure_node = nullptr;
  auto& children_by_identifier =
    _parent_node.children.get<ast::by_identifier>();
  auto node_iterator = children_by_identifier.find(token.identifier.value);
  if (node_iterator == children_by_identifier.end())
  {
    // Create new structure_node.
    structure_node = _impl.structure_pool.create();
    structure_node->identifier = token.identifier.value;
    // structure_node->template_parameters =
    structure_node->parent_namescope = &_parent_node;
    // structure_node->depth =
    // structure_node->attributes =
    structure_node->comments = translate_comments(token.meta.comments);
    translate_template_parameters(*structure_node, token);
    // structure_node->attributes =
    structure_node->token = &token;
    // structure_node->base =
    token.ast_node = structure_node;
    _parent_node.children.insert(structure_node);
    bool success =
      _parent_node.structures
        .insert_or_assign(structure_node->identifier, structure_node)
        .second;
    BOOST_ASSERT(success);
    (void)success;
  }
  else
  {
    handle_name_collision(token.identifier, *node_iterator);
    return false;
  }

  BOOST_ASSERT(structure_node);
  if (!structure_node)
    return false;

  auto result = true;
  for (auto& field : token.fields)
  {
    // Check if another field of the same name is already defined.
    auto other_field_iter =
      std::find_if(structure_node->fields.begin(), structure_node->fields.end(),
                   [&](ast::field_node* other_field) -> bool {
                     return field.identifier.value == other_field->identifier;
                   });
    if (other_field_iter == structure_node->fields.end())
    {
      // Create field node.
      auto* field_node = _impl.field_pool.create();
      // field_node->this_type =
      field_node->identifier = field.identifier.value;
      field_node->parent_namescope = &_parent_node;
      // field_node->depth =
      // field_node->attributes =
      field_node->comments = translate_comments(field.meta.comments);
      field_node->token = &field;
      field_node->parent_structure = structure_node;
      field_node->is_const = field.is_const;
      // field_node->reference =
      // field_node->default_value =

      field.ast_node = field_node;
      structure_node->fields.push_back(field_node);
    }
    else
    {
      handle_name_collision(field.identifier, *other_field_iter);
      result = false;
    }
  }
  return result;
}

bool token_to_ast_visitor::translate_template_parameters(
  ast::type_node& node, const node_token& token)
{
  for (const auto& parameter_token : token.template_parameters)
  {
    ast::template_parameter_node paramter_node;
    paramter_node.token = &parameter_token;
    if (!parameter_token.is_typename)
    {
      // translate_type_path(parameter_token.type);
    }
    paramter_node.is_typename = parameter_token.is_typename;
    paramter_node.is_variadic = parameter_token.is_variadic;
    paramter_node.identifier = parameter_token.identifier.value;
    if (const auto* value_path =
          std::get_if<type_path_token>(&parameter_token.default_value);
        value_path != nullptr)
    {
      // translate_type_path(*value_path);
      /// ToDo
      // paramter_node.default_value =
    }
    else if (const auto* value_int64 =
               std::get_if<std::int64_t>(&parameter_token.default_value);
             value_int64 != nullptr)
    {
      paramter_node.default_value = *value_int64;
    }
    else if (const auto* value_uint64 =
               std::get_if<std::uint64_t>(&parameter_token.default_value);
             value_uint64 != nullptr)
    {
      paramter_node.default_value = *value_uint64;
    }
    else if (const auto* value_null =
               std::get_if<std::nullptr_t>(&parameter_token.default_value);
             value_null != nullptr)
    {
      paramter_node.default_value = nullptr;
    }
    else
    {
      BOOST_ASSERT(false);
      BOOST_THROW_EXCEPTION(
        core::logic_error() << core::context_info(
          "Cannot determine type of template_parameter_token::default_value."));
    }
    // paramter_node.default_value gets translated in type_reference_resolver
    // because it might reference another type which is not necessarily created,
    // yet.
    node.template_parameters.emplace_back(std::move(paramter_node));
  }
  return true;
}

bool token_to_ast_visitor::translate_type_path(const type_path_token& /*token*/)
{
  return false;
}

void token_to_ast_visitor::handle_name_collision(
  const string_token& new_identifier, const ast::type_node* existing_definition)
{
  if (auto* other_namescope = existing_definition->as_namescope())
  {
    _impl.error_stream << error_redefinition{new_identifier,
                                             other_namescope->token->identifier}
                       << std::endl;
  }
  else if (const auto* other_alias = existing_definition->as_alias())
  {
    _impl.error_stream << error_redefinition{new_identifier,
                                             other_alias->token->identifier}
                       << std::endl;
  }
  else if (const auto* other_enumeration =
             existing_definition->as_enumeration())
  {
    _impl.error_stream
      << error_redefinition{new_identifier,
                            other_enumeration->token->identifier}
      << std::endl;
  }
  else if (const auto* other_structure = existing_definition->as_structure())
  {
    _impl.error_stream << error_redefinition{new_identifier,
                                             other_structure->token->identifier}
                       << std::endl;
  }
  else if (const auto* other_field = existing_definition->as_field())
  {
    _impl.error_stream << error_redefinition{new_identifier,
                                             other_field->token->identifier}
                       << std::endl;
  }
  else
  {
    BOOST_ASSERT(false);
    BOOST_THROW_EXCEPTION(core::logic_error() << core::context_info(
                            "Cannot determine type of ast::type_node."));
  }
}

std::string token_to_ast_visitor::translate_comments(comments_token& comments)
{
  if (comments.empty())
    return {};
  else if (comments.size() == 1)
    return comments.front();
  else
  {
    std::stringstream result;
    bool first = true;
    for (const auto& line : comments)
    {
      if (first)
        first = false;
      else
        result << '\n';
      result << line;
    }
    return result.str();
  }
}
}
