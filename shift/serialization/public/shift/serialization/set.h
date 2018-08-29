#ifndef SHIFT_SERIALIZATION_SET_H
#define SHIFT_SERIALIZATION_SET_H

#include <set>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U>
InputArchive& operator>>(InputArchive& archive, std::set<U>& set)
{
  set.clear();
  begin_set begin;
  archive >> begin;
  for (std::size_t i = 0; i != begin.length; ++i)
  {
    U element;
    archive >> element;
    if (!set.insert(element).second)
    {
      BOOST_THROW_EXCEPTION(
        SerializationError() << SerializationErrorInfo(
          "Detected duplicate value in set deserialization routine."));
    }
  }
  archive >> end_set{};
  return archive;
}

///
template <class OutputArchive, typename U>
OutputArchive& operator<<(OutputArchive& archive, const std::set<U>& set)
{
  archive << begin_set{set.size()};
  for (const auto& element : set)
    archive << element;
  archive << end_set{};
  return archive;
}
}

#endif
