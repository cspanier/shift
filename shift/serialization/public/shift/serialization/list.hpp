#ifndef SHIFT_SERIALIZATION_LIST_HPP
#define SHIFT_SERIALIZATION_LIST_HPP

#include <list>
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
///
template <class InputArchive, typename U>
InputArchive& operator>>(InputArchive& archive, std::list<U>& list)
{
  begin_list begin;
  archive >> begin;
  list.resize(begin.length);
  for (auto& element : list)
    archive >> element;
  archive.end_list();
  return archive;
}

///
template <class OutputArchive, typename U>
OutputArchive& operator<<(OutputArchive& archive, const std::list<U>& list)
{
  archive << begin_list{list.size()};
  for (const auto& element : list)
    archive << element;
  archive << end_list{};
  return archive;
}
}

#endif
