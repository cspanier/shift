#ifndef SHIFT_SERIALIZATION_ARRAY_H
#define SHIFT_SERIALIZATION_ARRAY_H

#include <array>
#include "shift/serialization/types.h"
#include "shift/serialization/archive.h"

namespace shift::serialization
{
///
template <class InputArchive, typename U, std::size_t N>
InputArchive& operator>>(InputArchive& archive, std::array<U, N>& array)
{
  archive >> begin_array{N};
  for (auto& element : array)
    archive >> element;
  archive >> end_array{};
  return archive;
}

///
template <class OutputArchive, typename U, std::size_t N>
OutputArchive& operator<<(OutputArchive& archive, const std::array<U, N>& array)
{
  archive << begin_array{N};
  for (const auto& element : array)
    archive << element;
  archive << end_array{};
  return archive;
}
}

#endif
