#ifndef SHIFT_PROTO_TYPES_H
#define SHIFT_PROTO_TYPES_H

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <memory>
#include <variant>
#include <vector>
#include <string>
#include <unordered_map>
#include <utility>
#include <shift/core/boost_disable_warnings.h>
#include <boost/lexical_cast.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/exception.h>

namespace shift::proto
{
using parse_error_line =
  boost::error_info<struct parse_error_line_tag, std::size_t>;
using parse_error_column =
  boost::error_info<struct parse_error_column_tag, std::size_t>;
using parse_error_source =
  boost::error_info<struct parse_error_source_tag, std::string>;
using parse_error_message =
  boost::error_info<struct parse_error_message_tag, std::string>;

/// An exception type thrown if the parser reports an error.
struct parse_error : virtual core::runtime_error
{
};

/// Enumeration of supported built-in types.
enum class built_in_type : unsigned char
{
  undefined,
  boolean,
  char8,
  char16,
  char32,
  int8,
  uint8,
  int16,
  uint16,
  int32,
  uint32,
  int64,
  uint64,
  float32,
  float64,
  string,
  datetime,
  raw_ptr,
  unique_ptr,
  shared_ptr,
  weak_ptr,
  group_ptr,
  tuple,
  array,
  list,
  vector,
  set,
  matrix,
  map,
  variant,
  bitfield
};

struct alias;
struct type_reference;
struct enumerator;
struct enumeration;
struct field;
struct message;
struct interface;
struct service;
struct namescope;

using type =
  std::variant<namescope, alias, enumeration, message, interface, service>;

using attribute_value = std::variant<std::string, std::uint64_t>;
using attribute = std::pair<std::string, attribute_value>;
using attribute_pair = std::pair<std::string, attribute_value>;
using attribute_map = std::unordered_map<std::string, attribute_value>;
using namescope_path = std::vector<std::string>;
using type_path = std::vector<std::string>;

using type_variant =
  std::variant<built_in_type, const alias*, const enumeration*, const message*>;

using template_argument = std::variant<type_reference, int>;
using template_arguments = std::vector<template_argument>;
}

#endif
