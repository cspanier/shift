#ifndef SHIFT_RC_IMAGE_UTIL_BUILT_IN_ICC_PROFILES_HPP
#define SHIFT_RC_IMAGE_UTIL_BUILT_IN_ICC_PROFILES_HPP

#include <cstdint>
#include <array>

namespace shift::rc::image_util
{
const std::array<std::uint8_t, 688>& linear_icc();

const std::array<std::uint8_t, 672>& srgb_icc();
}

#endif
