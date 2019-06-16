#ifndef SHIFT_PARSER_JSON_HASH_HPP
#define SHIFT_PARSER_JSON_HASH_HPP

#include <shift/core/boost_disable_warnings.hpp>
#include <boost/functional/hash/hash.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/parser/json/json.hpp"

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
  if (shift::parser::json::get_if<std::nullptr_t>(&value) != nullptr)
    return 0;
  else if (auto* bool_value = shift::parser::json::get_if<bool>(&value);
           bool_value != nullptr)
  {
    return std::hash<bool>{}(*bool_value);
  }
  else if (auto* double_value = shift::parser::json::get_if<double>(&value);
           double_value != nullptr)
  {
    return std::hash<double>{}(*double_value);
  }
  else if (auto* int_value = shift::parser::json::get_if<std::int64_t>(&value);
           int_value != nullptr)
  {
    return std::hash<std::int64_t>{}(*int_value);
  }
  else if (auto* string_value =
             shift::parser::json::get_if<std::string>(&value);
           string_value != nullptr)
  {
    return std::hash<std::string>{}(*string_value);
  }
  else if (auto* object_value =
             shift::parser::json::get_if<shift::parser::json::object>(&value);
           object_value != nullptr)
  {
    return std::hash<shift::parser::json::object>{}(*object_value);
  }
  else if (auto* array_value =
             shift::parser::json::get_if<shift::parser::json::array>(&value);
           array_value != nullptr)
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
