#ifndef SHIFT_SERIALIZATION_MULTIMAP_HPP
#define SHIFT_SERIALIZATION_MULTIMAP_HPP

#include <map>
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
///
template <class InputArchive, typename U, typename V>
InputArchive& operator>>(InputArchive& archive, std::multimap<U, V>& map)
{
  map.clear();
  begin_map begin;
  archive >> begin;
  for (std::size_t i = 0; i != begin.length; ++i)
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
template <class OutputArchive, typename U, typename V>
OutputArchive& operator<<(OutputArchive& archive,
                          const std::multimap<U, V>& map)
{
  archive << begin_map{map.size()};
  for (const auto& element : map)
    archive << element.first << element.second;
  archive << end_map{};
  return archive;
}
}

#endif
