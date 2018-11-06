#ifndef SHIFT_MATH_SERIALIZATION_HPP
#define SHIFT_MATH_SERIALIZATION_HPP

#include <cstdint>
#include "shift/math/vector.hpp"
#include "shift/math/matrix.hpp"
#include "shift/math/aabb.hpp"

namespace shift::serialization
{
///
template <class InputArchive, std::size_t N, typename T>
InputArchive& operator>>(InputArchive& archive, math::vector<N, T>& vector)
{
  for (std::size_t i = 0; i < N; ++i)
    archive >> vector(i);
  return archive;
}

///
template <class OutputArchive, std::size_t N, typename T>
OutputArchive& operator<<(OutputArchive& archive,
                          const math::vector<N, T>& vector)
{
  for (std::size_t i = 0; i < N; ++i)
    archive << vector(i);
  return archive;
}

///
template <class InputArchive, std::size_t Rows, std::size_t Columns, typename T>
InputArchive& operator>>(InputArchive& archive,
                         math::matrix<Rows, Columns, T>& matrix)
{
  for (std::size_t i = 0; i < Columns; ++i)
    archive >> matrix.column_vector(i);
  return archive;
}

///
template <class OutputArchive, std::size_t Rows, std::size_t Columns,
          typename T>
OutputArchive& operator<<(OutputArchive& archive,
                          const math::matrix<Rows, Columns, T>& matrix)
{
  for (std::size_t i = 0; i < Columns; ++i)
    archive << matrix.column_vector(i);
  return archive;
}

///
template <class InputArchive, std::size_t N, typename T>
InputArchive& operator>>(InputArchive& archive, math::aabb<N, T>& box)
{
  archive >> box.center >> box.extent;
  return archive;
}

///
template <class OutputArchive, std::size_t N, typename T>
OutputArchive& operator<<(OutputArchive& archive, const math::aabb<N, T>& box)
{
  archive << box.center << box.extent;
  return archive;
}
}

#endif
