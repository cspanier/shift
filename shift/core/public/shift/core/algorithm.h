#ifndef SHIFT_CORE_ALGORITHM_H
#define SHIFT_CORE_ALGORITHM_H

#include <cmath>
#include <array>
#include <tuple>
#include "shift/core/core.h"
#include "shift/core/types.h"

#if defined min
#undef min
#endif
#if defined max
#undef max
#endif

namespace shift::core
{
///
template <typename T>
inline constexpr T min(T arg) noexcept
{
  return arg;
}

///
template <typename T, typename... Ts>
inline constexpr T min(T lhs, T rhs, Ts... args) noexcept
{
  return min(lhs < rhs ? lhs : rhs, args...);
}

///
template <typename T>
inline constexpr T max(T arg) noexcept
{
  return arg;
}

///
template <typename T, typename... Ts>
inline constexpr T max(T lhs, T rhs, Ts... args) noexcept
{
  return max(rhs < lhs ? lhs : rhs, args...);
}

/// Return the value constrained to the range min_value to max_value.
template <typename T>
inline constexpr T clamp(T value, T lower_bound, T upper_bound) noexcept
{
  return min(max(value, lower_bound), upper_bound);
}

/// Return the linear interpolation between a and b using 0 <= t <= 1.
template <typename T>
inline constexpr T lerp(T a, T b, float t) noexcept
{
  return a * (1 - t) + b * t;
}

/// Returns the fractional part of value.
template <typename T>
inline constexpr T fract(T value)
{
  return value - std::floor(value);
}

/// Generates a step function by comparing x to edge.
template <typename T>
inline constexpr T step(T edge, T x)
{
  return static_cast<T>(x < edge ? 0 : 1);
}

/// Perform a linear interpolation between x and y using w to weight between
/// them.
template <typename Tx, typename Ty, typename Tw>
inline constexpr auto mix(Tx x, Ty y, Tw w)
{
  return x * (1 - w) + y * w;
}

/// Returns an array whose elements are aggregate initialized with the default
/// constructed value of I.
template <typename T, typename I, std::size_t... Is>
std::array<T, sizeof...(Is)> make_array(std::index_sequence<Is...> /*unused*/)
{
  return {T{std::conditional_t<(Is || true), I, int>{}}...};
}

/// Returns an array of values.
template <typename... Ts>
inline constexpr std::array<std::decay_t<first_in_pack_t<Ts...>>, sizeof...(Ts)>
make_array(Ts&&... values) noexcept
{
  return {{std::forward<Ts>(values)...}};
}

/// Returns an array of values.
template <typename... Ts>
inline constexpr std::array<std::decay_t<first_in_pack_t<Ts...>>, sizeof...(Ts)>
make_array(const Ts&... values) noexcept
{
  return {values...};
}

/// Counts the number of 1-bits in value.
std::uint32_t hamming_weight(std::uint32_t value);

/// Accesses an std::array using an enumeration key.
template <typename T, std::size_t N, typename Enum>
constexpr auto& at(std::array<T, N>& array, Enum key)
{
  return array[static_cast<std::underlying_type_t<Enum>>(key)];
}

/// Accesses an std::array using an enumeration key.
template <typename T, std::size_t N, typename Enum>
constexpr const auto& at(const std::array<T, N>& array, Enum key)
{
  return array[static_cast<std::underlying_type_t<Enum>>(key)];
}
}

#endif
