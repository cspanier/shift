#ifndef SHIFT_CRYPTO_SALSA20_H
#define SHIFT_CRYPTO_SALSA20_H

#include <cstdint>
#include <array>
#include <vector>
#include <shift/platform/assert.h>
#include "shift/crypto/crypto.h"

namespace shift::crypto
{
struct salsa20
{
  static constexpr std::size_t position_size = 16;
  static constexpr std::size_t key_size = 32;
  static constexpr std::size_t nonce_size = 16;
  static constexpr std::size_t block_size = 64;
  static constexpr std::size_t rounds = 20;

  using position_t = std::array<std::uint8_t, position_size>;
  using key_t = std::array<std::uint8_t, key_size>;
  using nonce_t = std::array<std::uint8_t, nonce_size>;
  using block_t = std::array<std::uint8_t, block_size>;

public:
  /// Default constructor.
  salsa20();

  /// Destructor cleaning up secret data.
  ~salsa20();

  /// Returns a reference to the memory storing key information.
  key_t& key();

  /// Generate a block of pseudo random data based on input, key and nonce.
  /// @param position
  ///   Usually you want to use the stream position here to allow random
  ///   access to your encrypted data.
  /// @param nonce
  ///   An additional key which may be set to e.g. additional stream position
  ///   bits that didn't fit into position, the network packet number, or some
  ///   random IV which you could change periodically.
  /// @return
  ///   A block of pseudo random data which is usually XORed with either the
  ///   plain text or the encrypted text to allow symmetric encryption and
  ///   decryption.
  const block_t& generate(const position_t& position, const nonce_t& nonce);

  ///
  void transform(const nonce_t& nonce, std::vector<char>& data);

private:
  key_t _key{};
  block_t _output{};
};
}

#endif
