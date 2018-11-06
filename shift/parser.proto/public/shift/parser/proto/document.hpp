#ifndef SHIFT_PARSER_PROTO_DOCUMENT_HPP
#define SHIFT_PARSER_PROTO_DOCUMENT_HPP

#include <memory>
#include <ostream>
#include <shift/core/types.hpp>
#include "shift/parser/proto/source_module.hpp"
#include "shift/parser/proto/ast.hpp"

namespace shift::parser::proto
{
struct document_impl;

///
class document
{
public:
  /// Constructor.
  document();

  /// Destructor.
  ~document();

  document(const document&) = delete;

  /// Move constructor.
  document(document&& other);

  document& operator=(const document&) = delete;

  /// Move assignment operator.
  document& operator=(document&& other);

  /// Returns a reference to the AST root node.
  const ast::namescope_node& root() const;

  /// Parse a list of source modules and attempt to translate them into a
  /// single abstract syntax tree.
  ast::namescope_node* parse(
    std::vector<std::shared_ptr<source_module>> sources);

  ///// Adds additional source code to the document. The source is translated
  ///// into tokens and correctness of syntax is checked at this stage.
  ///// @pre
  /////   link has not been called, yet.
  // bool parse(const std::string& source, const std::string& source_name);

  ///// Semantically check all previously compiled tokens and build a singe
  ///// combined abstract syntax tree. This method may only be called once.
  ///// @pre
  /////   link has not been called, yet.
  ///// @post
  /////   If no error occurred, the document will contain a valid AST.
  // ast::namescope_node* link();

  /// Returns the list of errors detected during link.
  std::string errors() const;

  ///
  friend std::ostream& operator<<(std::ostream& stream, document& document);

private:
  std::unique_ptr<document_impl> _impl;
};
}

#endif
