#ifndef SHIFT_MATH_DETAIL_QUATERNION_IMPL_H
#define SHIFT_MATH_DETAIL_QUATERNION_IMPL_H

#include <cstdint>
#include <array>
#include <shift/core/types.h>
#include <shift/core/algorithm.h>
#include "shift/math/utility.h"

namespace shift::math::detail
{
// This file contains a collection of workarounds to overcome the
// limitations of C++11 style constexpr. Once MSVC supports C++14 style
// constexpr all of these functions may be merged back into their calling
// public interface functions.

template <typename T>
constexpr quaternion<T> mul(const quaternion<T>& lhs, T rhs) noexcept
{
  return {lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs};
}

template <typename T>
constexpr quaternion<T> inverse_impl(const quaternion<T>& q,
                                     T squared_magnitude) noexcept
{
  return (squared_magnitude >= epsilon<T>)
           ? mul(quaternion<T>{-q.x, -q.y, -q.z, q.w}, 1 / squared_magnitude)
           : quaternion<T>{T{0}, T{0}, T{0}, T{0}};
}
}

#endif
