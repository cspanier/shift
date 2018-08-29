#ifndef SHIFT_PARSER_JSON_GRAMMAR_H
#define SHIFT_PARSER_JSON_GRAMMAR_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/parser/json/json.h"

namespace shift::parser::json
{
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace encoding = boost::spirit::iso8859_1;

///
class json_comment_skipper : public qi::grammar<std::string::const_iterator>
{
public:
  json_comment_skipper() : json_comment_skipper::base_type(_rule, "JSON")
  {
    using namespace phoenix;
    using namespace qi::labels;
    using encoding::char_;
    using encoding::space;
    using qi::eol;
    using qi::lit;

    _rule %= space | (lit("#") > *(char_ - eol));
  }

  qi::rule<std::string::const_iterator> _rule;
};

///
class grammar
: public qi::grammar<std::string::const_iterator, value(), json_comment_skipper>
{
public:
  using iterator = std::string::const_iterator;
  using skipper = json_comment_skipper;

  /// Default constructor.
  grammar();

private:
  qi::symbols<const char, std::nullptr_t> _null;
  qi::symbols<const char, const char> _unescape_map;
  qi::rule<iterator, std::string(), skipper> _string;
  qi::rule<iterator, json::value(), skipper> _value;
  qi::rule<iterator, json::array(), skipper> _array;
  qi::rule<iterator, std::pair<std::string, json::value>(), skipper> _key_value;
  qi::rule<iterator, json::object(), skipper> _object;
};
}

#endif
