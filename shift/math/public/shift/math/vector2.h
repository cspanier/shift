#ifndef SHIFT_MATH_VECTOR2_H
#define SHIFT_MATH_VECTOR2_H

#include <cstdint>
#include <cmath>
#include <utility>
#include <iterator>
#include <ostream>
#include <shift/platform/assert.h>
#include <shift/core/contract.h>
#include <shift/core/algorithm.h>
#include "shift/math/math.h"

namespace shift::math
{
/// A two dimensional vector specialization.
template <typename T>
struct vector<2, T>
{
  static constexpr std::size_t row_count = 2;
  using value_type = T;

  constexpr vector() noexcept = default;
  constexpr vector(T x, T y) noexcept;
  constexpr vector(T fill_value) noexcept;

  constexpr vector(const vector&) noexcept = default;
  constexpr vector(vector&&) noexcept = default;
  ~vector() noexcept = default;
  constexpr vector& operator=(const vector&) noexcept = default;
  constexpr vector& operator=(vector&&) noexcept = default;

  /// @pre
  ///   The row selector must not exceed the size of the vector.
  constexpr T& operator()(std::size_t row) SHIFT_EXPECTS(row < row_count);

  /// @pre
  ///   The row selector must not exceed the size of the vector.
  constexpr T operator()(std::size_t row) const SHIFT_EXPECTS(row < row_count);

  /// Performs componentwise addition.
  template <typename U>
  vector& operator+=(const vector<2, U>& other);

  /// Performs componentwise subtraction.
  template <typename U>
  vector& operator-=(const vector<2, U>& other);

  /// Performs componentwise multiplication.
  template <typename U>
  vector& operator*=(const vector<2, U>& other);

  /// Performs componentwise division.
  template <typename U>
  vector& operator/=(const vector<2, U>& other);

  /// Performs componentwise modulus.
  template <typename U>
  vector& operator%=(const vector<2, U>& other);

  /// Multiplies each component with the passed scalar value.
  template <typename U>
  vector& operator*=(const U scalar);

  /// Divides each component by the passed scalar value.
  template <typename U>
  vector& operator/=(const U scalar);

  /// Performs componentwise modulus.
  template <typename U>
  vector& operator%=(const U scalar);

  ///
  template <typename... Args>
  constexpr vector& assign(Args&&... args);

  union {
    T x;
    T r;
    T s;
  };
  union {
    T y;
    T g;
    T t;
  };

private:
  /// Implementation of method assign.
  template <std::size_t Index, typename Arg, typename... Args>
  constexpr void assign_impl(Arg&& arg, Args&&... args);

  /// End of recursion.
  template <std::size_t Index>
  constexpr void assign_impl();
};

/// Equality operator.
template <typename U, typename V>
constexpr bool operator==(const vector<2, U>& lhs,
                          const vector<2, V>& rhs) noexcept
{
  return lhs.x == rhs.x && lhs.y == rhs.y;
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(const vector<2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() + std::declval<V>())>{
    lhs.x + rhs.x, lhs.y + rhs.y};
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(U lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() + std::declval<V>())>{
    lhs + rhs.x, lhs + rhs.y};
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(const vector<2, U>& lhs, V rhs)
{
  return vector<2, decltype(std::declval<U>() + std::declval<V>())>{
    lhs.x + rhs, lhs.y + rhs};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(const vector<2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() - std::declval<V>())>{
    lhs.x - rhs.x, lhs.y - rhs.y};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(U lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() - std::declval<V>())>{
    lhs - rhs.x, lhs - rhs.y};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(const vector<2, U>& lhs, V rhs)
{
  return vector<2, decltype(std::declval<U>() - std::declval<V>())>{
    lhs.x - rhs, lhs.y - rhs};
}

/// Performs componentwise negation.
template <typename U>
auto operator-(const vector<2, U>& value)
{
  return vector<2, U>{-value.x, -value.y};
}

/// Performs componentwise multiplication.
template <typename U, typename V>
auto operator*(const vector<2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() * std::declval<V>())>{
    lhs.x * rhs.x, lhs.y * rhs.y};
}

/// Performs componentwise division.
template <typename U, typename V>
auto operator/(const vector<2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() / std::declval<V>())>{
    lhs.x / rhs.x, lhs.y / rhs.y};
}

/// Performs componentwise modulus.
template <typename U, typename V>
auto operator%(const vector<2, U>& lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() % std::declval<V>())>{
    lhs.x % rhs.x, lhs.y % rhs.y};
}

/// Multiplies each component of rhs with the passed scalar value lhs.
template <typename U, typename V>
auto operator*(const U lhs, const vector<2, V>& rhs)
{
  return vector<2, decltype(std::declval<U>() * std::declval<V>())>{
    lhs * rhs.x, lhs * rhs.y};
}

/// Multiplies each component of lhs with the passed scalar value rhs.
template <typename U, typename V>
auto operator*(const vector<2, U>& lhs, const V rhs)
{
  return vector<2, decltype(std::declval<U>() * std::declval<V>())>{
    lhs.x * rhs, lhs.y * rhs};
}

/// Divides each component of lhs by the passed scalar value rhs.
template <typename U, typename V>
auto operator/(const vector<2, U>& lhs, const V rhs)
{
  return vector<2, decltype(std::declval<U>() / std::declval<V>())>{
    lhs.x / rhs, lhs.y / rhs};
}

