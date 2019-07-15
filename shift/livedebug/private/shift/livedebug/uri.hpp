#ifndef SHIFT_LIVEDEBUG_URI_HPP
#define SHIFT_LIVEDEBUG_URI_HPP

#include <string>
#include <string_view>

namespace shift::livedebug
{
// Hold string_views to individual components of an Uniform Resource Identifier.
struct uri_t
{
  std::string_view identifier;
  std::string_view scheme;
  std::string_view authority;
  std::string_view path;
  std::string_view query;
  std::string_view fragment;
};

/// Splits a valid Uniform Resource Identifier (URI) into its components.
/// @return
///   True if the identifier could be split successfuly, false otherwise.
bool uri_split(std::string_view identifier, uri_t& result);
}

#endif
