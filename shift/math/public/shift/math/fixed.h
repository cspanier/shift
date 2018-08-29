#ifndef SHIFT_MATH_FIXED_H
#define SHIFT_MATH_FIXED_H

#include <type_traits>
#include <boost/operators.hpp>
#include "shift/core/types.h"

namespace shift::math
{
template <typename K, std::size_t F>
class fixed;
}

namespace std
{
/// Returns the squared magnitude of the scalar value (|value|^2).
template <typename K, std::size_t F>
shift::math::fixed<K, F> norm(const shift::math::fixed<K, F>& value)
{
  return value * value;
}

/// Returns the magnitude of the scalar value (|value|).
template <typename K, std::size_t F>
shift::math::fixed<K, F> abs(shift::math::fixed<K, F> value)
{
  value.data() = std::abs(value.data());
  return value;
}

///
template <typename K, std::size_t F>
shift::math::fixed<K, F> floor(shift::math::fixed<K, F> value)
{
  if ((value.data() & shift::math::fixed<K, F>::fractional_mask) != 0)
  {
    if (value.data() >= 0)
      return shift::math::fixed<K, F>(static_cast<K>(value));
    else
      return shift::math::fixed<K, F>(static_cast<K>(value) - 1);
  }
  else
    return value;
}

///
template <typename K, std::size_t F>
shift::math::fixed<K, F> ceil(shift::math::fixed<K, F> value)
{
  if (value.data() & shift::math::fixed<K, F>::fractional_mask != 0)
  {
    if (value.data() >= 0)
      return shift::math::fixed<K, F>(static_cast<K>(value) + 1);
    else
      return shift::math::fixed<K, F>(static_cast<K>(value));
  }
  else
    return value;
}

///
template <typename K, std::size_t F>
shift::math::fixed<K, F> round(shift::math::fixed<K, F> value)
{
  return shift::math::fixed<K, F>(
    static_cast<K>(value + shift::math::fixed<K, F>::one / 2));
}
}

namespace shift::math
{
/// A fixed-point arithmetic class
template <typename K, std::size_t F>
class fixed : boost::operators<fixed<K, F>>, boost::shiftable<fixed<K, F>, int>
{
public:
  static_assert(std::is_integral<K>::value,
                "K in fixed<K, F> must be a signed integral type.");

  using storage_type = K;

  static const std::size_t total_bits = sizeof(K);
  static const std::size_t fractional_bits = F;
  static const std::size_t fractional_mask = (1 << F) - 1;
  static const std::size_t integer_mask = ~fractional_mask;
  static const std::size_t integer_bits = total_bits - F;
  static const K one = K(1) << fractional_bits;

  struct direct_copy
  {
  };

public:
  /// Default constructor.
  fixed() : _number(0)
  {
  }

  /// Copy constructor.
  fixed(const fixed& other) : _number(other._number)
  {
  }

  /// Copy constructor.
  /// ToDo...
  // template <typename K2, std::size_t F2>
  // fixed(const fixed<K2, F2>& other) : _number(other._number)
  //{
  //}

  /// Constructor which interprets the passed value as correctly formatted
  /// fixed point number.
  fixed(K value, direct_copy&) : _number(value)
  {
  }

  /// Constructor from a signed integral type T.
  template <typename T, ENABLE_IF(std::is_integral<T>::value)>
  fixed(T number) : _number(static_cast<K>(number) << fractional_bits)
  {
  }

  /// Constructor from a floating-point type.
  template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
  fixed(T number) : _number(static_cast<K>(one * number))
  {
  }

  /// Conversion operator to signed integral type T.
  template <typename T, ENABLE_IF(std::is_integral<T>::value)>
  operator T() const
  {
    static_assert(sizeof(T) * 8 >= sizeof(K) * 8 - fractional_bits,
                  "Cannot convert fixed-point number to type T because of "
                  "possible loss of data.");
    return static_cast<T>(_number >> fractional_bits);
  }

  /// Conversion operator to floating-point type T.
  template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
  operator T() const
  {
    return static_cast<T>(_number) / (1 << fractional_bits);
  }

  /// Returns a reference to the internal number store.
  storage_type& data()
  {
    return _number;
  }

  /// Assignment operator.
  fixed& operator=(const fixed& other)
  {
    _number = other._number;
    return *this;
  }

  ///
  bool operator==(const fixed& other) const
  {
    return _number == other._number;
  }

  ///
  bool operator<(const fixed& other) const
  {
    return _number < other._number;
  }

  ///
  bool operator!() const
  {
    return !_number;
  }

  ///
  fixed operator~() const
  {
    return fixed(~_number, direct_copy());
  }

  ///
  fixed operator-() const
  {
    return fixed(-t._number, direct_copy());
  }

  ///
  fixed& operator++()
  {
    _number += one;
    return *this;
  }

  ///
  fixed& operator--()
  {
    _number -= one;
    return *this;
  }

  ///
  fixed& operator+=(const fixed& other)
  {
    _number += other._number;
    return *this;
  }

  ///
  fixed& operator-=(const fixed& other)
  {
    _number -= other._number;
    return *this;
  }

  ///
  fixed& operator&=(const fixed& other)
  {
    _number &= other._number;
    return *this;
  }

  ///
  fixed& operator|=(const fixed& other)
  {
    _number |= other._number;
    return *this;
  }

  ///
  fixed& operator^=(const fixed& other)
  {
    _number ^= other._number;
    return *this;
  }

  ///
  fixed& operator>>=(const int& other)
  {
    _number >>= other;
    return *this;
  }

  ///
  fixed& operator<<=(const int& other)
  {
    _number <<= other;
    return *this;
  }

  ///
  fixed& operator>>=(const fixed& other)
  {
    _number >>= static_cast<K>(other);
    return *this;
  }

  ///
  fixed& operator<<=(const fixed& other)
  {
    _number <<= static_cast<K>(other);
    return *this;
  }

private:
  storage_type _number;
};
}

#endif
