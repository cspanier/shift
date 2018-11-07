#ifndef SHIFT_PARSER_PROTO_GRAMMAR_HPP
#define SHIFT_PARSER_PROTO_GRAMMAR_HPP

// #define BOOST_SPIRIT_DEBUG 1

#include <vector>
#include <string>
#include <map>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/parser/proto/proto.hpp"
#include "shift/parser/proto/tokens.hpp"
#include "shift/parser/proto/annotation_handler.hpp"
#include "shift/parser/proto/error_handler.hpp"

namespace shift::parser::proto
{
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::iso8859_1;

/// The Proto grammar definition, which contains rules used for lexical
/// analysis and parsing of Proto definition files.
class grammar : public qi::grammar<std::string_view::const_iterator,
                                   definition_tokens(), encoding::space_type>
{
public:
  using iterator = std::string_view::const_iterator;
  using skipper = encoding::space_type;

  /// Default constructor.
  grammar(const std::shared_ptr<source_module>& source);

private:
  qi::rule<iterator, void(), skipper> _optional_space;
  qi::rule<iterator, void(), skipper> _whitespace;
  qi::rule<iterator, void(), skipper> _period;
  qi::rule<iterator, void(), skipper> _colon;
  qi::rule<iterator, void(), skipper> _semicolon;
  qi::rule<iterator, void(), skipper> _comma;
  qi::rule<iterator, void(), skipper> _equal_sign;
  qi::rule<iterator, void(), skipper> _bracket_open;
  qi::rule<iterator, void(), skipper> _bracket_close;
  qi::rule<iterator, void(), skipper> _curly_bracket_open;
  qi::rule<iterator, void(), skipper> _curly_bracket_close;
  qi::rule<iterator, void(), skipper> _angle_bracket_open;
  qi::rule<iterator, void(), skipper> _angle_bracket_close;
  qi::rule<iterator, void(), skipper> _single_quote;
  qi::rule<iterator, void(), skipper> _quote;

  qi::rule<iterator, std::int64_t(), skipper> _sint;
  qi::rule<iterator, std::uint64_t(), skipper> _uint;
  qi::rule<iterator, std::string(), skipper> _string_impl;
  qi::rule<iterator, string_token(), skipper> _string;
  qi::symbols<const char, const char> _escape_sequences;

  qi::rule<iterator, std::string(), skipper> _identifier_impl;
  qi::rule<iterator, string_token(), skipper> _identifier;
  qi::rule<iterator, std::string(), skipper> _template_identifier_impl;
  qi::rule<iterator, string_token(), skipper> _template_identifier;
  qi::rule<iterator, string_token(), skipper> _any_identifier;
  qi::rule<iterator, template_argument_token(), skipper> _template_argument;
  qi::rule<iterator, type_path_element_token(), skipper> _type_path_element;
  qi::rule<iterator, type_path_token(), skipper> _type_path;

  qi::rule<iterator, comment_token(), skipper> _comment;
  qi::rule<iterator, type_attribute_token(), skipper> _attribute;
  qi::rule<iterator, type_attribute_tokens(), skipper> _attributes;
  qi::rule<iterator, bool(), skipper> _const_qualifier;
  qi::rule<iterator, enumerant_reference_token(), skipper> _enumerant_reference;
  qi::rule<iterator, template_parameter_token(), skipper> _template_parameter;
  qi::rule<iterator, template_parameter_tokens(), skipper>
    _template_parameters_impl;
  qi::rule<iterator, template_parameter_tokens(), skipper> _template_parameters;
  qi::rule<iterator, meta_token(), skipper> _meta;
  qi::rule<iterator, alias_token(), skipper> _alias;
  qi::rule<iterator, enumerant_value, skipper> _enumerant_value;
  qi::rule<iterator, enumerant_token, skipper> _enumerant;
  qi::rule<iterator, enumeration_token(), skipper> _enumeration;
  qi::rule<iterator, field_value(), skipper> _field_value;
  qi::rule<iterator, field_token(), skipper> _field;
  qi::rule<iterator, structure_token(), skipper> _structure;
  qi::rule<iterator, definition_tokens(), skipper> _namescope_content;
  qi::rule<iterator, namescope_token(), skipper> _namescope;

  phoenix::function<annotation_handler<iterator>> _annotation;
  phoenix::function<error_handler<iterator>> _error;
  std::vector<std::pair<std::string, std::string>> _diagnostics;
};
}

#endif
