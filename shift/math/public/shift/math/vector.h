#ifndef SHIFT_MATH_VECTOR_H
#define SHIFT_MATH_VECTOR_H

#include <cstdint>
#include <utility>
#include <array>
#include <cmath>
#include <complex>
#include <iostream>
#include <shift/core/types.h>
#include <shift/core/core.h>
#include "shift/math/math.h"
#include "shift/math/utility.h"
#include "shift/math/matrix.h"

namespace shift::math
{
/// Equality operator.
template <std::size_t Rows, typename T,
          std::enable_if_t<std::is_floating_point<T>::value>* = nullptr>
inline constexpr bool operator==(const vector<Rows, T>& lhs,
                                 const vector<Rows, T>& rhs) noexcept
{
  using std::abs;
  for (std::size_t i = 0; i < Rows; ++i)
  {
    if (!almost_equal(lhs(i), rhs(i), 8388608))
      return false;
  }
  return true;
}

/// Equality operator.
template <std::size_t Rows, typename T,
          std::enable_if_t<!std::is_floating_point<T>::value>* = nullptr>
inline constexpr bool operator==(const vector<Rows, T>& lhs,
                                 const vector<Rows, T>& rhs) noexcept
{
  for (std::size_t i = 0; i < Rows; ++i)
  {
    if (lhs(i) != rhs(i))
      return false;
  }
  return true;
}

/// Inequality operator.
template <std::size_t Rows, typename T>
inline constexpr bool operator!=(const vector<Rows, T>& lhs,
                                 const vector<Rows, T>& rhs) noexcept
{
  return !(lhs == rhs);
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator+(const vector<Rows, T>& lhs,
                                const vector<Rows, U>& rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() + std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) + rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator+(const vector<Rows, T>& lhs, U rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() + std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) + rhs;
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator+(T lhs, const vector<Rows, U>& rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() + std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs + rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator-(const vector<Rows, T>& lhs,
                                const vector<Rows, U>& rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() - std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) - rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator-(const vector<Rows, T>& lhs, U rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() - std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) - rhs;
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator-(U lhs, const vector<Rows, T>& rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() - std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs - rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T>
inline constexpr auto operator-(const vector<Rows, T>& value) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = -value(row);
  return result;
}

/// Component-wise vector multiplication.
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator*(const vector<Rows, T>& lhs,
                                const vector<Rows, U>& rhs) noexcept
{
  vector<Rows, decltype(std::declval<T>() * std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) * rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator%(const vector<Rows, T>& lhs,
                                const vector<Rows, U>& rhs)
{
  vector<Rows, decltype(std::declval<T>() % std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) % rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator%(T lhs, const vector<Rows, U>& rhs)
{
  vector<Rows, decltype(std::declval<T>() % std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs % rhs(row);
  return result;
}

///
template <std::size_t Rows, typename T, typename U>
inline constexpr auto operator%(const vector<Rows, T>& lhs, U rhs)
{
  vector<Rows, decltype(std::declval<T>() % std::declval<U>())> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = lhs(row) % rhs;
  return result;
}

///
template <std::size_t Rows, typename T>
inline constexpr bool operator<(const vector<Rows, T>& lhs,
                                const vector<Rows, T>& rhs) noexcept
{
  using std::norm;
  return norm(lhs) < norm(rhs);
}

///
template <std::size_t Rows, typename T>
inline constexpr bool operator>(const vector<Rows, T>& lhs,
                                const vector<Rows, T>& rhs) noexcept
{
  using std::norm;
  return norm(lhs) > norm(rhs);
}

///
template <std::size_t Rows, typename T>
inline constexpr bool operator<=(const vector<Rows, T>& lhs,
                                 const vector<Rows, T>& rhs) noexcept
{
  return !(rhs > lhs);
}

///
template <std::size_t Rows, typename T>
inline constexpr bool operator>=(const vector<Rows, T>& lhs,
                                 const vector<Rows, T>& rhs) noexcept
{
  return !(rhs < lhs);
}

/// Vector addition.
template <std::size_t Rows, typename T, typename U>
vector<Rows, T>& operator+=(vector<Rows, T>& lhs,
                            const vector<Rows, U>& rhs) noexcept
{
  lhs = static_cast_matrix<T>(lhs + rhs);
  return lhs;
}

/// Vector subtraction.
template <std::size_t Rows, typename T, typename U>
vector<Rows, T>& operator-=(vector<Rows, T>& lhs,
                            const vector<Rows, U>& rhs) noexcept
{
  lhs = static_cast_matrix<T>(lhs - rhs);
  return lhs;
}

/// Scalar product.
template <std::size_t Rows, typename T, typename U>
vector<Rows, T>& operator*=(vector<Rows, T>& lhs, U factor) noexcept
{
  lhs = static_cast_matrix<T>(lhs * factor);
  return lhs;
}

/// Scalar division.
template <std::size_t Rows, typename T, typename U>
vector<Rows, T>& operator/=(vector<Rows, T>& lhs, U divisor)
{
  lhs = static_cast_matrix<T>(lhs / divisor);
  return lhs;
}

/// Component-wise modulus assignment.
template <std::size_t Rows, typename T, typename U>
vector<Rows, T>& operator%=(vector<Rows, T>& lhs, U divisor)
{
  lhs = static_cast_matrix<T>(lhs % divisor);
  return lhs;
}

///
template <std::size_t Rows, typename T>
inline std::ostream& operator<<(std::ostream& stream,
                                const math::vector<Rows, T>& vector)
{
  stream << "(";
  for (std::size_t row = 0; row < Rows; ++row)
  {
    if (row != 0)
      stream << ", ";
    stream << vector(row);
  }
  stream << ")";

  return stream;
}

///
template <std::size_t Rows, typename T>
inline std::istream& operator>>(std::istream& stream,
                                math::vector<Rows, T>& vector)
{
  stream.get();
  for (std::size_t i = 0; i < Rows; ++i)
  {
    if (i != 0)
      stream.get();
    typename std::conditional<std::is_same<T, unsigned char>::value,
                              unsigned short, T>::M component;
    if ((stream >> component).fail())
      break;
    vector(i) = static_cast<T>(component);
  }
  stream.get();

  if (stream.fail())
    vector = math::vector<Rows, T>();

  return stream;
}

/// Construct a vector by copying fill_value to each component.
template <std::size_t Rows, typename T>
inline constexpr auto make_default_vector(const T fill_value) noexcept
{
  return make_default_matrix<Rows, 1, T>(fill_value);
}

/// Constructs a vector filled with either signaling_nan<T>() or T{0}, depending
/// on whether type T does have a signalling NaN value defined.
template <std::size_t Rows, typename T>
constexpr vector<Rows, T> make_nan_vector() noexcept
{
  return make_nan_matrix<Rows, 1, T>();
};

/// Construct a vector from a series of values.
template <typename... Ts>
constexpr auto make_vector_from(Ts&&... values) noexcept
{
  return make_matrix_from_column_major<
    detail::count_elements<std::decay_t<Ts>...>::count(), 1,
    detail::element_type_t<std::decay_t<core::first_in_pack_t<Ts...>>>>(
    std::forward<Ts>(values)...);
}

/// Construct a vector from a C-style array.
/// @remarks
///   If you need to initialize the vector from a pointer to a memory
///   region, you may use
///   make_vector<Rows>::from(*reinterpret_cast<T(*)[Rows]>(pointer)).
template <std::size_t Rows, typename T>
constexpr vector<Rows, T> make_vector_from(const T (&array)[Rows]) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = array[row];
  return result;
}

/// Construct a vector from a std::array.
template <std::size_t Rows, typename T>
constexpr auto make_vector_from(const std::array<T, Rows>& array) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = array[row];
  return result;
}

/// Makes a unit vector in the direction of the given axis.
template <std::size_t Rows, typename T, std::size_t Axis>
inline constexpr auto make_unit_vector() noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = (row == Axis) ? T{1} : T{0};
  return result;
}

///
template <typename U, std::size_t Rows, typename T>
inline constexpr auto static_cast_vector(const vector<Rows, T>& v) noexcept
{
  return static_cast_matrix<U>(v);
}

/// Returns a vector composed from arbitrary components of another vector or
/// quaternion.
/// @remarks
///   This is an alternative syntax to the name swizzling known from e.g.
///   GLSL, so swizzle<1, 1, 0, 2>(vec) is equal to vec.yyxz.
template <std::size_t... Components, std::size_t Rows, typename T>
inline constexpr auto swizzle(const vector<Rows, T>& source) noexcept
{
  return vector<sizeof...(Components), T>{source(Components)...};
}

/// Returns the squared length of the vector v (|v|^2).
template <std::size_t Rows, typename T>
T norm(const vector<Rows, T>& v) noexcept
{
  return dot(v, v);
}

/// Returns the length of the vector v (|v|).
template <std::size_t Rows, typename T,
          std::enable_if_t<!std::is_integral<T>::value>* = nullptr>
T abs(const vector<Rows, T>& v) noexcept
{
  using std::sqrt;
  return sqrt(norm(v));
}

/// Returns the length of the vector v (|v|).
/// @remarks
///   This overload for vectors of integral element data type converts the
///   argument to a vector of type R first.
template <typename R = float, std::size_t Rows, typename T,
          std::enable_if_t<std::is_integral<T>::value>* = nullptr>
R abs(const vector<Rows, T>& v) noexcept
{
  using std::abs;
  return abs(static_cast_matrix<R>(v));
}

/// Applies std::floor to each vector component and returns the result.
template <std::size_t Rows, typename T>
inline auto floor(vector<Rows, T> v) noexcept
{
  using std::floor;
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = floor(v(row));
  return result;
}

/// Applies std::ceil to each vector component and returns the result.
template <std::size_t Rows, typename T>
inline auto ceil(vector<Rows, T> v) noexcept
{
  using std::ceil;
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = ceil(v(row));
  return result;
}

/// Applies std::round to each vector component and returns the result.
template <std::size_t Rows, typename T>
inline auto round(vector<Rows, T> v) noexcept
{
  using std::round;
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = round(v(row));
  return result;
}

/// Component-wise clamp function.
template <std::size_t Rows, typename T>
inline constexpr auto clamp(const vector<Rows, T>& v,
                            const vector<Rows, T>& lower_bound,
                            const vector<Rows, T>& upper_bound) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = core::clamp(v(row), lower_bound(row), upper_bound(row));
  return result;
}

/// Normalizes a direction vector.
template <std::size_t Rows, typename T>
auto normalize(const vector<Rows, T>& direction) noexcept
{
  using std::abs;
  auto magnitude = abs(direction);
  if (magnitude == 0)
    return vector<Rows, decltype(magnitude)>{};
  return direction / magnitude;
}

/// Returns the smallest component of the passed vector.
template <std::size_t Rows, typename T>
inline constexpr T min(const vector<Rows, T>& v) noexcept
{
  T result = v(0);
  for (std::size_t row = 1; row < Rows; ++row)
  {
    if (v(row) < result)
      result = v(row);
  }
  return result;
}

/// Returns the largest component of the passed vector.
template <std::size_t Rows, typename T>
inline constexpr T max(const vector<Rows, T>& v) noexcept
{
  T result = v(0);
  for (std::size_t row = 1; row < Rows; ++row)
  {
    if (v(row) > result)
      result = v(row);
  }
  return result;
}

/// Componentwise min function.
template <std::size_t Rows, typename T>
inline constexpr auto min(const vector<Rows, T>& v1, const vector<Rows, T>& v2)
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = v1(row) <= v2(row) ? v1(row) : v2(row);
  return result;
}

/// Componentwise max function.
template <std::size_t Rows, typename T>
inline constexpr vector<Rows, T> max(const vector<Rows, T>& v1,
                                     const vector<Rows, T>& v2)
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = v1(row) >= v2(row) ? v1(row) : v2(row);
  return result;
}

