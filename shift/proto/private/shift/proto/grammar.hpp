#ifndef SHIFT_PROTO_GRAMMAR_HPP
#define SHIFT_PROTO_GRAMMAR_HPP

#include <vector>
#include <string>
#include <map>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/proto/proto.hpp"
#include "shift/proto/errorhandler.hpp"

namespace shift::proto
{
namespace phoenix = boost::phoenix;
namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;
namespace iso8859_1 = boost::spirit::iso8859_1;

/// The Proto skipper grammar definition, which contains a rule used for
/// splitting a Proto document into tokens.
class skipper : public qi::grammar<std::string_view::const_iterator>
{
public:
  using iterator = std::string_view::const_iterator;

  /// Default constructor.
  skipper();

private:
  qi::rule<iterator> _skipper;
};

/// The Proto grammar definition, which contains rules used for lexical
/// analysis and parsing of Proto definition files.
class grammar : public qi::grammar<std::string_view::const_iterator,
                                   std::vector<type>(), skipper>
{
public:
  using iterator = std::string_view::const_iterator;

  /// Default constructor.
  grammar();

private:
  qi::rule<iterator, std::int64_t(), skipper> _sint;
  qi::rule<iterator, std::uint64_t(), skipper> _uint;
  qi::rule<iterator, std::string(), skipper> _namescope_name;
  qi::rule<iterator, std::string(), skipper> _typename;
  qi::rule<iterator, std::string(), skipper> _interface_name;
  qi::rule<iterator, type_path(), skipper> _type_path;
  qi::rule<iterator, type_path(), skipper> _interface_path;
  qi::rule<iterator, std::string(), skipper> _identifier;
  qi::rule<iterator, std::string(), skipper> _string;

  qi::symbols<char, built_in_type> _bool_type;
  qi::symbols<char, built_in_type> _char_type;
  qi::symbols<char, built_in_type> _sint_type;
  qi::symbols<char, built_in_type> _uint_type;
  qi::symbols<char, built_in_type> _float_type;
  qi::symbols<char, built_in_type> _string_type;
  qi::symbols<char, built_in_type> _template_type;
  qi::rule<iterator, built_in_type(), skipper> _int_type;
  qi::rule<iterator, built_in_type(), skipper> _built_in_type;

  qi::rule<iterator, proto::attribute(), skipper> _attribute;
  qi::rule<iterator, attribute_map(), skipper> _attributes;
  qi::rule<iterator, type_reference(), skipper> _type_reference;
  qi::rule<iterator, alias(), skipper> _alias;
  qi::rule<iterator, enumerator(int), skipper> _enumerator;
  qi::rule<iterator, enumeration(),
           // Local variable _a storing the next enumerator's automatic value.
           qi::locals<int>, skipper>
    _enumeration;
  qi::rule<iterator, field(), skipper> _field;
  qi::rule<iterator, message(), skipper> _message;
  qi::rule<iterator, interface(), skipper> _interface;
  qi::symbols<char, bool> _service_type;
  qi::rule<iterator, service(), skipper> _service;
  qi::rule<iterator, std::vector<type>(), skipper> _global_scope;
  qi::rule<iterator, namescope(), skipper> _namescope;
  phoenix::function<error_handler<iterator>> _error_handler;
};
}

#endif
