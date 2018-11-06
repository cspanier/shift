#ifndef SHIFT_RENDER_VK_PASS_HPP
#define SHIFT_RENDER_VK_PASS_HPP

#include <shift/core/bit_field.hpp>
#include <shift/math/vector.hpp>
#include <shift/render/vk/shared.hpp>

namespace shift::resource
{
class scene;
}

namespace shift::render::vk::layer1
{
class device;
}

namespace shift::render::vk
{
enum class pass_flag : std::uint32_t
{
  terrain = 0b0001,
  scene = 0b0010,
  text = 0b0100,
  warp = 0b1000
};

using pass_flags = core::bit_field<pass_flag>;

///
class pass
{
public:
  /// Constructor.
  pass(vk::layer1::device& device);

protected:
  vk::layer1::device* _device;
};
}

#endif
