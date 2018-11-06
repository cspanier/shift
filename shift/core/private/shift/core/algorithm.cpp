#include "shift/core/algorithm.hpp"
#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace shift::core
{
std::uint32_t hamming_weight(std::uint32_t value)
{
#if defined(__GNUC__)
  return static_cast<std::uint32_t>(__builtin_popcount(value));
#elif defined(_MSC_VER)
  return __popcnt(value);
#else
  // Variable-precision SWAR algorithm.
  value = value - ((value >> 1) & 0x55555555);
  value = (value & 0x33333333) + ((value >> 2) & 0x33333333);
  return (((value + (value >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
#endif
}
}
