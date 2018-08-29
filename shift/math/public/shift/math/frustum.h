#ifndef SHIFT_MATH_FRUSTUM_H
#define SHIFT_MATH_FRUSTUM_H

#include <array>
#include "shift/math/matrix.h"
#include "shift/math/plane.h"

namespace shift::math
{
///
template <std::size_t N, typename T>
class frustum
{
public:
  /// Default constructor.
  constexpr frustum() = default;

  /// Constructor leaving all members uninitialized.
  constexpr explicit frustum(do_not_initialize) noexcept;

  ///
  constexpr plane<N, T>& operator()(std::size_t index);

  ///
  constexpr const plane<N, T>& operator()(std::size_t index) const;

public:
  std::array<plane<N, T>, 2 * N> planes;
};

template <std::size_t N, typename T>
constexpr frustum<N, T>::frustum(do_not_initialize) noexcept
{
  /// ToDo: Find way to initialize array with 2 * N plane{do_not_initialize{}}s.
}

template <std::size_t N, typename T>
constexpr plane<N, T>& frustum<N, T>::operator()(std::size_t index)
{
  return planes[index];
}

template <std::size_t N, typename T>
constexpr const plane<N, T>& frustum<N, T>::operator()(std::size_t index) const
{
  return planes[index];
}

template <std::size_t N, typename T>
constexpr inline frustum<N, T> operator*(const matrix<N + 1, N + 1, T>& lhs,
                                         const frustum<N, T>& rhs)
{
  frustum<N, T> result;
  for (std::size_t i = 0; i < 2 * N; ++i)
    result(i) = lhs * rhs(i);
  return result;
}

///
template <typename T>
constexpr frustum<3, T> make_frustum_from_projection(
  const matrix<4, 4, T>& projection, bool normalize)
{
  frustum<3, T> result;

  // Left clipping plane
  result.planes[0].normal(make_vector_from(
    projection(3, 0) + projection(0, 0), projection(3, 1) + projection(0, 1),
    projection(3, 2) + projection(0, 2)));
  result.planes[0].distance(projection(3, 3) + projection(0, 3));
  // Right clipping plane
  result.planes[1].normal(make_vector_from(
    projection(3, 0) - projection(0, 0), projection(3, 1) - projection(0, 1),
    projection(3, 2) - projection(0, 2)));
  result.planes[1].distance(projection(3, 3) - projection(0, 3));
  // Top clipping plane
  result.planes[2].normal(make_vector_from(
    projection(3, 0) - projection(1, 0), projection(3, 1) - projection(1, 1),
    projection(3, 2) - projection(1, 2)));
  result.planes[2].distance(projection(3, 3) - projection(1, 3));
  // Bottom clipping plane
  result.planes[3].normal(make_vector_from(
    projection(3, 0) + projection(1, 0), projection(3, 1) + projection(1, 1),
    projection(3, 2) + projection(1, 2)));
  result.planes[3].distance(projection(3, 3) + projection(1, 3));
  // Near clipping plane
  result.planes[4].normal(make_vector_from(
    projection(3, 0) + projection(2, 0), projection(3, 1) + projection(2, 1),
    projection(3, 2) + projection(2, 2)));
  result.planes[4].distance(projection(3, 3) + projection(2, 3));
  // Far clipping plane
  result.planes[5].normal(make_vector_from(
    projection(3, 0) - projection(2, 0), projection(3, 1) - projection(2, 1),
    projection(3, 2) - projection(2, 2)));
  result.planes[5].distance(projection(3, 3) - projection(2, 3));

  if (normalize)
  {
    result.planes[0].normalize();
    result.planes[1].normalize();
    result.planes[2].normalize();
    result.planes[3].normalize();
    result.planes[4].normalize();
    result.planes[5].normalize();
  }

  return result;
}
}

#endif
