#ifndef SHIFT_PARSER_JSON_HASH_H
#define SHIFT_PARSER_JSON_HASH_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/functional/hash/hash.hpp>
#include <shift/core/boost_restore_warnings.h>
#include "shift/parser/json/json.h"

namespace std
{
template <>
struct hash<shift::parser::json::object>
{
  inline std::size_t operator()(
    const shift::parser::json::object& object) const;
};

template <>
struct hash<shift::parser::json::array>
{
  inline std::size_t operator()(const shift::parser::json::array& array) const;
};

template <>
struct hash<shift::parser::json::value>
{
  inline std::size_t operator()(const shift::parser::json::value& value) const;
};

inline std::size_t hash<shift::parser::json::object>::operator()(
  const shift::parser::json::object& object) const
{
  auto result = std::hash<std::size_t>{}(object.size());
  for (const auto& value : object)
  {
    boost::hash_combine(result, std::hash<std::string>{}(value.first));
    boost::hash_combine(result,
                        std::hash<shift::parser::json::value>{}(value.second));
  }
  return result;
}

inline std::size_t hash<shift::parser::json::array>::operator()(
  const shift::parser::json::array& array) const
{
  auto result = std::hash<std::size_t>{}(array.size());
  for (const auto& value : array)
    boost::hash_combine(result, std::hash<shift::parser::json::value>{}(value));
  return result;
}

inline std::size_t hash<shift::parser::json::value>::operator()(
  const shift::parser::json::value& value) const
{
  if (auto* null_value = shift::parser::json::get_if<std::nullptr_t>(&value))
    return 0;
  else if (auto* bool_value = shift::parser::json::get_if<bool>(&value))
    return std::hash<bool>{}(*bool_value);
  else if (auto* double_value = shift::parser::json::get_if<double>(&value))
    return std::hash<double>{}(*double_value);
  else if (auto* string_value =
             shift::parser::json::get_if<std::string>(&value))
  {
    return std::hash<std::string>{}(*string_value);
  }
  else if (auto* object_value =
             shift::parser::json::get_if<shift::parser::json::object>(&value))
  {
    return std::hash<shift::parser::json::object>{}(*object_value);
  }
  else if (auto* array_value =
             shift::parser::json::get_if<shift::parser::json::array>(&value))
  {
    return std::hash<shift::parser::json::array>{}(*array_value);
  }
  else
  {
    BOOST_ASSERT(false);
    return 0;
  }
}
}

#endif
