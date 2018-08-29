#ifndef SHIFT_PARSER_JSON_JSON_H
#define SHIFT_PARSER_JSON_JSON_H

#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <shift/core/exception.h>
#include <shift/core/stream_util.h>

namespace shift::parser::json
{
/// Exception type used for parsing errors.
struct parse_error : virtual core::runtime_error
{
};

/// A simple container which is able to hold all kind of JSON values.
struct value
{
public:
  using array_t = std::vector<value>;
  using object_t = std::map<std::string, value>;

public:
  /// Default constructor.
  value() = default;

  /// Default destructor.
  ~value() = default;

  /// Copy constructor.
  value(const value& other) = default;

  /// Move constructor.
  value(value&& other) = default;

  /// Constructor initializing value to nullptr.
  value(std::nullptr_t) : _value(nullptr)
  {
  }

  /// Constructor initializing value to bool.
  value(bool new_value) : _value(new_value)
  {
  }

  /// Constructor initializing value to double.
  value(double new_value) : _value(new_value)
  {
  }

  /// Constructor initializing value from a string.
  value(const std::string& new_value) : _value(new_value)
  {
  }

  /// Constructor initializing value from a string.
  value(std::string&& new_value) : _value(std::move(new_value))
  {
  }

  /// Constructor initializing value from an array.
  value(const array_t& new_value) : _value(new_value)
  {
  }

  /// Constructor initializing value from an array.
  value(array_t&& new_value) : _value(std::move(new_value))
  {
  }

  /// Constructor initializing value from an object.
  value(const object_t& new_value) : _value(new_value)
  {
  }

  /// Constructor initializing value from an object.
  value(object_t&& new_value) : _value(std::move(new_value))
  {
  }

  /// Copy assignment operator.
  value& operator=(const value& other) = default;

  /// Move assignment operator.
  value& operator=(value&& other) = default;

  /// ToDo: Add assignment operators?

  /// Equality comparison operator.
  bool operator==(const value& other) const
  {
    return _value == other._value;
  }

  /// Inequality comparison operator.
  bool operator!=(const value& other) const
  {
    return !(*this == other);
  }

public:
  template <typename T>
  friend auto& get(value& instance);

  template <typename T>
  friend const auto& get(const value& instance);

  template <typename T>
  friend auto* get_if(value* instance);

  template <typename T>
  friend const auto* get_if(const value* instance);

  template <typename T>
  friend auto visit(T&& callable, const value& instance);

private:
  std::variant<std::nullptr_t, bool, double, std::string, array_t, object_t>
    _value;
};

using array = value::array_t;
using object = value::object_t;

///
template <typename T>
auto& get(value& instance)
{
  return std::get<T>(instance._value);
}

///
template <typename T>
const auto& get(const value& instance)
{
  return std::get<T>(instance._value);
}

///
template <typename T>
auto* get_if(value* instance)
{
  return std::get_if<T>(&instance->_value);
}

///
template <typename T>
const auto* get_if(const value* instance)
{
  return std::get_if<T>(&instance->_value);
}

///
template <typename T>
T& get(json::object& object, const std::string& name)
{
  auto value_iter = object.find(name);
  if (value_iter != object.end())
  {
    if (auto* value = json::get_if<T>(&value_iter->second))
      return *value;
    else
    {
      BOOST_THROW_EXCEPTION(parse_error() << core::context_info(
                              "JSON value is of unexpected type"));
    }
  }
  else
  {
    BOOST_THROW_EXCEPTION(parse_error()
                          << core::context_info("JSON value missing."));
  }
}

///
template <typename T>
const T& get(const json::object& object, const std::string& name)
{
  auto value_iter = object.find(name);
  if (value_iter != object.end())
  {
    if (auto* value = json::get_if<T>(&value_iter->second))
      return *value;
    else
    {
      BOOST_THROW_EXCEPTION(parse_error() << core::context_info(
                              "JSON value is of unexpected type"));
    }
  }
  else
  {
    BOOST_THROW_EXCEPTION(parse_error()
                          << core::context_info("JSON value missing."));
  }
}

///
template <typename T>
T* get_if(json::object& object, const std::string& name)
{
  auto value_iter = object.find(name);
  if (value_iter != object.end())
    return json::get_if<T>(&value_iter->second);
  return nullptr;
}

///
template <typename T>
const T* get_if(const json::object& object, const std::string& name)
{
  auto value_iter = object.find(name);
  if (value_iter != object.end())
    return json::get_if<T>(&value_iter->second);
  return nullptr;
}

///
template <typename T>
auto visit(T&& callable, const value& instance)
{
  return std::visit(std::forward<T>(callable), instance._value);
}

///
inline bool has(const json::object& object, const std::string& index)
{
  return object.find(index) != object.end();
}

/// Controls whether strings shall be escaped or not.
/// @remarks
///   When not specified string escaping is turned off but gets automatically
///   turned on when writing a JSON object. This does not happen when escaping
///   is turned off explicitely.
core::implementation::io_manipulator<bool> escape_strings(bool enable);

/// This operator will read the whole content of stream into a buffer, parse
/// this for json data and store the result in value.
std::istream& operator>>(std::istream& stream, json::value& value);

/// Print a JSON tree in formatted text form into the passed ostream.
/// @remarks
///   Note that the stream won't get flushed by this function.
/// @remarks
///   Use core::indent_multiplier to control tab width.
std::ostream& operator<<(std::ostream& stream, const json::object& object);

/// Print a JSON array in formatted text form into the passed ostream.
/// @remarks
///   Note that the stream won't get flushed by this function.
std::ostream& operator<<(std::ostream& stream, const json::array& values);

/// Print a JSON value in formatted text form into the passed ostream.
/// @remarks
///   Note that the stream won't get flushed by this function.
std::ostream& operator<<(std::ostream& stream, const json::value& value);
}

#endif
