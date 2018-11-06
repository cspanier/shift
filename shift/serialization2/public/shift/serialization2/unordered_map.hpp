#ifndef SHIFT_SERIALIZATION2_UNORDERED_MAP_HPP
#define SHIFT_SERIALIZATION2_UNORDERED_MAP_HPP

#include <unordered_map>
#include "shift/serialization2/types.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U, typename V>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::unordered_map<U, V>& map)
{
  map.clear();
  begin_map begin;
  archive >> begin;
  for (std::size_t i = 0; i != begin.length; ++i)
  {
    U key;
    V value;
    archive >> key >> value;
    if (!map.insert(std::make_pair(std::move(key), std::move(value))).second)
    {
      BOOST_THROW_EXCEPTION(serialization_error() << serialization_error_info(
                              "Detected duplicate key value in unordered_map "
                              "deserialization routine."));
    }
  }
  archive >> end_map{};
  return archive;
}

///
template <boost::endian::order Order, typename U, typename V>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::unordered_map<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& element : map)
    archive << element.first << element.second;
  archive << end_map{};
  return archive;
}
}

#endif
