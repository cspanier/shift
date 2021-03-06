#ifndef SHIFT_MATH_SERIALIZATION2_HPP
#define SHIFT_MATH_SERIALIZATION2_HPP

#include <cstdint>
#include <boost/endian/conversion.hpp>
#include <shift/serialization2/types.hpp>
#include "shift/math/vector.hpp"
#include "shift/math/matrix.hpp"
#include "shift/math/aabb.hpp"

namespace shift::serialization2
{
///
template <boost::endian::order Order, std::size_t N, typename T>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         math::vector<N, T>& vector)
{
  for (std::size_t i = 0; i < N; ++i)
    archive >> vector(i);
  return archive;
}

///
template <boost::endian::order Order, std::size_t N, typename T>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const math::vector<N, T>& vector)
{
  for (std::size_t i = 0; i < N; ++i)
    archive << vector(i);
  return archive;
}

///
template <boost::endian::order Order, std::size_t Rows, std::size_t Columns,
          typename T>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         math::matrix<Rows, Columns, T>& matrix)
{
  for (std::size_t i = 0; i < Columns; ++i)
    archive >> matrix.column_vector(i);
  return archive;
}

///
template <boost::endian::order Order, std::size_t Rows, std::size_t Columns,
          typename T>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive,
  const math::matrix<Rows, Columns, T>& matrix)
{
  for (std::size_t i = 0; i < Columns; ++i)
    archive << matrix.column_vector(i);
  return archive;
}

///
template <boost::endian::order Order, std::size_t N, typename T>
compact_input_archive<Order>& operator>>(compact_input_archive<Order>& archive,
                                         math::aabb<N, T>& box)
{
  archive >> box.center >> box.extent;
  return archive;
}

///
template <boost::endian::order Order, std::size_t N, typename T>
compact_output_archive<Order>& operator<<(
  compact_output_archive<Order>& archive, const math::aabb<N, T>& box)
{
  archive << box.center << box.extent;
  return archive;
}
}

#endif
