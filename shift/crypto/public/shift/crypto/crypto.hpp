#ifndef CRYPTO_CRYPTO_H
#define CRYPTO_CRYPTO_H

#include <cstdint>
#include <array>
#include <vector>
#include <string>
#include <shift/core/boost_disable_warnings.hpp>
#include <boost/endian/arithmetic.hpp>
#include <shift/core/boost_restore_warnings.hpp>
#include "shift/core/types.hpp"

#if defined(SHIFT_CRYPTO_SHARED)
#if defined(SHIFT_CRYPTO_EXPORT)
#define SHIFT_CRYPTO_API SHIFT_EXPORT
#else
#define SHIFT_CRYPTO_API SHIFT_IMPORT
#endif
#else
#define SHIFT_CRYPTO_API
#endif

namespace shift::crypto
{
/// Function to shift an unsigned integral number by n bits to the left.
template <typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T shift_left(T value, const std::size_t n) noexcept
{
  return value << n;
}

/// Function to shift an unsigned integral number by n bits to the left.
template <std::size_t N, typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T shift_left(T value) noexcept
{
  static_assert(N <= sizeof(T) * 8, "Constant out of range.");
  return value << N;
}

/// Function to shift an unsigned integral number by n bits to the right.
template <typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T shift_right(T value, const std::size_t n) noexcept
{
  return value >> n;
}

/// Function to shift an unsigned integral number by n bits to the right.
template <std::size_t N, typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T shift_right(T value) noexcept
{
  static_assert(N <= sizeof(T) * 8, "Constant out of range.");
  return value >> N;
}

/// Function to rotate an unsigned integral number by n bits to the left.
template <typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T rotate_left(T value, const std::size_t n) noexcept
{
  return (value << n) | (value >> (sizeof(T) * 8 - n));
}

/// Function to rotate an unsigned integral number by N bits to the left.
template <std::size_t N, typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T rotate_left(T value) noexcept
{
  static_assert(N <= sizeof(T) * 8, "Constant out of range.");
  return (value << N) | (value >> (sizeof(T) * 8 - N));
}

/// function to rotate an unsigned integral number by n bits to the right.
template <typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T rotate_right(T value, const std::size_t n) noexcept
{
  return (value >> n) | (value << (sizeof(T) * 8 - n));
}

/// function to rotate an unsigned integral number by N bits to the right.
template <std::size_t N, typename T, ENABLE_IF(std::is_unsigned<T>::value)>
inline constexpr T rotate_right(T value) noexcept
{
  static_assert(N <= sizeof(T) * 8, "Constant out of range.");
  return (value >> N) | (value << (sizeof(T) * 8 - N));
}

///
inline std::uint32_t load_little_endian(const std::uint32_t& source)
{
#if BOOST_ENDIAN_LITTLE_BYTE
  return source;
#else
  const auto* memory = reinterpret_cast<const std::uint8_t*>(&source);
  return static_cast<const std::uint32_t>(memory[0]) |
         (static_cast<const std::uint32_t>(memory[1]) << 8) |
         (static_cast<const std::uint32_t>(memory[2]) << 16) |
         (static_cast<const std::uint32_t>(memory[3]) << 24);
#endif
}

///
inline std::uint32_t load_big_endian(const std::uint32_t& source)
{
#if BOOST_ENDIAN_BIG_BYTE
  return source;
#else
  const auto* memory = reinterpret_cast<const std::uint8_t*>(&source);
  return (static_cast<std::uint32_t>(memory[0]) << 24) |
         (static_cast<std::uint32_t>(memory[1]) << 16) |
         (static_cast<std::uint32_t>(memory[2]) << 8) |
         static_cast<std::uint32_t>(memory[3]);
#endif
}

///
inline void store_little_endian(std::uint32_t& destination,
                                const std::uint32_t value)
{
#if BOOST_ENDIAN_LITTLE_BYTE
  destination = value;
#else
  auto* memory = reinterpret_cast<std::uint8_t*>(&destination);
  memory[0] = static_cast<std::uint8_t>(value);
  memory[1] = static_cast<std::uint8_t>(value >> 8);
  memory[2] = static_cast<std::uint8_t>(value >> 16);
  memory[3] = static_cast<std::uint8_t>(value >> 24);
#endif
}

///
inline void store_big_endian(std::uint32_t& destination,
                             const std::uint32_t value)
{
#if BOOST_ENDIAN_BIG_BYTE
  destination = value;
#else
  auto* memory = reinterpret_cast<std::uint8_t*>(&destination);
  memory[0] = static_cast<std::uint8_t>(value >> 24);
  memory[1] = static_cast<std::uint8_t>(value >> 16);
  memory[2] = static_cast<std::uint8_t>(value >> 8);
  memory[3] = static_cast<std::uint8_t>(value);
#endif
}

/// Combine two digests using simple byte-wise xor, storing the result in the
/// first argument.
template <typename T, std::size_t N>
inline std::array<T, N>& combine(std::array<T, N>& lhs,
                                 const std::array<T, N>& rhs) noexcept
{
  for (std::size_t i = 0; i < N; ++i)
    lhs[i] ^= rhs[i];
  return lhs;
}

/// Reduce the size of a digest by XORing several parts into a single one.
/// @remarks
///   When using a good cryptographic hash function, this effectively doesn't
///   give any benefit compared to a much simpler truncation of the digest.
///   However, when using a non-cryptographic hash function, this should be
///   the prefered way of reducing the size of a digest.
template <typename R, typename T, std::size_t N>
inline R reduce(const std::array<T, N>& digest) noexcept
{
  static_assert(N / sizeof(R) * sizeof(R) == N,
                "N must be a multiple of the size of R.");
  R result = 0;
  auto* data = reinterpret_cast<const R*>(digest.data());
  for (std::size_t i = 0; i < N / sizeof(R); ++i, ++data)
    result ^= *data;
  return result;
}
}

#endif
