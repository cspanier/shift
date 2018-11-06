#include "shift/parser/json/grammar.hpp"
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_fusion.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <shift/core/boost_restore_warnings.hpp>

namespace shift::parser::json
{
grammar::grammar() : grammar::base_type(_value, "json document root")
{
  using encoding::char_;
  using qi::bool_;
  using qi::double_;
  using qi::no_skip;
  using namespace qi::labels;

  using phoenix::construct;
  using phoenix::val;

  _null.add("null", nullptr);
  _unescape_map.add(R"(\")", '\"');
  _unescape_map.add(R"(\\)", '\\');
  _unescape_map.add(R"(\/)", '/');
  _unescape_map.add(R"(\b)", '\b');
  _unescape_map.add(R"(\f)", '\f');
  _unescape_map.add(R"(\n)", '\n');
  _unescape_map.add(R"(\r)", '\r');
  _unescape_map.add(R"(\t)", '\t');
  _string %=
    '"' > no_skip[*(_unescape_map | ("\\u" >> qi::hex) | (qi::char_ - '"'))] >
    '"';
  _value %= _null | bool_ | double_ | _string | _object | _array;
  _array %= '[' > -(_value % ',') > ']';
  _key_value %= _string > ':' > _value;
  _object %= '{' > -(_key_value % ',') > '}';

  _null.name("null");
  _unescape_map.name("escape sequence");
  _string.name("string");
  _value.name("value");
  _array.name("array");
  _key_value.name("key-value pair");
  _object.name("object");

  /// ToDo: Replace std::cerr with log::error, or add a custom error
  /// handler as in protogen project.
  qi::on_error<qi::fail>(_value,
                         std::cerr << val("Error! Expecting ") << _4
                                   <<  // what failed?
                           val(" here: \"") << construct<std::string>(_3, _2)
                                   <<  // iterators to error-pos, end
                           val("\"") << std::endl);
}

std::istream& operator>>(std::istream& stream, json::value& value)
{
  value = nullptr;

  std::string content;              // We will read the contents here.
  stream.unsetf(std::ios::skipws);  // No white space skipping!
  std::copy(std::istream_iterator<char>(stream), std::istream_iterator<char>(),
            std::back_inserter(content));

  json_comment_skipper skipper;
  if (!qi::phrase_parse(content.cbegin(), content.cend(), grammar{}, skipper,
                        qi::skip_flag::postskip, value))
  {
    /// ToDo: Improve error handling.
    BOOST_THROW_EXCEPTION(parse_error());
  }
  return stream;
}
}
