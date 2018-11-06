#ifndef SHIFT_SERIALIZATION2_ARRAY_HPP
#define SHIFT_SERIALIZATION2_ARRAY_HPP

#include <array>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U, std::size_t N>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::array<U, N>& array)
{
  archive >> begin_array{N};
  for (auto& element : array)
    archive >> element;
  archive >> end_array{};
  return archive;
}

///
template <boost::endian::order Order, typename U, std::size_t N>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::array<U, N>& array)
{
  archive << begin_array{N};
  for (const auto& element : array)
    archive << element;
  archive << end_array{};
  return archive;
}
}

#endif
