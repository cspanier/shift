#ifndef SHIFT_MATH_UTILITY_HPP
#define SHIFT_MATH_UTILITY_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <cmath>
#include <shift/core/types.hpp>

namespace shift::math
{
template <typename T>
std::enable_if_t<std::is_floating_point_v<T>, bool> almost_equal(
  T lhs, T rhs, int units_in_the_last_place = 2)
{
  T diff = std::abs(lhs - rhs);
  // Return true if the difference is not greater than the machine epsilon
  // multiplied by the desired precision in units in the last place, or if the
  // difference is is subnormal.
  return diff <= std::numeric_limits<T>::epsilon() * units_in_the_last_place ||
         diff < std::numeric_limits<T>::min();
}

template <typename T>
static constexpr T pi = static_cast<T>(3.14159265358979323846l);

/// Checks if an unsigned integer value is a power of 2.
template <typename T>
constexpr bool is_power_of_two(T value)
{
  static_assert(std::is_unsigned<T>::value,
                "This function only works for unsigned integer types.");

  if (value == 0)
    return false;
  while ((value & 1) == 0)
    value >>= 1;
  return value == 1;
}

/// Calculates the next power of two for a given unsigned integer value.
/// @param value
///    Any value smaller than 2^(num_bits(T) - 1).
/// @return
///    If value < 2^(num_bits(T) - 1) the function returns pow(2, i) where
///    pow(2, i) > value >= pow(2, i - 1) with 7 > i >= 0.
///    If value >= 2^(num_bits(T) - 1) the next power of 2 cannot be
///    represented in a single variable of type T, resulting in a return value
///    of 0 and the carry flag set.
/// @remarks
///   C++14 constexpr alow a more readable implementation:
///   for (auto i = 1; i < sizeof(T) * 8; i *= 2)
///     value |= value >> i;
///   return value;
template <typename T>
inline constexpr T next_power_of_two(T value)
{
  static_assert(std::is_unsigned<T>::value,
                "This function only works for unsigned integer types.");

  for (unsigned int N = sizeof(T) * 8 / 2; N > 0; N /= 2)
    value |= value >> N;
  return static_cast<T>(value + 1ULL);
}

/// Calculates the smallest multiple of b with
/// next_multiple_of(b, x) = n * b >= x, and n being an integral number.
template <typename T>
inline constexpr T next_multiple_of(T b, T x)
{
  T r = x % b;
  if (r == 0)
    return x;
  else
    return x + b - r;
}

/// Shorter function for std::numeric_limits<T>::quiet_NaN().
template <typename T>
inline constexpr T quiet_nan() noexcept
{
  if (std::numeric_limits<T>::has_quiet_NaN)
    return std::numeric_limits<T>::quiet_NaN();
  else
    return T{};
}

/// Shorter function for std::numeric_limits<T>::signaling_NaN().
/// @remarks
///   This function was created due to a bug in MSVC2015 and below where
///   std::numeric_limits<T>::signaling_NaN() actually returned a QNaN value.
template <typename T>
inline constexpr T signaling_nan() noexcept
{
  if (std::numeric_limits<T>::has_signaling_NaN)
    return std::numeric_limits<T>::signaling_NaN();
  else
    return T{};
}

/// Wrapper function for std::isnan<T>().
template <typename T, ENABLE_IF(std::numeric_limits<T>::has_quiet_NaN)>
inline bool is_nan(T value) noexcept
{
  return std::isnan(value);
}

/// Wrapper function for std::isnan<T>().
template <typename T, ENABLE_IF(!std::numeric_limits<T>::has_quiet_NaN)>
inline constexpr bool is_nan(T /*value*/) noexcept
{
  return false;
}

/// Helper function to convert degree to radiant.
template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
inline constexpr T deg2rad(T angle)
{
  return pi<T> / 180 * angle;
}

/// Helper function to convert radiant to degree.
template <typename T, ENABLE_IF(std::is_floating_point<T>::value)>
inline constexpr T rad2deg(T angle)
{
  return 180 / pi<T> * angle;
}

namespace detail
{
  constexpr std::size_t ct_sqrt_impl(std::size_t x, std::size_t lo,
                                     std::size_t hi, std::size_t mid)
  {
    return lo != hi
             ? ((x / mid < mid) ? ct_sqrt_impl(x, lo, mid - 1, (lo + mid) / 2)
                                : ct_sqrt_impl(x, mid, hi, (mid + hi + 1) / 2))
             : lo;
  }
}

constexpr std::size_t ct_sqrt(std::size_t x)
{
  return detail::ct_sqrt_impl(x, 0, x / 2 + 1, x / 4 + 1);
}

/// A cleaned up namespace used only for literal operators, which you may
/// inline using the 'using namespace' directive.
namespace literals
{
  /// User-defined literal operator used to clearly identify values in degree
  /// and convert them to radians.
  constexpr double operator""_deg(unsigned long long degree)
  {
    return static_cast<double>(degree) * math::pi<double> / 180;
  }

  /// User-defined literal operator used to clearly identify values in degree
  /// and convert them to radians.
  constexpr double operator""_deg(long double degree)
  {
    return static_cast<double>(degree) * math::pi<double> / 180;
  }

  /// User-defined literal operator used to clearly identify values in degree
  /// and convert them to radians.
  constexpr float operator""_fdeg(unsigned long long degree)
  {
    return static_cast<float>(degree) * math::pi<float> / 180;
  }

  /// User-defined literal operator used to clearly identify values in degree
  /// and convert them to radians.
  constexpr float operator""_fdeg(long double degree)
  {
    return static_cast<float>(degree) * math::pi<float> / 180;
  }

  /// User-defined literal operator used to clearly identify values in
  /// milliradians, which is an angular measurement used in military context,
  /// and convert them to radians.
  constexpr double operator""_mil(unsigned long long degree)
  {
    return static_cast<double>(degree) * math::pi<double> / 3200;
  }

  /// User-defined literal operator used to clearly identify values in
  /// milliradians, which is an angular measurement used in military context,
  /// and convert them to radians.
  constexpr double operator""_mil(long double degree)
  {
    return static_cast<double>(degree) * math::pi<double> / 3200;
  }

  /// User-defined literal operator used to clearly identify values in
  /// milliradians, which is an angular measurement used in military context,
  /// and convert them to radians.
  constexpr float operator""_fmil(unsigned long long degree)
  {
    return static_cast<float>(degree) * math::pi<float> / 3200;
  }

  /// User-defined literal operator used to clearly identify values in
  /// milliradians, which is an angular measurement used in military context,
  /// and convert them to radians.
  constexpr float operator""_fmil(long double degree)
  {
    return static_cast<float>(degree) * math::pi<float> / 3200;
  }
}
}

#endif
