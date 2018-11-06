#ifndef SHIFT_PARSER_PROTO_TYPE_REFERENCE_RESOLVER_HPP
#define SHIFT_PARSER_PROTO_TYPE_REFERENCE_RESOLVER_HPP

#include "shift/parser/proto/ast.hpp"
#include "shift/parser/proto/tokens.hpp"

namespace shift::parser::proto
{
struct document_impl;

/// This visitor type is used in the second pass of translating tokens to public
/// AST nodes. It performs the following actions:
/// - resolve alias type references
/// - resolve structure base type references
/// - resolve structure field type references
class type_reference_resolver
{
public:
  using return_type = bool;

  /// Constructor.
  type_reference_resolver(document_impl& impl);

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
  ast::type_variant find_type(ast::namescope_node* namescope,
                              const type_path_token& path);

  ///
  ast::type_variant find_type_node(ast::namescope_node* namescope,
                                   const type_path_token& path,
                                   std::size_t current_depth = 0);

  document_impl& _impl;
};
}

#endif