/// Performs componentwise modulus.
template <typename U, typename V>
auto operator%(const vector<2, U>& lhs, const V rhs)
{
  return vector<2, decltype(std::declval<U>() % std::declval<V>())>{
    lhs.x % rhs, lhs.y % rhs};
}

/// Returns the dot product of lhs and rhs.
template <typename U, typename V>
constexpr auto dot(const vector<2, U>& lhs, const vector<2, V>& rhs) noexcept
{
  return lhs.x * rhs.x + lhs.y * rhs.y;
}

/// Returns the cross product of lhs and rhs.
template <typename U, typename V>
constexpr auto cross(const vector<2, U>& lhs, const vector<2, V>& rhs) noexcept
{
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

/// Applies std::floor to each vector component and returns the result.
template <typename U>
inline auto floor(const vector<2, U>& value) noexcept
{
  using std::floor;
  return vector<2, U>{floor(value.x), floor(value.y)};
}

/// Applies std::ceil to each vector component and returns the result.
template <typename U>
inline auto ceil(const vector<2, U>& value) noexcept
{
  using std::ceil;
  return vector<2, U>{ceil(value.x), ceil(value.y)};
}

/// Applies std::round to each vector component and returns the result.
template <typename U>
inline auto round(const vector<2, U>& value) noexcept
{
  using std::round;
  return vector<2, U>{round(value.x), round(value.y)};
}

/// Component-wise clamp function.
template <typename U>
constexpr auto clamp(const vector<2, U>& value, const vector<2, U>& lower_bound,
                     const vector<2, U>& upper_bound) noexcept
{
  using core::clamp;
  return vector<2, U>{clamp(value.x, lower_bound.x, upper_bound.x),
                      clamp(value.y, lower_bound.y, upper_bound.y)};
}

/// Returns the smallest component of the passed vector.
template <typename U>
constexpr U min(const vector<2, U>& value) noexcept
{
  using core::min;
  return min(value.x, value.y);
}

/// Returns the largest component of the passed vector.
template <typename U>
constexpr U max(const vector<2, U>& value) noexcept
{
  using core::max;
  return max(value.x, value.y);
}

/// Componentwise min function.
template <typename U>
constexpr auto min(const vector<2, U>& lhs, const vector<2, U>& rhs)
{
  using std::min;
  return vector<2, U>{min(lhs.x, rhs.x), min(lhs.y, rhs.y)};
}

/// Componentwise max function.
template <typename U>
constexpr auto max(const vector<2, U>& lhs, const vector<2, U>& rhs)
{
  using std::max;
  return vector<2, U>{max(lhs.x, rhs.x), max(lhs.y, rhs.y)};
}

/// Print the passed vector in human readable form to the output stream.
template <typename U>
std::ostream& operator<<(std::ostream& stream, const vector<2, U>& value)
{
  stream << '(' << value.x << ',' << value.y << ')';
  return stream;
}

template <typename T>
constexpr vector<2, T>::vector(T x, T y) noexcept : x(x), y(y)
{
}

template <typename T>
constexpr vector<2, T>::vector(T fill_value) noexcept
: x(fill_value), y(fill_value)
{
}

template <typename T>
constexpr T& vector<2, T>::operator()(std::size_t row)
{
  BOOST_ASSERT(row < row_count);
  switch (row)
  {
  case 0:
    return x;
  case 1:
    return y;
  }
  return x;
}

template <typename T>
constexpr T vector<2, T>::operator()(std::size_t row) const
{
  BOOST_ASSERT(row < row_count);
  switch (row)
  {
  case 0:
    return x;
  case 1:
    return y;
  default:
    return T{};
  }
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator+=(const vector<2, U>& other)
{
  x += other.x;
  y += other.y;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator-=(const vector<2, U>& other)
{
  x -= other.x;
  y -= other.y;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator*=(const vector<2, U>& other)
{
  x *= other.x;
  y *= other.y;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator/=(const vector<2, U>& other)
{
  x /= other.x;
  y /= other.y;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator*=(const U scalar)
{
  x *= scalar;
  y *= scalar;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator/=(const U scalar)
{
  x /= scalar;
  y /= scalar;
  return *this;
}

template <typename T>
template <typename U>
vector<2, T>& vector<2, T>::operator%=(const U scalar)
{
  x %= scalar;
  y %= scalar;
  return *this;
}

template <typename T>
template <typename... Args>
constexpr vector<2, T>& vector<2, T>::assign(Args&&... args)
{
  assign_impl<0>(std::forward<Args>(args)...);
  return *this;
}

template <typename T>
template <std::size_t Index, typename Arg, typename... Args>
constexpr void vector<2, T>::assign_impl(Arg&& arg, Args&&... args)
{
  constexpr auto components_count = detail::components_count_v<Arg>;
  if constexpr (components_count > 1)
  {
    for (auto index = 0u; index < components_count; ++index)
      (*this)(Index + index) = arg(index);
  }
  else
    (*this)(Index) = arg;
  assign_impl<Index + components_count>(std::forward<Args>(args)...);
}

template <typename T>
template <std::size_t Index>
constexpr void vector<2, T>::assign_impl()
{
  static_assert(Index == row_count);
}

/// Casts a vector<2, V> to vector<2, U>.
template <typename U, typename V>
constexpr auto static_cast_vector(const vector<2, V>& any_vector) noexcept
{
  return vector<2, U>{static_cast<U>(any_vector.x),
                      static_cast<U>(any_vector.y)};
}
}

#endif
