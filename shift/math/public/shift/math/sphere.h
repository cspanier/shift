#ifndef SHIFT_MATH_SPHERE_H
#define SHIFT_MATH_SPHERE_H

#include "shift/math/vector.h"

namespace shift::math
{
/// A template class for circles/spheres in T^N.
template <std::size_t N, typename T>
class sphere final
{
public:
  /// Default constructor.
  sphere() = default;

  /// Constructor.
  constexpr sphere(const math::vector<N, T>& center, const T radius)
  : center(center), radius(radius)
  {
  }

public:
  math::vector<N, T> center;
  T radius;
};
}

#endif
