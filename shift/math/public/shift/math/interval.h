#ifndef SHIFT_MATH_INTERVAL_H
#define SHIFT_MATH_INTERVAL_H

#include <shift/core/algorithm.h>
#include "shift/math/vector.h"

namespace shift::math
{
/// Template class for intervals.
template <std::size_t Dimensions, typename T>
class interval final
{
public:
  /// Default constructor.
  constexpr interval() = default;

  /// Copy constructor.
  constexpr interval(const interval& /*other*/) noexcept = default;

  /// Move constructor.
  constexpr interval(interval&& /*other*/) noexcept = default;

  /// Constructor taking two vectors.
  /// @remarks
  ///   You need to make sure that the interval has no negative extent in any
  ///   dimension. If you want to pass arbitrary points use interval::create
  ///   instead.
  constexpr interval(const vector<Dimensions, T> min,
                     const vector<Dimensions, T> max);

  /// Constructor for coordinates.
  constexpr interval(const T x, const T y, const T width, const T height);

  /// Copy assignment operator.
  interval& operator=(const interval& /*other*/) noexcept = default;

  /// Move assignment operator.
  constexpr interval& operator=(interval&& /*other*/) noexcept = default;

  /// Equality operator.
  bool operator==(const interval& other) const;

  /// Inequality operator.
  bool operator!=(const interval& other) const;

  /// Returns the extent of this interval in the specified dimension.
  T extent(std::size_t dimension) const;

  /// Returns the center of this interval.
  vector<Dimensions, T> center() const;

public:
  vector<Dimensions, T> min;
  vector<Dimensions, T> max;
};

template <std::size_t Dimensions, typename T>
constexpr interval<Dimensions, T>::interval(const vector<Dimensions, T> min,
                                            const vector<Dimensions, T> max)
: min((min.x <= max.x && min.y <= max.y) ? min
                                         : make_nan_vector<Dimensions, T>()),
  max((min.x <= max.x && min.y <= max.y) ? max
                                         : make_nan_vector<Dimensions, T>())
{
}

template <std::size_t Dimensions, typename T>
constexpr interval<Dimensions, T>::interval(const T x, const T y, const T width,
                                            const T height)
{
  BOOST_ASSERT(width >= 0 && height >= 0);
  min.x() = x;
  min.y() = y;
  max.x() = x + width;
  max.y() = y + height;
}

template <std::size_t Dimensions, typename T>
bool interval<Dimensions, T>::operator==(
  const interval<Dimensions, T>& other) const
{
  return min == other.min && max == other.max;
}

template <std::size_t Dimensions, typename T>
bool interval<Dimensions, T>::operator!=(
  const interval<Dimensions, T>& other) const
{
  return !(*this == other);
}

template <std::size_t Dimensions, typename T>
T interval<Dimensions, T>::extent(std::size_t dimension) const
{
  return max(dimension) - min(dimension);
}

template <std::size_t Dimensions, typename T>
vector<Dimensions, T> interval<Dimensions, T>::center() const
{
  return (max + min) / 2;
}

namespace detail
{
  /// This simple helper is needed to remove template parameter T from the
  /// public make_interval_* functions.
  template <std::size_t Dimensions, typename T>
  constexpr interval<Dimensions, T> make_interval_from(
    const vector<Dimensions, T>& min, const vector<Dimensions, T>& max)
  {
    return {min, max};
  }
}

/// Returns the smallest interval spanning all passed vectors.
template <typename... TVectors>
auto make_interval_from(const TVectors&... vectors)
{
  return detail::make_interval_from(
    math::make_vector_from(core::min(vectors(0)...), core::min(vectors(1)...)),
    math::make_vector_from(core::max(vectors(0)...), core::max(vectors(1)...)));
}

/// Returns the smallest interval fully spanning all passed intervals.
template <typename... TIntervals>
auto make_interval_union(const TIntervals&... intervals)
{
  return detail::make_interval_from(
    make_vector_from(core::min(intervals.min(0)...),
                     core::min(intervals.min(1)...)),
    make_vector_from(core::max(intervals.max(0)...),
                     core::max(intervals.max(1)...)));
}

/// Returns the intersection between all passed intervals.
template <typename... TIntervals>
auto make_interval_intersection(const TIntervals&... intervals)
{
  return detail::make_interval_from(
    make_vector_from(core::max(intervals.min(0)...),
                     core::max(intervals.min(1)...)),
    make_vector_from(core::min(intervals.max(0)...),
                     core::min(intervals.max(1)...)));
}
}

#endif
