#include "shift/parser/proto/document.hpp"
#include "shift/parser/proto/document_impl.hpp"
#include "shift/parser/proto/grammar.hpp"
#include "shift/parser/proto/token_to_ast_visitor.hpp"
#include "shift/parser/proto/type_reference_resolver.hpp"
#include "shift/parser/proto/error.hpp"
#include "shift/parser/proto/built_in_types.hpp"

namespace shift::parser::proto
{
document::document() : _impl(std::make_unique<document_impl>())
{
}

document::~document() = default;

document::document(document&& other) : _impl(std::move(other._impl))
{
}

document& document::operator=(document&& other)
{
  _impl = std::move(other._impl);
  return *this;
}

const ast::namescope_node& document::root() const
{
  return _impl->root_node;
}

ast::namescope_node* document::parse(
  std::vector<std::shared_ptr<source_module>> sources)
{
  bool valid = true;
  grammar::skipper skipper{};

  // First parse built-in types module.
  try
  {
    grammar grammar{built_in_types::source};
    qi::phrase_parse(built_in_types::source->code.cbegin(),
                     built_in_types::source->code.cend(), grammar, skipper,
                     qi::skip_flag::postskip, _impl->root_token.children);
  }
  catch (boost::exception& e)
  {
    _impl->error_stream << *boost::get_error_info<parse_error_message>(e)
                        << std::endl;
    valid = false;
  }

  // Parse all passed source modules.
  for (auto& source : sources)
  {
    try
    {
      grammar grammar{source};
      qi::phrase_parse(source->code.cbegin(), source->code.cend(), grammar,
                       skipper, qi::skip_flag::postskip,
                       _impl->root_token.children);
    }
    catch (boost::exception& e)
    {
      _impl->error_stream << *boost::get_error_info<parse_error_message>(e)
                          << std::endl;
      valid = false;
    }
  }

  // First translation pass.
  for (auto& child : _impl->root_token.children)
  {
    valid &= std::visit(
      token_to_ast_visitor{_impl->root_token, _impl->root_node, *_impl}, child);
  }

  // Second translation pass.
  if (valid)
  {
    for (auto& child : _impl->root_token.children)
      valid &= std::visit(type_reference_resolver{*_impl}, child);
  }

  return valid ? &_impl->root_node : nullptr;
}

std::string document::errors() const
{
  return _impl->error_stream.str();
}

std::ostream& operator<<(std::ostream& stream, document& /*document*/)
{
  /// ToDo: Print document.
  return stream;
}
}
