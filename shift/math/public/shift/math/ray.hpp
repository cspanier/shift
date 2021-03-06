#ifndef SHIFT_MATH_RAY_HPP
#define SHIFT_MATH_RAY_HPP

#include "shift/math/vector.hpp"

namespace shift::math
{
/// A template class for rays in T^N.
template <std::size_t N, typename T>
class ray
{
public:
  /// Default constructor.
  ray() = default;

  /// Constructor.
  constexpr ray(const math::vector<N, T>& origin,
                const math::vector<N, T>& direction)
  : origin(origin), direction(direction)
  {
  }

public:
  math::vector<N, T> origin;
  math::vector<N, T> direction;
};
}

#endif
