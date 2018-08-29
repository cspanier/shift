#ifndef SHIFT_PARSER_XML_GRAMMAR_H
#define SHIFT_PARSER_XML_GRAMMAR_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/parser/xml/xml.h"

namespace shift::parser::xml
{
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::iso8859_1;

/// Xml files must not skip any whitespace, but put these into text
/// elements.
/// @remarks
///   Boost Spirit doesn't seem to provide an empty skipper for some reason.
struct xml_skipper : public qi::grammar<std::string::const_iterator>
{
  xml_skipper() : xml_skipper::base_type(empty_skipper, "XML")
  {
  }

  qi::rule<std::string::const_iterator> empty_skipper;
};

///
class grammar : public qi::grammar<std::string::const_iterator,
                                   node::children_t(), xml_skipper>
{
public:
  using iterator = std::string::const_iterator;
  using skipper = xml_skipper;

  /// Default constructor.
  grammar();

private:
  qi::symbols<char, char> _entities;
  qi::rule<iterator, std::string()> _tag_name;
  qi::rule<iterator, attribute_t()> _attribute;
  qi::rule<iterator, node::child_t()> _child;
  qi::rule<iterator, std::string()> _empty_string;
  qi::rule<iterator, attributes_t()> _empty_attributes;
  qi::rule<iterator, node::children_t()> _empty_children;
  qi::rule<iterator, node_type()> _element_type;
  qi::rule<iterator, node_type()> _text_type;
  qi::rule<iterator, node_type()> _comment_type;
  qi::rule<iterator, node_type()> _declaration_type;
  qi::rule<iterator, std::string()> _start_tag;
  qi::rule<iterator, void(std::string)> _end_tag;
  qi::rule<iterator, node()> _text_node;
  qi::rule<iterator, node()> _comment_node;
  qi::rule<iterator, node()> _declaration_node;
  qi::rule<iterator, node(), qi::locals<std::string>> _element_node;
  qi::rule<iterator, node::children_t(), skipper> _root;
};
}

#endif
