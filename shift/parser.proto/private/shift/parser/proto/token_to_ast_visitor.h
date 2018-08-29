#ifndef SHIFT_PARSER_PROTO_TOKEN_TO_AST_VISITOR_H
#define SHIFT_PARSER_PROTO_TOKEN_TO_AST_VISITOR_H

#include "shift/parser/proto/ast.h"
#include "shift/parser/proto/tokens.h"

namespace shift::parser::proto
{
struct document_impl;

/// This type helps walk the tree of token variant types and
/// - assign each node_token its proper parent pointer
/// - performs identifier checks to avoid redefinition of symbols.
/// - translates all type definition tokens to public AST nodes.
class token_to_ast_visitor
{
public:
  using return_type = bool;

  /// Constructor.
  token_to_ast_visitor(namescope_token& parent_token,
                       ast::namescope_node& parent_node, document_impl& impl);

  ///
  return_type operator()(namescope_token& token);

  ///
  return_type operator()(alias_token& token);

  ///
  return_type operator()(enumeration_token& token);

  ///
  return_type operator()(structure_token& token);

private:
  ///
  bool translate_template_parameters(ast::type_node& node,
                                     const node_token& token);

  ///
  bool translate_type_path(const type_path_token& token);

  ///
  void handle_name_collision(const string_token& new_identifier,
                             const ast::type_node* existing_definition);

  ///
  static std::string translate_comments(comments_token& comments);

  namescope_token& _parent_token;
  ast::namescope_node& _parent_node;
  document_impl& _impl;
};
}

#endif
