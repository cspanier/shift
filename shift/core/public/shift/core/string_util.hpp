#ifndef SHIFT_CORE_STRINGUTIL_HPP
#define SHIFT_CORE_STRINGUTIL_HPP

#include <cctype>
#include <cstddef>
#include <type_traits>
#include <vector>
#include <string>
#include <string_view>

namespace shift::core
{
///
std::wstring convert(std::string_view input);

/// A simple split algorithm which takes any iterable type Range (which
/// supports std::begin and std::end functions) and splitting it into tokens
/// by identifying the splitters using a custom predicate. Found tokens are
/// returned in a vector.
template <typename Range, typename Predicate>
std::vector<Range> split(const Range& input, Predicate predicate,
                         bool include_empty = false)
{
  std::vector<Range> tokens;
  auto first = std::begin(input);
  for (auto current = std::begin(input); current != std::end(input); ++current)
  {
    if (predicate(*current))
    {
      if (include_empty || first != current)
        tokens.emplace_back(first, current);
      first = current;
      ++first;
    }
  }
  if (include_empty || first != std::end(input))
    tokens.emplace_back(first, std::end(input));
  return tokens;
}

/// An advanced split algorithm which takes any iterable type Range (which
/// supports cbegin and cend methods) and splitting it into tokens by
/// identifying the splitters using a custom predicate and passing each
/// found token to another custom functor action.
template <typename Input, typename Predicate, typename Action>
void split(Input begin, Input end, Predicate predicate, Action action,
           bool include_empty = false)
{
  auto first = begin;
  for (auto current = begin; current != end; ++current)
  {
    if (predicate(*current))
    {
      if (include_empty || first != current)
        action(first, current);
      first = current;
      ++first;
    }
  }
  if (include_empty || first != end)
    action(first, end);
}

/// Concatenates a vector of strings into one string.
std::string implode(const std::vector<std::string>& strings);

/// Concatenates a vector of strings into one string.
std::string implode(const std::vector<std::string>& strings,
                    std::string_view delimiter);

/// Escapes special characters like line feeds or tabulators.
std::string escape(std::string_view source);

/// Unescapes escape sequences like '\n' or '\t' in the source string.
std::string unescape(std::string_view source);

/// Converts any uppercase letters in the array of characters to lowercase.
std::string to_lower(std::string_view string);

/// Converts any uppercase letters in the array of characters to lowercase.
std::wstring to_lower(std::wstring_view string);

/// Converts any lowercase letters in the array of characters to uppercase.
std::string to_upper(std::string_view string);

/// Converts any lowercase letters in the array of characters to uppercase.
std::wstring to_upper(std::wstring_view string);

/// A ready to use replacement for std::isupper.
/// @remarks
///   Like all other functions from <cctype>, the behavior of std::isupper is
///   undefined if the argument's value is neither representable as unsigned
///   char nor equal to EOF.
template <typename T>
bool is_upper(T c)
{
  return std::isupper(static_cast<unsigned char>(c)) != 0;
}

/// A ready to use replacement for std::islower.
/// @remarks
///   Like all other functions from <cctype>, the behavior of std::isupper is
///   undefined if the argument's value is neither representable as unsigned
///   char nor equal to EOF.
template <typename T>
bool is_lower(T c)
{
  return std::islower(static_cast<unsigned char>(c)) != 0;
}

/// A ready to use replacement for std::toupper.
/// @remarks
///   Like all other functions from <cctype>, the behavior of std::isupper is
///   undefined if the argument's value is neither representable as unsigned
///   char nor equal to EOF.
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T to_upper(T c)
{
  return static_cast<T>(std::toupper(static_cast<unsigned char>(c)));
}

/// A ready to use replacement for std::tolower.
/// @remarks
///   Like all other functions from <cctype>, the behavior of std::isupper is
///   undefined if the argument's value is neither representable as unsigned
///   char nor equal to EOF.
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T to_lower(T c)
{
  return static_cast<T>(std::tolower(static_cast<unsigned char>(c)));
}
}

#endif
