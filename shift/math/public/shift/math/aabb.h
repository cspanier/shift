#ifndef SHIFT_MATH_AABB_H
#define SHIFT_MATH_AABB_H

#include "shift/math/vector.h"

namespace shift::math
{
/// A simple axis-aligned bounding box class.
template <std::size_t N, typename T>
class aabb
{
public:
  vector<N, T> center;
  vector<N, T> extent;
};

/// Create an aabb instance from two vectors min and max where
/// min(i) <= max(i) for all 0 <= i < N.
template <std::size_t N, typename T>
aabb<N, T> make_aabb_from_min_max(const vector<N, T>& min,
                                  const vector<N, T>& max)
{
  for (std::size_t i = 0; i < N; ++i)
    BOOST_ASSERT(min(i) <= max(i));
  aabb<N, T> result;
  result.center = (max + min) / 2;
  result.extent = (max - min) / 2;
  return result;
}
}

#endif
