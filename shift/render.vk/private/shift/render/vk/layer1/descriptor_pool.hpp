#ifndef SHIFT_RENDER_VK_LAYER1_DESCRIPTORPOOL_HPP
#define SHIFT_RENDER_VK_LAYER1_DESCRIPTORPOOL_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
class descriptor_pool
{
public:
  /// Constructor.
  descriptor_pool(vk::layer1::device& device,
                  const descriptor_pool_create_info& create_info);

  descriptor_pool(const descriptor_pool&) = delete;
  descriptor_pool(descriptor_pool&&) = delete;

  /// Destructor.
  ~descriptor_pool();

  descriptor_pool& operator=(const descriptor_pool&) = delete;
  descriptor_pool& operator=(descriptor_pool&&) = delete;

  ///
  VkDescriptorPool& handle() noexcept
  {
    return _descriptor_pool;
  }

  ///
  const VkDescriptorPool& handle() const noexcept
  {
    return _descriptor_pool;
  }

  /// Returns whether this descriptor pool supports freeing allocated sets.
  bool supports_free() const
  {
    return _supports_free;
  }

private:
  device* _device = nullptr;
  VkDescriptorPool _descriptor_pool = nullptr;
  bool _supports_free = false;
};
}

#endif
