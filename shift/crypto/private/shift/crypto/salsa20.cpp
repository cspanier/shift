#include "shift/crypto/salsa20.h"
#include "shift/crypto/crypto.h"
#include <cstring>
#include <memory>

namespace shift::crypto
{
salsa20::salsa20() = default;

salsa20::~salsa20()
{
  std::memset(_key.data(), 0, _key.size());
  std::memset(_output.data(), 0, _output.size());
}

salsa20::key_t& salsa20::key()
{
  return _key;
}

const salsa20::block_t& salsa20::generate(const salsa20::position_t& position,
                                          const salsa20::nonce_t& nonce)
{
#if !defined(BOOST_ENDIAN_LITTLE_BYTE)
#error This implementation assumes a little endian platform.
#endif
  using position32_t = std::array<std::uint32_t, 4>;
  using key32_t = std::array<std::uint32_t, 8>;
  using nonce32_t = std::array<std::uint32_t, 4>;
  using output32_t = std::array<std::uint32_t, 16>;

  // Directly convert 8-bit arrays to 32-bit arrays as we're assuming to be
  // little endian.
  const auto& position32 = reinterpret_cast<const position32_t&>(position);
  const auto& key32 = reinterpret_cast<const key32_t&>(_key);
  const auto& nonce32 = reinterpret_cast<const nonce32_t&>(nonce);
  auto& output32 = reinterpret_cast<output32_t&>(_output);

  output32_t x = {{
    // clang-format off
    load_little_endian(nonce32[0]),
    load_little_endian(key32[0]),
    load_little_endian(key32[1]),
    load_little_endian(key32[2]),
    load_little_endian(key32[3]),
    load_little_endian(nonce32[1]),
    load_little_endian(position32[0]),
    load_little_endian(position32[1]),
    load_little_endian(position32[2]),
    load_little_endian(position32[3]),
    load_little_endian(nonce32[2]),
    load_little_endian(key32[4]),
    load_little_endian(key32[5]),
    load_little_endian(key32[6]),
    load_little_endian(key32[7]),
    load_little_endian(nonce32[3])
    // clang-format on
  }};
  output32_t j = x;

  for (int i = rounds; i > 0; i -= 2)
  {
    x[0x04] ^= rotate_left<7>(x[0x00] + x[0x0C]);
    x[0x08] ^= rotate_left<9>(x[0x04] + x[0x00]);
    x[0x0C] ^= rotate_left<13>(x[0x08] + x[0x04]);
    x[0x00] ^= rotate_left<18>(x[0x0C] + x[0x08]);

    x[0x09] ^= rotate_left<7>(x[0x05] + x[0x01]);
    x[0x0D] ^= rotate_left<9>(x[0x09] + x[0x05]);
    x[0x01] ^= rotate_left<13>(x[0x0D] + x[0x09]);
    x[0x05] ^= rotate_left<18>(x[0x01] + x[0x0D]);

    x[0x0E] ^= rotate_left<7>(x[0x0A] + x[0x06]);
    x[0x02] ^= rotate_left<9>(x[0x0E] + x[0x0A]);
    x[0x06] ^= rotate_left<13>(x[0x02] + x[0x0E]);
    x[0x0A] ^= rotate_left<18>(x[0x06] + x[0x02]);

    x[0x03] ^= rotate_left<7>(x[0x0F] + x[0x0B]);
    x[0x07] ^= rotate_left<9>(x[0x03] + x[0x0F]);
    x[0x0B] ^= rotate_left<13>(x[0x07] + x[0x03]);
    x[0x0F] ^= rotate_left<18>(x[0x0B] + x[0x07]);

    x[0x01] ^= rotate_left<7>(x[0x00] + x[0x03]);
    x[0x02] ^= rotate_left<9>(x[0x01] + x[0x00]);
    x[0x03] ^= rotate_left<13>(x[0x02] + x[0x01]);
    x[0x00] ^= rotate_left<18>(x[0x03] + x[0x02]);

    x[0x06] ^= rotate_left<7>(x[0x05] + x[0x04]);
    x[0x07] ^= rotate_left<9>(x[0x06] + x[0x05]);
    x[0x04] ^= rotate_left<13>(x[0x07] + x[0x06]);
    x[0x05] ^= rotate_left<18>(x[0x04] + x[0x07]);

    x[0x0B] ^= rotate_left<7>(x[0x0A] + x[0x09]);
    x[0x08] ^= rotate_left<9>(x[0x0B] + x[0x0A]);
    x[0x09] ^= rotate_left<13>(x[0x08] + x[0x0B]);
    x[0x0A] ^= rotate_left<18>(x[0x09] + x[0x08]);

    x[0x0C] ^= rotate_left<7>(x[0x0F] + x[0x0E]);
    x[0x0D] ^= rotate_left<9>(x[0x0C] + x[0x0F]);
    x[0x0E] ^= rotate_left<13>(x[0x0D] + x[0x0C]);
    x[0x0F] ^= rotate_left<18>(x[0x0E] + x[0x0D]);
  }

  store_little_endian(output32[0x0], x[0x00] + j[0x00]);
  store_little_endian(output32[0x1], x[0x01] + j[0x01]);
  store_little_endian(output32[0x2], x[0x02] + j[0x02]);
  store_little_endian(output32[0x3], x[0x03] + j[0x03]);
  store_little_endian(output32[0x4], x[0x04] + j[0x04]);
  store_little_endian(output32[0x5], x[0x05] + j[0x05]);
  store_little_endian(output32[0x6], x[0x06] + j[0x06]);
  store_little_endian(output32[0x7], x[0x07] + j[0x07]);
  store_little_endian(output32[0x8], x[0x08] + j[0x08]);
  store_little_endian(output32[0x9], x[0x09] + j[0x09]);
  store_little_endian(output32[0xA], x[0x0A] + j[0x0A]);
  store_little_endian(output32[0xB], x[0x0B] + j[0x0B]);
  store_little_endian(output32[0xC], x[0x0C] + j[0x0C]);
  store_little_endian(output32[0xD], x[0x0D] + j[0x0D]);
  store_little_endian(output32[0xE], x[0x0E] + j[0x0E]);
  store_little_endian(output32[0xF], x[0x0F] + j[0x0F]);

  return _output;
}

void salsa20::transform(const nonce_t& nonce, std::vector<char>& data)
{
  if (data.empty())
    return;

  std::array<std::uint64_t, position_size / sizeof(std::uint64_t)> position{};
  for (std::uint64_t dataOffset = 0; dataOffset < data.size();
       dataOffset += block_size)
  {
    position[position_size / sizeof(std::uint64_t) - 1] =
      dataOffset / sizeof(std::uint64_t);
    generate(*reinterpret_cast<const position_t*>(&position), nonce);

    std::size_t blockOffset = 0;
    while (blockOffset < block_size &&
           dataOffset + sizeof(std::uint64_t) <= data.size())
    {
      *reinterpret_cast<std::uint64_t*>(&data[dataOffset]) ^=
        *reinterpret_cast<std::uint64_t*>(&_output[blockOffset]);
      dataOffset += sizeof(std::uint64_t);
      blockOffset += sizeof(std::uint64_t);
    }
    if (blockOffset < block_size &&
        dataOffset + sizeof(std::uint32_t) <= data.size())
    {
      *reinterpret_cast<std::uint32_t*>(&data[dataOffset]) ^=
        *reinterpret_cast<std::uint32_t*>(&_output[blockOffset]);
      dataOffset += sizeof(std::uint32_t);
      blockOffset += sizeof(std::uint32_t);
    }
    if (blockOffset < block_size &&
        dataOffset + sizeof(std::uint16_t) <= data.size())
    {
      *reinterpret_cast<std::uint16_t*>(&data[dataOffset]) ^=
        *reinterpret_cast<std::uint16_t*>(&_output[blockOffset]);
      dataOffset += sizeof(std::uint16_t);
      blockOffset += sizeof(std::uint16_t);
    }
    if (blockOffset < block_size &&
        dataOffset + sizeof(std::uint8_t) <= data.size())
    {
      *reinterpret_cast<std::uint8_t*>(&data[dataOffset]) ^=
        *reinterpret_cast<std::uint8_t*>(&_output[blockOffset]);
      dataOffset += sizeof(std::uint8_t);
      // blockOffset += sizeof(std::uint8_t);
    }
  }
}
}
