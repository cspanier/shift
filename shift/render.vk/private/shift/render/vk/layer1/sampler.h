#ifndef SHIFT_RENDER_VK_LAYER1_SAMPLER_H
#define SHIFT_RENDER_VK_LAYER1_SAMPLER_H

#include "shift/render/vk/shared.h"

namespace shift::render::vk::layer1
{
class device;

///
class sampler
{
public:
  /// Constructor.
  sampler(vk::layer1::device& device, const sampler_create_info& create_info);

  sampler(const sampler&) = delete;
  sampler(sampler&&) = delete;

  /// Destructor.
  ~sampler();

  sampler& operator=(const sampler&) = delete;
  sampler& operator=(sampler&&) = delete;

  ///
  VkSampler& handle() noexcept
  {
    return _sampler;
  }

  ///
  const VkSampler& handle() const noexcept
  {
    return _sampler;
  }

private:
  device* _device = nullptr;
  VkSampler _sampler = nullptr;
};
}

#endif
