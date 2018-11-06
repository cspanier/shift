#ifndef SHIFT_RENDER_VK_LAYER1_FRAMEBUFFER_HPP
#define SHIFT_RENDER_VK_LAYER1_FRAMEBUFFER_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
class framebuffer
{
public:
  /// Constructor.
  framebuffer(vk::layer1::device& device,
              const framebuffer_create_info& create_info);

  framebuffer(const framebuffer&) = delete;
  framebuffer(framebuffer&&) = delete;

  /// Destructor.
  ~framebuffer();

  framebuffer& operator=(const framebuffer&) = delete;
  framebuffer& operator=(framebuffer&&) = delete;

  ///
  VkFramebuffer& handle() noexcept
  {
    return _framebuffer;
  }

  ///
  const VkFramebuffer& handle() const noexcept
  {
    return _framebuffer;
  }

private:
  device* _device = nullptr;
  VkFramebuffer _framebuffer = nullptr;
};
}

#endif
