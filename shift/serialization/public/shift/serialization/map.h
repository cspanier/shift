#ifndef SHIFT_SERIALIZATION_MAP_H
#define SHIFT_SERIALIZATION_MAP_H

#include <map>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U, typename V>
InputArchive& operator>>(InputArchive& archive, std::map<U, V>& map)
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
        SerializationError() << SerializationErrorInfo(
          "Detected duplicate key value in map deserialization routine."));
    }
  }
  archive >> end_map{};
  return archive;
}

///
template <class OutputArchive, typename U, typename V>
OutputArchive& operator<<(OutputArchive& archive, const std::map<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& key_value : map)
    archive << key_value;
  archive << end_map{};
  return archive;
}
}

#endif
