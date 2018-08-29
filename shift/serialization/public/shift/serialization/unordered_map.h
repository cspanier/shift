#ifndef SHIFT_SERIALIZATION_UNORDEREDMAP_H
#define SHIFT_SERIALIZATION_UNORDEREDMAP_H

#include <unordered_map>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U, typename V>
InputArchive& operator>>(InputArchive& archive, std::unordered_map<U, V>& map)
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
      BOOST_THROW_EXCEPTION(SerializationError() << SerializationErrorInfo(
                              "Detected duplicate key value in unordered_map "
                              "deserialization routine."));
    }
  }
  archive >> end_map{};
  return archive;
}

///
template <class OutputArchive, typename U, typename V>
OutputArchive& operator<<(OutputArchive& archive,
                          const std::unordered_map<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& element : map)
    archive << element.first << element.second;
  archive << end_map{};
  return archive;
}
}

#endif
