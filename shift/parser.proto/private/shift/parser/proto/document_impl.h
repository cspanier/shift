#ifndef SHIFT_PARSER_PROTO_DOCUMENTIMPL_H
#define SHIFT_PARSER_PROTO_DOCUMENTIMPL_H

#include <unordered_map>
#include <shift/core/object_pool.h>
#include "shift/parser/proto/tokens.h"
#include "shift/parser/proto/document.h"

namespace shift::parser::proto
{
struct document_impl
{
  namescope_token root_token;
  ast::namescope_node root_node;
  core::object_pool<ast::namescope_node> namescope_pool;
  core::object_pool<ast::alias_node> alias_pool;
  core::object_pool<ast::enumeration_node> enumeration_pool;
  core::object_pool<ast::field_node> field_pool;
  core::object_pool<ast::structure_node> structure_pool;
  std::stringstream error_stream;
};
}

#endif
