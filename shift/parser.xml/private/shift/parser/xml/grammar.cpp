//#define BOOST_NO_CXX11_RVALUE_REFERENCES
#include "shift/parser/xml/grammar.h"
#include <shift/core/boost_disable_warnings.h>
#include <boost/spirit/home/support/container.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/phoenix/fusion.hpp>
#include <shift/core/boost_restore_warnings.h>

namespace shift::parser::xml
{
namespace phoenix = boost::phoenix;

///
grammar::grammar() : grammar::base_type(_root, "xml document")
{
  using qi::eps;
  using qi::lit;
  using qi::lexeme;
  using qi::skip;
  using qi::no_skip;
  using encoding::char_;
  using encoding::string;
  using encoding::space;
  using namespace qi::labels;

  using phoenix::construct;
  using phoenix::val;

  _entities.add("&lt;", '<');
  _entities.add("&gt;", '>');
  _entities.add("&amp;", '&');
  _entities.add("&apos;", '\'');
  _entities.add("&quot;", '"');

  _tag_name %= lexeme[char_("a-zA-Z") >> *char_("a-zA-Z0-9_:")];
  _attribute %=
    skip(space)[(lexeme[char_("a-zA-Z") >> *char_("a-zA-Z0-9_:")] >> '=') >
                ('"' > lexeme[*(_entities | (char_ - '"'))] > '"' |
                 '\'' > lexeme[*(_entities | (char_ - '\''))] > '\'')];

  _child %= _comment_node | _element_node | _text_node;
  /// ToDo: For some reason the following eps parsers need to be wrapped
  /// into separate rules, or otherwise wired compile errors are generated.
  /// Find out why.
  _empty_string %= eps[_val = ""];
  _empty_attributes %= eps[_val = attributes_t{}];
  _empty_children %= eps[_val = node::children_t{}];

  _element_type %= eps[_val = node_type::element];
  _text_type %= eps[_val = node_type::text];
  _comment_type %= eps[_val = node_type::comment];
  _declaration_type %= eps[_val = node_type::declaration];

  _start_tag %= ('<' >> !lit('/')) > _tag_name;
  _end_tag %= "</" > string(_r1) > '>';

  _text_node %= _empty_string >> _text_type >> _empty_attributes >>
                +(_entities | (char_ - '<')) >> _empty_children;
  _comment_node %= "<!--" > _empty_string > _comment_type > _empty_attributes >
                   lexeme[*(_entities | (char_ - "-->"))] > "-->" >
                   _empty_children;

  _declaration_node %= "<?xml" > _empty_string > _declaration_type >
                       skip(space)[*_attribute] > _empty_string >
                       _empty_children > "?>";
  _element_node %= _start_tag[_a = _1] > _element_type >
                   skip(space)[*_attribute] > _empty_string >
                   (('>' > (*_child) > _end_tag(_a)) | "/>");
  _root %=
    *((_declaration_node | _comment_node | _element_node) > skip(space)[""]);

  _attribute.name("xml attribute");
  _child.name("xml child");
  _empty_string.name("no string");
  _empty_attributes.name("no attributes");
  _empty_children.name("no children");
  _element_type.name("element type");
  _text_type.name("text type");
  _comment_type.name("comment type");
  _declaration_type.name("declaration type");
  _start_tag.name("xml start tag");
  _end_tag.name("xml end tag");
  _text_node.name("text");
  _comment_node.name("xml comment");
  _declaration_node.name("xml declaration");
  _element_node.name("xml element");
  _root.name("xml root elements");

  /// ToDo: Replace std::wcerr with core::error, or add a custom error
  /// handler as in protogen project.
  qi::on_error<qi::fail>(_root,
                         std::cerr << val("Error! Expecting ") << _4
                                   <<  // what failed?
                           val(" here: \"") << construct<std::string>(_3, _2)
                                   <<  // iterators to error-pos, end
                           val("\"") << std::endl);
}
}

// clang-format off
#define ADAPT_MEMBER(TYPE, MEMBER) (decltype(TYPE::MEMBER), MEMBER)

BOOST_FUSION_ADAPT_STRUCT(shift::parser::xml::node,
  ADAPT_MEMBER(shift::parser::xml::node, name)
  ADAPT_MEMBER(shift::parser::xml::node, type)
  ADAPT_MEMBER(shift::parser::xml::node, attributes)
  ADAPT_MEMBER(shift::parser::xml::node, text)
  ADAPT_MEMBER(shift::parser::xml::node, children))

BOOST_FUSION_ADAPT_STRUCT(shift::parser::xml::attribute_t,
  ADAPT_MEMBER(shift::parser::xml::attribute_t, first)
  ADAPT_MEMBER(shift::parser::xml::attribute_t, second))

#undef ADAPT_MEMBER
// clang-format on

namespace boost::spirit::traits
{
using namespace shift::parser::xml;

template <>
struct push_back_container<attributes_t, attribute_t>
{
  static bool call(attributes_t& container, attribute_t const& value)
  {
    return container.insert(value).second;
  }
};
}
