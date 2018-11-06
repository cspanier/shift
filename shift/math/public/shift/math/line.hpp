#ifndef SHIFT_MATH_LINE_HPP
#define SHIFT_MATH_LINE_HPP

#include "shift/math/vector.hpp"

namespace shift::math
{
/// A template class for line segments in T^N.
template <std::size_t N, typename T>
class line
{
public:
  /// Default constructor.
  line() = default;

  /// Constructor.
  line(const math::vector<N, T>& begin, const math::vector<N, T>& end)
  : _start(begin), _end(end)
  {
  }

  const math::vector<N, T>& begin() const
  {
    return _start;
  }

  const math::vector<N, T>& end() const
  {
    return _end;
  }

private:
  math::vector<N, T> _start;
  math::vector<N, T> _end;
};
}

#endif
