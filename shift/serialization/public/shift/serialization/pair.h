#ifndef SHIFT_SERIALIZATION_PAIR_H
#define SHIFT_SERIALIZATION_PAIR_H

#include <utility>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U, typename V>
InputArchive& operator>>(InputArchive& archive, std::pair<U, V>& pair)
{
  archive >> begin_pair{} >> pair.first >> pair.second >> end_pair{};
  return archive;
}

///
template <class OutputArchive, typename U, typename V>
OutputArchive& operator<<(OutputArchive& archive, const std::pair<U, V>& pair)
{
  archive << begin_pair{} << pair.first << pair.second << end_pair{};
  return archive;
}
}

#endif
