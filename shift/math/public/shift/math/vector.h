#ifndef SHIFT_MATH_VECTOR_H
#define SHIFT_MATH_VECTOR_H

#include <cstdint>
#include <cmath>
#include <utility>
#include <array>
#include <complex>
#include "shift/math/math.h"
#include "shift/math/utility.h"
#include "shift/math/vector1.h"
#include "shift/math/vector2.h"
#include "shift/math/vector3.h"
#include "shift/math/vector4.h"

namespace shift::math
{
/// Equality operator.
template <std::size_t Rows, typename U, typename V>
constexpr bool operator==(const vector<Rows, U>& lhs,
                          const vector<Rows, V>& rhs) noexcept
{
  for (std::size_t row = 0u; row < Rows; ++row)
  {
    if (lhs(row) != rhs(row))
      return false;
  }
  return true;
}

/// Inequality operator.
/// @remarks
///   Implemented in terms of !(lhs == rhs).
template <std::size_t Rows, typename U, typename V>
constexpr bool operator!=(const vector<Rows, U>& lhs,
                          const vector<Rows, V>& rhs) noexcept
{
  return !(lhs == rhs);
}

///
template <std::size_t Rows, typename U, typename V>
constexpr bool operator<(const vector<Rows, U>& lhs,
                         const vector<Rows, V>& rhs) noexcept
{
  return norm(lhs) < norm(rhs);
}

///
template <std::size_t Rows, typename U, typename V>
constexpr bool operator>(const vector<Rows, U>& lhs,
                         const vector<Rows, V>& rhs) noexcept
{
  return norm(lhs) > norm(rhs);
}

/// @remarks
///   Implemented in terms of operator>().
template <std::size_t Rows, typename U, typename V>
constexpr bool operator<=(const vector<Rows, U>& lhs,
                          const vector<Rows, V>& rhs) noexcept
{
  return !(rhs > lhs);
}

/// @remarks
///   Implemented in terms of operator<().
template <std::size_t Rows, typename U, typename V>
constexpr bool operator>=(const vector<Rows, U>& lhs,
                          const vector<Rows, V>& rhs) noexcept
{
  return !(rhs < lhs);
}

/// Componentwise comparison using almost_equal.
template <std::size_t Rows, typename T>
constexpr std::enable_if_t<std::is_floating_point_v<T>, bool> almost_equal(
  const vector<Rows, T>& lhs, const vector<Rows, T>& rhs,
  int units_in_the_last_place = 2) noexcept
{
  for (std::size_t row = 0u; row < Rows; ++row)
  {
    if (!almost_equal(lhs(row), rhs(row), units_in_the_last_place))
      return false;
  }
  return true;
}

/// Construct a vector by copying fill_value to each component.
template <std::size_t Rows, typename T>
constexpr auto make_default_vector(const T fill_value) noexcept
{
  return vector<Rows, T>(fill_value);
}

/// Constructs a vector filled with either signaling_nan<T>() or T{}, depending
/// on whether type T does have a signalling NaN value defined.
template <std::size_t Rows, typename T>
constexpr vector<Rows, T> make_nan_vector() noexcept
{
  if constexpr (std::numeric_limits<T>::has_signaling_NaN)
    return vector<Rows, T>(signaling_nan<T>());
  else
    return vector<Rows, T>(T{});
};

/// Makes a vector from a series of arguments.
/// @remarks
///   The resulting vector's dimension is automatically deduced from the sum of
///   argument components, i.e. passing other vectors as arguments to this
///   function splits those into components. The function also tries to
///   automatically deduce the resulting vector's type, which is based on the
///   result of operator* of the list of components.
template <typename... Args>
constexpr auto make_vector_from(Args&&... args) noexcept
{
  return vector<detail::components_count_v<Args...>,
                detail::select_type_t<Args...>>{}
    .assign(std::forward<Args>(args)...);
}

/// Makes a vector from a series of arguments.
/// @tparam T
///   The resulting vector's type.
/// @remarks
///   The resulting vector's dimension is automatically deduced from the sum of
///   argument components, i.e. passing other vectors as arguments to this
///   function splits those into components. All argument's components are cast
///   to the resulting vector's type.
template <typename T, typename... Args>
constexpr auto make_vector_from(Args&&... args) noexcept
{
  return vector<detail::components_count_v<Args...>, T>{}.assign(
    std::forward<Args>(args)...);
}

/// Construct a vector from a C-style array.
/// @remarks
///   If you need to initialize the vector from a pointer to a memory
///   region, you may use
///   make_vector<Rows>::from(*reinterpret_cast<T(*)[Rows]>(pointer)).
template <std::size_t Rows, typename T>
constexpr vector<Rows, T> make_vector_from(const T (&array)[Rows]) noexcept
{
  if constexpr (Rows == 1)
    return vector<Rows, T>{array[0]};
  else if constexpr (Rows == 2)
    return vector<Rows, T>{array[0], array[1]};
  else if constexpr (Rows == 3)
    return vector<Rows, T>{array[0], array[1], array[2]};
  else if constexpr (Rows == 4)
    return vector<Rows, T>{array[0], array[1], array[2], array[3]};
  else
  {
    vector<Rows, T> result;
    for (auto row = 0u; row < Rows; ++row)
      result(row) = array[row];
    return result;
  }
}

/// Construct a vector from a std::array.
template <std::size_t Rows, typename T>
constexpr auto make_vector_from(const std::array<T, Rows>& array) noexcept
{
  if constexpr (Rows == 1)
    return vector<Rows, T>{array[0]};
  else if constexpr (Rows == 2)
    return vector<Rows, T>{array[0], array[1]};
  else if constexpr (Rows == 3)
    return vector<Rows, T>{array[0], array[1], array[2]};
  else if constexpr (Rows == 4)
    return vector<Rows, T>{array[0], array[1], array[2], array[3]};
  else
  {
    vector<Rows, T> result;
    for (auto row = 0u; row < Rows; ++row)
      result(row) = array[row];
    return result;
  }
}

/// Casts a vector<Rows, V> to vector<Rows, U>.
/// @remarks
///   There are specialized overloads for 2-, 3-, and 4-dimensional vectors.
template <typename U, std::size_t Rows, typename V>
constexpr auto static_cast_vector(const vector<Rows, V>& any_vector) noexcept
{
  vector<Rows, U> result;
  for (auto row = 0u; row < Rows; ++row)
    result(row) = static_cast<U>(any_vector(row));
  return result;
}

/// Returns a vector composed from arbitrary components of another vector or
/// quaternion.
/// @remarks
///   This is an alternative syntax to the name swizzling known from e.g.
///   GLSL, so swizzle<1, 1, 0, 2>(vec) is equal to vec.yyxz.
template <std::size_t... Components, std::size_t Rows, typename T>
constexpr auto swizzle(const vector<Rows, T>& source) noexcept
{
  return vector<sizeof...(Components), T>{source(Components)...};
}

/// Returns the squared length of the vector v (|v|^2).
template <std::size_t Rows, typename U>
constexpr U norm(const vector<Rows, U>& v) noexcept
{
  return dot(v, v);
}

/// Returns the length of the vector v (|v|).
template <std::size_t Rows, typename U>
constexpr auto abs(const vector<Rows, U>& v) noexcept
{
  using std::sqrt;
  return sqrt(norm(v));
}

/// Applies std::floor to each vector component and returns the result.
template <std::size_t Rows, typename U>
inline auto floor(vector<Rows, U> value) noexcept
{
  using std::floor;
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = floor(value(row));
  return result;
}

/// Applies std::ceil to each vector component and returns the result.
template <std::size_t Rows, typename U>
inline auto ceil(vector<Rows, U> value) noexcept
{
  using std::ceil;
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = ceil(value(row));
  return result;
}

/// Applies std::round to each vector component and returns the result.
template <std::size_t Rows, typename U>
inline auto round(vector<Rows, U> value) noexcept
{
  using std::round;
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = round(value(row));
  return result;
}

/// Component-wise clamp function.
template <std::size_t Rows, typename U>
constexpr auto clamp(const vector<Rows, U>& value,
                     const vector<Rows, U>& lower_bound,
                     const vector<Rows, U>& upper_bound) noexcept
{
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = core::clamp(value(row), lower_bound(row), upper_bound(row));
  return result;
}

/// Reflects a direction vector on a plane with the passed normal.
template <std::size_t Rows, typename U, typename V>
constexpr auto reflect(const vector<Rows, U>& direction,
                       const vector<Rows, V>& normal) noexcept
{
  return direction - 2 * dot(direction, normal) * normal;
}

/// Generates a step function by comparing v(i) to edge.
template <std::size_t Rows, typename U>
auto step(U edge, const vector<Rows, U>& value) noexcept
{
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = core::step(edge, value(row));
  return result;
}

/// Generates a step function by comparing v(i) to edge(i).
template <std::size_t Rows, typename U>
auto step(const vector<Rows, U>& edge, const vector<Rows, U>& value) noexcept
{
  vector<Rows, U> result{};
  for (std::size_t row = 00; row < Rows; ++row)
    result(row) = core::step(edge(row), value(row));
  return result;
}

/// Normalizes a direction vector.
template <std::size_t Rows, typename U>
auto normalize(const vector<Rows, U>& direction) noexcept
{
  using std::abs;
  auto magnitude = abs(direction);
  if (magnitude == 0)
    return vector<Rows, decltype(magnitude)>{};
  return direction / magnitude;
}

/// Returns the smallest component of the passed vector.
template <std::size_t Rows, typename U>
constexpr U min(const vector<Rows, U>& value) noexcept
{
  auto result = value(0);
  for (std::size_t row = 1u; row < Rows; ++row)
  {
    if (value(row) < result)
      result = value(row);
  }
  return result;
}

/// Returns the largest component of the passed vector.
template <std::size_t Rows, typename U>
constexpr U max(const vector<Rows, U>& value) noexcept
{
  auto result = value(0);
  for (std::size_t row = 1u; row < Rows; ++row)
  {
    if (value(row) > result)
      result = value(row);
  }
  return result;
}

/// Componentwise min function.
template <std::size_t Rows, typename U>
constexpr auto min(const vector<Rows, U>& v1,
                   const vector<Rows, U>& v2) noexcept
{
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = v1(row) <= v2(row) ? v1(row) : v2(row);
  return result;
}

/// Componentwise max function.
template <std::size_t Rows, typename U>
constexpr auto max(const vector<Rows, U>& v1,
                   const vector<Rows, U>& v2) noexcept
{
  vector<Rows, U> result{};
  for (std::size_t row = 0u; row < Rows; ++row)
    result(row) = v1(row) >= v2(row) ? v1(row) : v2(row);
  return result;
}
}

#endif
