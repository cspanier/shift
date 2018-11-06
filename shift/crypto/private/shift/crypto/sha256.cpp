#include "shift/crypto/sha256.hpp"

namespace shift::crypto
{
template <typename T>
inline constexpr T sha2_ch(const T x, const T y, const T z) noexcept
{
  return (x & y) ^ (~x & z);
}

template <typename T>
inline constexpr T sha2_maj(const T x, const T y, const T z) noexcept
{
  return (x & y) ^ (x & z) ^ (y & z);
}

template <typename T>
inline constexpr T sha256_f1(const T x) noexcept
{
  return rotate_right<2>(x) ^ rotate_right<13>(x) ^ rotate_right<22>(x);
}

template <typename T>
inline constexpr T sha256_f2(const T x) noexcept
{
  return rotate_right<6>(x) ^ rotate_right<11>(x) ^ rotate_right<25>(x);
}

template <typename T>
inline constexpr T sha256_f3(const T x) noexcept
{
  return rotate_right<7>(x) ^ rotate_right<18>(x) ^ shift_right<3>(x);
}

template <typename T>
inline constexpr T sha256_f4(const T x) noexcept
{
  return rotate_right<17>(x) ^ rotate_right<19>(x) ^ shift_right<10>(x);
}

inline void sha2_unpack32(std::uint32_t x, std::uint8_t* str)
{
  str[3] = static_cast<std::uint8_t>(x & 0xFF);
  str[2] = static_cast<std::uint8_t>((x >> 8) & 0xFF);
  str[1] = static_cast<std::uint8_t>((x >> 16) & 0xFF);
  str[0] = static_cast<std::uint8_t>((x >> 24) & 0xFF);
}

inline std::uint32_t sha2_pack32(const std::uint8_t* str)
{
  return (static_cast<std::uint32_t>(str[3])) |
         (static_cast<std::uint32_t>(str[2]) << 8) |
         (static_cast<std::uint32_t>(str[1]) << 16) |
         (static_cast<std::uint32_t>(str[0]) << 24);
}

const std::array<std::uint32_t, 64> sha256::k = {
  {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1,
   0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
   0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174, 0xe49b69c1, 0xefbe4786,
   0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
   0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
   0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85, 0xa2bfe8a1, 0xa81a664b,
   0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a,
   0x5b9cca4f, 0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
   0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2}};

sha256::sha256(sha256::digest_t& digest) : _buffer(), _hash(), _digest(digest)
{
  _hash[0] = 0x6a09e667;
  _hash[1] = 0xbb67ae85;
  _hash[2] = 0x3c6ef372;
  _hash[3] = 0xa54ff53a;
  _hash[4] = 0x510e527f;
  _hash[5] = 0x9b05688c;
  _hash[6] = 0x1f83d9ab;
  _hash[7] = 0x5be0cd19;
}

sha256& sha256::operator()(const char* message, std::size_t length)
{
  BOOST_ASSERT(!_finalized);
  const auto* umessage = reinterpret_cast<const std::uint8_t*>(message);

  auto consume_length = std::min(length, block_size - _buffer_length);
  if (_buffer_length > 0 || consume_length < block_size)
  {
    // Append data to buffer until it is full.
    memcpy(&_buffer[_buffer_length], umessage, consume_length);
    if (_buffer_length + length < block_size)
    {
      _buffer_length += length;
      return *this;
    }
    umessage += consume_length;
    length -= consume_length;
    // Hash first block.
    transform(_buffer.data(), 1);
    _total_length += block_size;
  }
  // Hash all full blocks.
  auto block_count = length / block_size;
  if (block_count > 0)
  {
    consume_length = block_count * block_size;
    transform(umessage, block_count);
    umessage += consume_length;
    length -= consume_length;
    _total_length += consume_length;
  }

  // Copy remaining bytes to buffer.
  _buffer_length = length;
  memcpy(_buffer.data(), umessage, _buffer_length);

  return *this;
}

const sha256::digest_t& sha256::finalize()
{
  BOOST_ASSERT(!_finalized);
  if (!_finalized)
  {
    auto block_count =
      ((block_size - 9) < (_buffer_length % block_size)) ? 2u : 1u;
    auto length_in_bits =
      static_cast<std::uint32_t>((_total_length + _buffer_length) * 8);
    auto finalLength = block_count * block_size;
    memset(&_buffer[_buffer_length], 0, finalLength - _buffer_length);
    _buffer[_buffer_length] = 0x80;
    sha2_unpack32(length_in_bits, &_buffer[finalLength - 4]);
    transform(_buffer.data(), block_count);
    for (std::size_t i = 0; i < 8; i++)
      sha2_unpack32(_hash[i], &_digest[i << 2]);
    _finalized = true;
  }
  return _digest;
}

const sha256::digest_t& sha256::hash(sha256::digest_t& result,
                                     std::string_view message)
{
  crypto::sha256 context{result};
  context << message;
  return context.finalize();
}

void sha256::transform(const std::uint8_t* block, std::size_t count)
{
  std::uint32_t w[64];
  std::uint32_t wv[8];
  std::uint32_t t1, t2;
  for (std::size_t i = 0; i < count; ++i, block += block_size)
  {
    for (auto j = 0u; j < 16u; j++)
      w[j] = sha2_pack32(&block[j << 2]);
    for (auto j = 16u; j < 64u; j++)
    {
      w[j] = sha256_f4(w[j - 2]) + w[j - 7] + sha256_f3(w[j - 15]) + w[j - 16];
    }
    for (auto j = 0u; j < 8u; j++)
      wv[j] = _hash[j];
    for (auto j = 0u; j < 64u; j++)
    {
      t1 =
        wv[7] + sha256_f2(wv[4]) + sha2_ch(wv[4], wv[5], wv[6]) + k[j] + w[j];
      t2 = sha256_f1(wv[0]) + sha2_maj(wv[0], wv[1], wv[2]);
      wv[7] = wv[6];
      wv[6] = wv[5];
      wv[5] = wv[4];
      wv[4] = wv[3] + t1;
      wv[3] = wv[2];
      wv[2] = wv[1];
      wv[1] = wv[0];
      wv[0] = t1 + t2;
    }
    for (auto j = 0u; j < 8u; j++)
      _hash[j] += wv[j];
  }
}
}