/// Returns the dot product of lhs and rhs.
template <std::size_t Rows, typename T, typename U>
inline auto dot(const vector<Rows, T>& lhs, const vector<Rows, U>& rhs) noexcept
{
  decltype(std::declval<T>() * std::declval<U>()) result = 0;
  for (std::size_t row = 0; row < Rows; ++row)
    result += lhs(row) * rhs(row);
  return result;
}

/// Returns the cross product of the 2 dimensional vectors lhs and rhs.
template <typename T, typename U>
inline constexpr auto cross(const vector<2, T>& lhs,
                            const vector<2, U>& rhs) noexcept
{
  return lhs.x * rhs.y - lhs.y * rhs.x;
}

/// Returns the cross product of the 3 dimensional vectors lhs and rhs.
template <typename T, typename U>
inline constexpr auto cross(const vector<3, T>& lhs,
                            const vector<3, U>& rhs) noexcept
{
  using result_t = vector<3, decltype(std::declval<T>() * std::declval<U>())>;
  return result_t{lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z,
                  lhs.x * rhs.y - lhs.y * rhs.x};
}

/// Reflects a direction vector on a plane with the passed normal.
template <std::size_t Rows, typename T>
constexpr auto reflect(const vector<Rows, T>& direction,
                       const vector<Rows, T>& normal) noexcept
{
  return direction - 2 * dot(direction, normal) * normal;
}

/// Generates a step function by comparing v(i) to edge.
template <std::size_t Rows, typename T>
auto step(T edge, const vector<Rows, T>& v) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = core::step(edge, v(row));
  return result;
}

/// Generates a step function by comparing v(i) to edge(i).
template <std::size_t Rows, typename T>
auto step(const vector<Rows, T>& edge, const vector<Rows, T>& v) noexcept
{
  vector<Rows, T> result{};
  for (std::size_t row = 0; row < Rows; ++row)
    result(row) = core::step(edge(row), v(row));
  return result;
}
}

#endif
