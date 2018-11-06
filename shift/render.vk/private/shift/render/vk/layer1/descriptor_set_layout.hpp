#ifndef SHIFT_RENDER_VK_LAYER1_DESCRIPTORSETLAYOUT_HPP
#define SHIFT_RENDER_VK_LAYER1_DESCRIPTORSETLAYOUT_HPP

#include <vector>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
class descriptor_set_layout
{
public:
  /// Default constructor.
  descriptor_set_layout() noexcept = default;

  /// Constructor.
  descriptor_set_layout(vk::layer1::device& device,
                        const descriptor_set_layout_create_info& create_info);

  descriptor_set_layout(const descriptor_set_layout&) = delete;
  descriptor_set_layout(descriptor_set_layout&&) = delete;

  /// Destructor.
  ~descriptor_set_layout();

  descriptor_set_layout& operator=(const descriptor_set_layout&) = delete;
  descriptor_set_layout& operator=(descriptor_set_layout&&) = delete;

  ///
  VkDescriptorSetLayout& handle() noexcept
  {
    return _descriptor_set_layout;
  }

  ///
  const VkDescriptorSetLayout& handle() const noexcept
  {
    return _descriptor_set_layout;
  }

private:
  device* _device = nullptr;
  VkDescriptorSetLayout _descriptor_set_layout = nullptr;
};
}

#endif
