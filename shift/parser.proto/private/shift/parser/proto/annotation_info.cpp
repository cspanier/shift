#include "shift/parser/proto/annotation_info.h"
#include <algorithm>

namespace shift::parser::proto
{
static std::size_t longest_line_length(std::string_view::const_iterator begin,
                                       std::string_view::const_iterator end)
{
  std::size_t max_length = 1;
  std::size_t current_length = 0;
  for (auto i = begin; i != end; ++i)
  {
    if (*i != '\n')
      ++current_length;
    else
    {
      if (current_length > max_length)
        max_length = current_length;
      current_length = 0;
    }
  }
  return (current_length > max_length) ? current_length : max_length;
}

std::ostream& operator<<(std::ostream& stream, const annotation_source& where)
{
  auto source = where.info.source.lock();
  if (source)
  {
    const auto& code = source->code;
    auto line =
      std::count(code.begin(), code.begin() + where.info.token_begin, '\n');
    stream << std::string{source->name} << "(" << (line + 1) << ")";
  }
  else
    stream << "<unknown source>";
  return stream;
}

std::ostream& operator<<(std::ostream& stream, const annotation_code& where)
{
  auto source = where.info.source.lock();
  if (source)
  {
    auto token_begin = source->code.begin() + where.info.token_begin;
    auto token_end = source->code.begin() + where.info.token_end;

    // Find begin of line.
    auto line_begin = token_begin;
    auto i = token_begin;
    // Handle special case when token_begin points to the end of file or to an
    // end of line character.
    if (i == source->code.end() || (*i == '\n'))
      --i;
    for (; *i != '\n'; --i)
    {
      line_begin = i;
      if (i == source->code.begin())
        break;
    }
    // Find end of line.
    auto line_end = token_end;
    while ((line_end != source->code.end()) && (*line_end != '\n'))
    {
      ++line_end;
    }

    auto column = std::distance(line_begin, token_begin);
    auto length = longest_line_length(token_begin, token_end);
    stream << std::string(line_begin, line_end) << "\n";
    stream << std::string(column, ' ') << std::string(length, '^');
  }
  return stream;
}
}
