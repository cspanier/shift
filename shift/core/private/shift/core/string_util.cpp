#include "shift/core/string_util.h"
#include <cstring>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <locale>
#include <utility>
#include <cwchar>

namespace shift::core
{
std::wstring convert(std::string_view input)
{
  std::wstring output(input.begin(), input.end());
  // std::use_facet<std::ctype<wchar_t>>(std::cout.getloc()).widen(
  //  input.begin(), input.end(), output);
  return output;
}

std::string implode(const std::vector<std::string>& strings)
{
  std::stringstream stream;
  for (const auto& string : strings)
    stream << string;
  return stream.str();
}

std::string implode(const std::vector<std::string>& strings,
                    std::string_view delimiter)
{
  std::stringstream stream;
  bool first = true;
  for (const auto& string : strings)
  {
    if (first)
      first = false;
    else
      stream << delimiter;
    stream << string;
  }
  return stream.str();
}

std::string escape(std::string_view source)
{
  std::stringstream result;
  for (auto c : source)
  {
    switch (c)
    {
    case '\r':
      // Ignore carriage return characters.
      break;
    case '\n':
      result << "\\n";
      break;
    case '\t':
      result << "\\t";
      break;
    case '\\':
      result << "\\\\";
      break;
    default:
      result << c;
    }
  }
  return result.str();
}

std::string unescape(std::string_view source)
{
  std::stringstream result;
  for (auto c = source.cbegin(); c != source.cend(); ++c)
  {
    if (*c == '\\')
    {
      if (++c != source.cend())
      {
        switch (*c)
        {
        case 'r':
          // Ignore carriage return characters.
          break;
        case 'n':
          result << '\n';
          break;
        case 't':
          result << '\t';
          break;
        case '\\':
          result << '\\';
          break;
        }
      }
      else
      {
        result << '\\';
        break;
      }
    }
    else
      result << *c;
  }
  return result.str();
}

std::string to_lower(std::string_view string)
{
  if (string.length() == 0)
    return "";
  std::string result(string.data(), string.length());
  std::use_facet<std::ctype<char>>(std::locale(""))
    .tolower(&result[0], &result[string.length()]);
  return result;
}

std::wstring to_lower(std::wstring_view string)
{
  if (string.length() == 0)
    return L"";
  std::wstring result(string.data(), string.length());
  std::use_facet<std::ctype<std::wstring::value_type>>(std::locale(""))
    .tolower(&result[0], &result[string.length()]);
  return result;
}

std::string to_upper(std::string_view string)
{
  if (string.length() == 0)
    return "";
  std::string result(string.data(), string.length());
  std::use_facet<std::ctype<char>>(std::locale(""))
    .toupper(&result[0], &result[string.length()]);
  return result;
}

std::wstring to_upper(std::wstring_view string)
{
  if (string.length() == 0)
    return L"";
  std::wstring result(string.data(), string.length());
  std::use_facet<std::ctype<std::wstring::value_type>>(std::locale(""))
    .toupper(&result[0], &result[string.length()]);
  return result;
}
}
