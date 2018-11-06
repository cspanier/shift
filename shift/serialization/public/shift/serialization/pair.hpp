#ifndef SHIFT_SERIALIZATION_PAIR_HPP
#define SHIFT_SERIALIZATION_PAIR_HPP

#include <utility>
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

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
