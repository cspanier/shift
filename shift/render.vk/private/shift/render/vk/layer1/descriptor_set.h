#ifndef SHIFT_RENDER_VK_LAYER1_DESCRIPTORSET_H
#define SHIFT_RENDER_VK_LAYER1_DESCRIPTORSET_H

#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"

namespace shift::render::vk::layer1
{
class device;
class descriptor_pool;
class descriptor_set_layout;

///
class descriptor_set
{
public:
  /// Constructor.
  descriptor_set(vk::layer1::device& device,
                 vk::shared_ptr<vk::layer1::descriptor_pool> descriptor_pool,
                 vk::layer1::descriptor_set_layout& descriptor_set_layout);

  descriptor_set(const descriptor_set&) = delete;
  descriptor_set(descriptor_set&&) = delete;

  /// Destructor.
  ~descriptor_set();

  descriptor_set& operator=(const descriptor_set&) = delete;
  descriptor_set& operator=(descriptor_set&&) = delete;

  ///
  VkDescriptorSet& handle() noexcept
  {
    return _descriptor_set;
  }

  ///
  const VkDescriptorSet& handle() const noexcept
  {
    return _descriptor_set;
  }

private:
  device* _device = nullptr;
  vk::shared_ptr<vk::layer1::descriptor_pool> _descriptor_pool;
  VkDescriptorSet _descriptor_set = nullptr;
};
}

#endif
