#ifndef SHIFT_MATH_VECTOR1_H
#define SHIFT_MATH_VECTOR1_H

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
/// A seemingly useless one dimensional vector specialization which is used in
/// generic programming only.
template <typename T>
struct vector<1, T>
{
  static constexpr std::size_t row_count = 1;
  using value_type = T;

  constexpr vector() noexcept = default;
  constexpr vector(T x) noexcept;

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
  vector& operator+=(const vector<1, U>& other);

  /// Performs componentwise subtraction.
  template <typename U>
  vector& operator-=(const vector<1, U>& other);

  /// Performs componentwise multiplication.
  template <typename U>
  vector& operator*=(const vector<1, U>& other);

  /// Performs componentwise division.
  template <typename U>
  vector& operator/=(const vector<1, U>& other);

  /// Performs componentwise modulus.
  template <typename U>
  vector& operator%=(const vector<1, U>& other);

  /// Multiplies each component with the passed scalar value.
  template <typename U>
  vector& operator*=(U scalar);

  /// Divides each component by the passed scalar value.
  template <typename U>
  vector& operator/=(U scalar);

  /// Performs componentwise modulus.
  template <typename U>
  vector& operator%=(U scalar);

  union {
    T x;
    T r;
    T s;
  };
};

/// Equality operator.
template <typename U, typename V>
constexpr bool operator==(const vector<1, U>& lhs,
                          const vector<1, V>& rhs) noexcept
{
  return lhs.x == rhs.x;
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(const vector<1, U>& lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() + std::declval<V>())>{lhs.x +
                                                                    rhs.x};
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(U lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() + std::declval<V>())>{lhs +
                                                                    rhs.x};
}

/// Performs componentwise addition.
template <typename U, typename V>
auto operator+(const vector<1, U>& lhs, V rhs)
{
  return vector<1, decltype(std::declval<U>() + std::declval<V>())>{lhs.x +
                                                                    rhs};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(const vector<1, U>& lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() - std::declval<V>())>{lhs.x -
                                                                    rhs.x};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(U lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() - std::declval<V>())>{lhs -
                                                                    rhs.x};
}

/// Performs componentwise subtraction.
template <typename U, typename V>
auto operator-(const vector<1, U>& lhs, V rhs)
{
  return vector<1, decltype(std::declval<U>() - std::declval<V>())>{lhs.x -
                                                                    rhs};
}

/// Performs componentwise negation.
template <typename U>
auto operator-(const vector<1, U>& value)
{
  return vector<1, U>{-value.x};
}

/// Performs componentwise multiplication.
template <typename U, typename V>
auto operator*(const vector<1, U>& lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() * std::declval<V>())>{lhs.x *
                                                                    rhs.x};
}

/// Performs componentwise division.
template <typename U, typename V>
auto operator/(const vector<1, U>& lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() / std::declval<V>())>{lhs.x /
                                                                    rhs.x};
}

/// Performs componentwise modulus.
template <typename U, typename V>
auto operator%(const vector<1, U>& lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() % std::declval<V>())>{lhs.x %
                                                                    rhs.x};
}

/// Multiplies each component of rhs with the passed scalar value lhs.
template <typename U, typename V>
auto operator*(const U lhs, const vector<1, V>& rhs)
{
  return vector<1, decltype(std::declval<U>() * std::declval<V>())>{lhs *
                                                                    rhs.x};
}

/// Multiplies each component of lhs with the passed scalar value rhs.
template <typename U, typename V>
auto operator*(const vector<1, U>& lhs, const V rhs)
{
  return vector<1, decltype(std::declval<U>() * std::declval<V>())>{lhs.x *
                                                                    rhs};
}

/// Divides each component of lhs by the passed scalar value rhs.
template <typename U, typename V>
auto operator/(const vector<1, U>& lhs, const V rhs)
{
  return vector<1, decltype(std::declval<U>() / std::declval<V>())>{lhs.x /
                                                                    rhs};
}

/// Performs componentwise modulus.
template <typename U, typename V>
auto operator%(const vector<1, U>& lhs, const V rhs)
{
  return vector<1, decltype(std::declval<U>() % std::declval<V>())>{lhs.x %
                                                                    rhs};
}

/// Returns the dot product of lhs and rhs.
template <typename U, typename V>
constexpr auto dot(const vector<1, U>& lhs, const vector<1, V>& rhs) noexcept
{
  return lhs.x * rhs.x;
}

/// Applies std::floor to each vector component and returns the result.
template <typename U>
inline auto floor(const vector<1, U>& value) noexcept
{
  using std::floor;
  return vector<1, U>{floor(value.x)};
}

/// Applies std::ceil to each vector component and returns the result.
template <typename U>
inline auto ceil(const vector<1, U>& value) noexcept
{
  using std::ceil;
  return vector<1, U>{ceil(value.x)};
}

/// Applies std::round to each vector component and returns the result.
template <typename U>
inline auto round(const vector<1, U>& value) noexcept
{
  using std::round;
  return vector<1, U>{round(value.x)};
}

/// Component-wise clamp function.
template <typename U>
constexpr auto clamp(const vector<1, U>& value, const vector<1, U>& lower_bound,
                     const vector<1, U>& upper_bound) noexcept
{
  using core::clamp;
  return vector<1, U>{clamp(value.x, lower_bound.x, upper_bound.x)};
}

/// Returns the smallest component of the passed vector.
template <typename U>
constexpr U min(const vector<1, U>& value) noexcept
{
  using core::min;
  return min(value.x);
}

/// Returns the largest component of the passed vector.
template <typename U>
constexpr U max(const vector<1, U>& value) noexcept
{
  using core::max;
  return max(value.x);
}

/// Componentwise min function.
template <typename U>
constexpr auto min(const vector<1, U>& lhs, const vector<1, U>& rhs)
{
  using std::min;
  return vector<1, U>{min(lhs.x, rhs.x)};
}

/// Componentwise max function.
template <typename U>
constexpr auto max(const vector<1, U>& lhs, const vector<1, U>& rhs)
{
  using std::max;
  return vector<1, U>{max(lhs.x, rhs.x)};
}

/// Print the passed vector in human readable form to the output stream.
template <typename U>
std::ostream& operator<<(std::ostream& stream, const vector<1, U>& value)
{
  stream << '(' << value.x << ')';
  return stream;
}

template <typename T>
constexpr vector<1, T>::vector(T x) noexcept : x(x)
{
}

template <typename T>
constexpr T& vector<1, T>::operator()(std::size_t row)
{
  BOOST_ASSERT(row < row_count);
  return x;
}

template <typename T>
constexpr T vector<1, T>::operator()(std::size_t row) const
{
  BOOST_ASSERT(row < row_count);
  return x;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator+=(const vector<1, U>& other)
{
  x += other.x;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator-=(const vector<1, U>& other)
{
  x -= other.x;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator*=(const vector<1, U>& other)
{
  x *= other.x;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator/=(const vector<1, U>& other)
{
  x /= other.x;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator*=(const U scalar)
{
  x *= scalar;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator/=(const U scalar)
{
  x /= scalar;
  return *this;
}

template <typename T>
template <typename U>
vector<1, T>& vector<1, T>::operator%=(const U scalar)
{
  x %= scalar;
  return *this;
}

/// Casts a vector<1, V> to vector<1, U>.
template <typename U, typename V>
constexpr auto static_cast_vector(const vector<1, V>& any_vector) noexcept
{
  return vector<1, U>{static_cast<U>(any_vector.x)};
}
}

#endif
