#ifndef SHIFT_PROTO_ERRORHANDLER_HPP
#define SHIFT_PROTO_ERRORHANDLER_HPP

#include <algorithm>
#include <string>
#include <map>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/proto/types.hpp"

namespace shift::proto
{
/// An error handler for the grammar parser.
template <typename Iterator>
struct error_handler
{
  template <class Info>
  void operator()(const std::map<std::string, std::string>& diagnostics,
                  Iterator begin, Iterator end, Iterator where,
                  Info const& info) const
  {
    // Find begin of line.
    Iterator line_begin = begin;
    for (Iterator i = where; i != begin && *i != '\n' && *i != '\r'; --i)
      line_begin = i;

    // Find end of line.
    Iterator line_end = where;
    for (Iterator i = where; i != end && *i != '\n' && *i != '\r'; ++i)
      line_end = i;
    if (line_end != end)
      ++line_end;

    std::size_t line = std::count(begin, where, '\n') + 1;
    std::size_t column = where - line_begin + 1;

    const auto& tag(info.tag);
    auto diagnostic = diagnostics.find(std::string(tag));

    BOOST_THROW_EXCEPTION(
      parse_error() << parse_error_line(line) << parse_error_column(column)
                    << parse_error_source(std::string(line_begin, line_end))
                    << parse_error_message((diagnostic != diagnostics.end())
                                             ? diagnostic->second
                                             : "Expected " + tag));
  }
};
}

#endif
