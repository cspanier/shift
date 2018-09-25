#ifndef SHIFT_CRYPTO_SHA256_H
#define SHIFT_CRYPTO_SHA256_H

#include <cstdint>
#include <cstring>
#include <array>
#include <string>
#include <string_view>
#include <vector>
#include <variant>
#include "shift/crypto/crypto.h"

namespace shift::crypto
{
/// Implements the sha256 cryptographic hash algorithm.
class sha256
{
public:
  static constexpr std::size_t block_size = 512 / 8;
  static constexpr std::size_t digest_size = 256 / 8;

  using digest_t = std::array<std::uint8_t, digest_size>;

public:
  /// Default constructor.
  sha256(digest_t& digest);

  /// Update operator.
  /// @remarks
  ///   The hash must not be finalized when calling this method.
  sha256& operator()(const char* message, std::size_t length);

  /// Computes the final digest
  /// @return
  ///   Returns a reference to the initial digest object passed to the
  ///   constructor.
  const digest_t& finalize();

  ///
  digest_t& digest()
  {
    return _digest;
  }

  /// Quick way to compute a hash over a string.
  static const digest_t& hash(digest_t& result, std::string_view message);

protected:
  const static std::array<std::uint32_t, 64> k;

protected:
  using digest32 =
    std::array<std::uint32_t, digest_size / sizeof(std::uint32_t)>;

  void transform(const std::uint8_t* block, std::size_t count);

  bool _finalized = false;
  std::size_t _total_length = 0;
  std::size_t _buffer_length = 0;
  std::array<std::uint8_t, 2 * block_size> _buffer;

  digest32 _hash;  /// ToDo: Remove duplicate storage.
  digest_t& _digest;
};

template <typename T, ENABLE_IF(std::is_fundamental_v<T> || std::is_enum_v<T>)>
sha256& operator<<(sha256& context, const T& data)
{
  return context(reinterpret_cast<const char*>(&data), sizeof(T));
}

inline sha256& operator<<(sha256& context, std::string_view string)
{
  return context(string.data(), string.size());
}

template <typename T>
inline sha256& operator<<(sha256& context, const std::vector<T>& vector)
{
  return context(reinterpret_cast<const char*>(vector.data()),
                 vector.size() * sizeof(T));
}

template <typename... Ts>
inline sha256& operator<<(sha256& context, const std::variant<Ts...>& value)
{
  context << static_cast<std::uint32_t>(value.index());
  std::visit([&](const auto& actual_value) { context << actual_value; }, value);
  return context;
}
}

#if __has_include(<shift/math/vector.h>)
#include <shift/math/vector.h>

namespace shift::crypto
{
template <std::size_t Rows, typename T>
inline sha256& operator<<(sha256& context, const math::vector<Rows, T>& vector)
{
  for (auto row = 0u; row < Rows; ++row)
    context << vector(row);
  return context;
}
}
#endif

#if __has_include(<shift/math/matrix.h>)
#include <shift/math/matrix.h>

namespace shift::crypto
{
template <std::size_t Rows, std::size_t Columns, typename T>
inline sha256& operator<<(sha256& context,
                          const math::matrix<Rows, Columns, T>& matrix)
{
  for (auto column = 0u; column < Columns; ++column)
    context << matrix.column_vector(column);
  return context;
}
}
#endif

#endif
