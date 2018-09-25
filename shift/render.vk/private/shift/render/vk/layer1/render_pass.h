#ifndef SHIFT_RENDER_VK_LAYER1_RENDERPASS_H
#define SHIFT_RENDER_VK_LAYER1_RENDERPASS_H

#include "shift/render/vk/shared.h"

namespace shift::render::vk::layer1
{
class device;

///
class render_pass
{
public:
  /// Constructor.
  render_pass(vk::layer1::device& device,
              const render_pass_create_info& create_info);

  render_pass(const render_pass&) = delete;
  render_pass(render_pass&&) = delete;

  /// Destructor.
  ~render_pass();

  render_pass& operator=(const render_pass&) = delete;
  render_pass& operator=(render_pass&&) = delete;

  ///
  VkRenderPass& handle() noexcept
  {
    return _render_pass;
  }

  ///
  const VkRenderPass& handle() const noexcept
  {
    return _render_pass;
  }

private:
  device* _device = nullptr;
  VkRenderPass _render_pass = nullptr;
};
}

#endif
