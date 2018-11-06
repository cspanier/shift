#ifndef SHIFT_SERIALIZATION_VECTOR_HPP
#define SHIFT_SERIALIZATION_VECTOR_HPP

#include <vector>
#include "shift/serialization/types.hpp"
#include "shift/serialization/archive.hpp"

namespace shift::serialization
{
///
template <class InputArchive, typename U>
InputArchive& operator>>(InputArchive& archive, std::vector<U>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  for (auto& element : vector)
    archive >> element;
  archive >> end_vector{};
  return archive;
}

///
template <class OutputArchive, typename U>
OutputArchive& operator<<(OutputArchive& archive, const std::vector<U>& vector)
{
  archive << begin_vector{vector.size()};
  for (const auto& element : vector)
    archive << element;
  archive << end_vector{};
  return archive;
}

/// Special case for std::vector<bool>.
template <class InputArchive>
InputArchive& operator>>(InputArchive& archive, std::vector<bool>& vector)
{
  begin_vector begin;
  archive >> begin;
  vector.resize(begin.length);
  for (std::size_t i = 0; i < vector.size(); ++i)
  {
    bool element;
    archive >> element;
    vector[i] = element;
  }
  archive >> end_vector{};
  return archive;
}

///
template <class OutputArchive>
OutputArchive& operator<<(OutputArchive& archive,
                          const std::vector<bool>& vector)
{
  archive << begin_vector{vector.size()};
  for (const auto element : vector)
    archive << element;
  archive << end_vector{};
  return archive;
}
}

#endif
