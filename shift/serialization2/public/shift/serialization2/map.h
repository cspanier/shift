#ifndef SHIFT_SERIALIZATION2_MAP_H
#define SHIFT_SERIALIZATION2_MAP_H

#include <map>
#include "shift/serialization2/types.h"

namespace shift::serialization2
{
///
template <boost::endian::order Order, typename U, typename V>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         std::map<U, V>& map)
{
  map.clear();
  begin_map begin;
  archive >> begin;
  for (std::size_t i = 0; i != begin.length; ++i)
  {
    std::pair<U, V> key_value;
    archive >> key_value;
    if (!map.insert(std::move(key_value)).second)
    {
      BOOST_THROW_EXCEPTION(
        serialization_error() << serialization_error_info(
          "Detected duplicate key value in map deserialization routine."));
    }
  }
  archive >> end_map{};
  return archive;
}

///
template <boost::endian::order Order, typename U, typename V>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const std::map<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& key_value : map)
    archive << key_value;
  archive << end_map{};
  return archive;
}
}

#endif
