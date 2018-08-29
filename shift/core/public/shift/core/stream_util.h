#ifndef SHIFT_CORE_STREAMUTIL_H
#define SHIFT_CORE_STREAMUTIL_H

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <tuple>
#include <ios>
#include <string>

namespace shift::core
{
namespace implementation
{
  /// An intermediate type to hold an io manipulator function and a user
  /// specified argument to be consumed by a custom overload of
  /// std::ostream& operator<<(std::ostream&, io_manipulator<T>&).
  template <typename T>
  class io_manipulator
  {
  public:
    using io_function_t = void (*)(std::ios_base&, T argument);

    /// Constructor.
    io_manipulator(io_function_t io_function, T argument)
    : _io_function(io_function), _argument(argument)
    {
    }

    friend std::ostream& operator<<(std::ostream& stream,
                                    const io_manipulator<T>& manipulator)
    {
      manipulator._io_function(stream, manipulator._argument);
      return stream;
    }

  private:
    io_function_t _io_function;
    T _argument;
  };

  ///
  int indent_level_index();

  ///
  int indent_character_index();

  ///
  int indent_width_index();

  ///
  void indent_character(std::ios_base& stream, int character);

  ///
  void indent_width(std::ios_base& stream, int width);
}

/// Increments the indentation depth.
std::ios_base& inc_indent(std::ios_base& stream);

/// Decrements the indentation depth.
std::ios_base& dec_indent(std::ios_base& stream);

/// Sets the character used for indenting. It defaults to the space character.
inline implementation::io_manipulator<int> indent_character(char character)
{
  return implementation::io_manipulator<int>(&implementation::indent_character,
                                             character);
}

/// Sets the character used for indenting. It defaults to the space character.
inline implementation::io_manipulator<int> indent_character(wchar_t character)
{
  return implementation::io_manipulator<int>(&implementation::indent_character,
                                             character);
}

/// Sets the number of times the indenting character shall be repeated per
/// indentation depth. It defaults to 2.
inline implementation::io_manipulator<int> indent_width(int width)
{
  return implementation::io_manipulator<int>(&implementation::indent_width,
                                             width);
}

/// Inserts width number of indentation characters into the stream.
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& tab(
  std::basic_ostream<CharT, Traits>& stream)
{
  auto width = stream.iword(implementation::indent_width_index());
  if (width <= 0)
    return stream;
  auto character =
    static_cast<CharT>(stream.iword(implementation::indent_character_index()));

  if (!character)
    character = static_cast<CharT>(' ');

  stream << std::basic_string<CharT>(width, character);
  return stream;
}

/// Inserts level * width number of indentation characters into the
/// stream.
template <class CharT, class Traits>
std::basic_ostream<CharT, Traits>& indent(
  std::basic_ostream<CharT, Traits>& stream)
{
  auto level = stream.iword(implementation::indent_level_index());
  if (level <= 0)
    return stream;
  auto width = stream.iword(implementation::indent_width_index());
  if (width <= 0)
    return stream;
  auto character =
    static_cast<CharT>(stream.iword(implementation::indent_character_index()));

  if (!character)
    character = static_cast<CharT>(' ');

  stream << std::basic_string<CharT>(width * level, character);
  return stream;
}

namespace implementation
{
  template <class T>
  auto serialize(std::ostream& stream, const T& object, int)
    -> decltype(object.serialize(stream), void())
  {
    object.serialize(stream);
  }

  template <typename T>
  void serialize(std::ostream& stream, const T& object, long)
  {
    stream.write(reinterpret_cast<const char*>(&object), sizeof(T));
  }

  template <class T>
  auto deserialize(std::istream& stream, T& object, int)
    -> decltype(object.deserialize(stream), void())
  {
    object.deserialize(stream);
  }

  template <typename T>
  void deserialize(std::istream& stream, T& object, long)
  {
    stream.read(reinterpret_cast<char*>(&object), sizeof(T));
  }
}

template <typename T>
struct binary_t
{
  T& data;

  friend std::ostream& operator<<(std::ostream& stream,
                                  const binary_t<T> reference)
  {
    implementation::serialize(stream, reference.data, 0);
    return stream;
  }

  friend std::istream& operator>>(std::istream& stream,
                                  const binary_t<T> reference)
  {
    implementation::deserialize(stream, reference.data, 0);
    return stream;
  }
};

template <typename T>
inline auto binary(T& data)
{
  return binary_t<T>{data};
}

template <typename T>
inline auto binary(const T& data)
{
  return binary_t<const T>{data};
}
}

#endif
