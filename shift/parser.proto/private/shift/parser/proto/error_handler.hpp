#ifndef SHIFT_PARSER_PROTO_ERRORHANDLER_HPP
#define SHIFT_PARSER_PROTO_ERRORHANDLER_HPP

#include <algorithm>
#include <string>
#include <map>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/spirit/include/qi.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include <shift/core/exception.hpp>
#include "shift/parser/proto/proto.hpp"
#include "shift/parser/proto/error.hpp"

namespace shift::parser::proto
{
using parse_error_message =
  boost::error_info<struct parse_error_message_tag, error_parse>;

/// An exception type thrown if the parser reports an error.
struct parse_error : virtual core::runtime_error
{
};

/// An error handler for the grammar parser.
template <typename Iterator>
struct error_handler
{
  template <class Info>
  void operator()(
    const std::vector<std::pair<std::string, std::string>>& diagnostics,
    Iterator /*begin*/, Iterator /*end*/, Iterator where,
    Info const& info) const
  {
    using std::distance;

    BOOST_ASSERT(source);

    auto token_location = distance(source->code.cbegin(), where);

    auto diagnostic = std::find_if(
      diagnostics.begin(), diagnostics.end(),
      [&](const auto& pair) { return pair.first == info.tag.c_str(); });

    // Throw exception because we cannot continue parsing the document.
    BOOST_THROW_EXCEPTION(
      parse_error() << parse_error_message(
        error_parse{(diagnostic != diagnostics.end()) ? diagnostic->second
                                                      : "Expected " + info.tag,
                    static_cast<std::size_t>(
                      std::distance(diagnostics.begin(), diagnostic)),
                    annotation_info(source, token_location, token_location)}));
  }

  std::shared_ptr<source_module> source;
};
}

#endif
