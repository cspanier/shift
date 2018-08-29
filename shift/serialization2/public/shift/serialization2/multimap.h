#ifndef SHIFT_SERIALIZATION2_MULTIMAP_H
#define SHIFT_SERIALIZATION2_MULTIMAP_H

#include <map>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U, typename V>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::multimap<U, V>& map)
{
  map.clear();
  begin_map begin;
  archive >> begin;
  for (auto i = 0u; i != begin.length; ++i)
  {
    U key;
    V value;
    archive >> key >> value;
    map.insert(std::make_pair(std::move(key), std::move(value)));
  }
  archive >> end_map{};
  return archive;
}

///
template <boost::endian::order Order, typename U, typename V>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::multimap<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& element : map)
    archive << element.first << element.second;
  archive << end_map{};
  return archive;
}
}

#endif
