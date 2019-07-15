#include "shift/livedebug/uri.hpp"
#include <boost/assert.hpp>
#include <regex>

namespace shift::livedebug
{
bool uri_split(std::string_view identifier, uri_t& result)
{
  static std::regex uri_regex(
    R"(^(([^:\/?#]+):)?(//([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)",
    std::regex::extended);
  std::match_results<std::string_view::const_iterator> uri_match_result;
  if (!std::regex_match(identifier.begin(), identifier.end(), uri_match_result,
                        uri_regex))
  {
    return false;
  }
  BOOST_ASSERT(uri_match_result.size() == 10);

  auto to_string_view = [](const auto input) -> std::string_view {
    return {input.first, static_cast<std::size_t>(input.length())};
  };

  result.identifier = identifier;
  result.scheme = to_string_view(uri_match_result[2]);
  result.authority = to_string_view(uri_match_result[4]);
  result.path = to_string_view(uri_match_result[5]);
  result.query = to_string_view(uri_match_result[7]);
  result.fragment = to_string_view(uri_match_result[9]);
  return true;
}
}
