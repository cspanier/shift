#ifndef SHIFT_MATH_TRIANGLE_H
#define SHIFT_MATH_TRIANGLE_H

#include <math.h>
#include "shift/core/core.h"
#include "shift/core/algorithm.h"
#include "shift/math/vector.h"

namespace shift::math
{
/// Tests whether the point p is located within the triangle defined by the
/// tree points a, b, and c, using Barycentric coordinates.
/// @remarks
///   For N > 2 this technique only works if p is on the plane defined by the
///   triangle.
template <bool IncludeEdge, typename K, std::size_t N, ENABLE_IF(N >= 2)>
bool point_in_triangle(const vector<K, N>& a, const vector<K, N>& b,
                       const vector<K, N>& c, const vector<K, N>& p)
{
  auto v0 = b - a;
  auto v1 = c - a;
  auto v2 = p - a;

  auto dot00 = v0 * v0;
  auto dot01 = v0 * v1;
  auto dot02 = v0 * v2;
  auto dot10 = v1 * v1;
  auto dot12 = v1 * v2;

  auto invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
  auto u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  auto v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  if (IncludeEdge)
    return u >= 0 && v >= 0 && u + v <= 1;
  else
    return u > 0 && v > 0 && u + v < 1;
}
}

#endif
