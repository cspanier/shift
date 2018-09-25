#ifndef SHIFT_RENDER_VK_LAYER1_IMAGE_H
#define SHIFT_RENDER_VK_LAYER1_IMAGE_H

#include "shift/render/vk/shared.h"
#include "shift/render/vk/memory_manager.h"

namespace shift::render::vk
{
class locked_command_queue;
}

namespace shift::render::vk::layer1
{
class device;
class swapchain;
class command_queue;

///
/// @remarks
///   The type is not moveable because image_view holds a pointer to its
///   associated image instance.
class image
{
public:
  /// Constructor.
  image(vk::layer1::device& device, const image_create_info& create_info);

  /// Constructor creating the object from an existing image (e.g. one of a
  /// swapchain), which won't be automatically destroyed in the destructor.
  image(VkImage image_handle, vk::image_type type, vk::format format,
        vk::extent_3d extent, std::uint32_t mip_levels,
        std::uint32_t array_layers);

  image(const image&) = delete;
  image(image&&) = delete;

  /// Destructor.
  ~image();

  image& operator=(const image&) = delete;
  image& operator=(image&&) = delete;

  ///
  VkImage& handle() noexcept
  {
    return _image;
  }

  ///
  const VkImage& handle() const noexcept
  {
    return _image;
  }

  ///
  vk::image_type type() const
  {
    return _type;
  }

  ///
  vk::format format() const
  {
    return _format;
  }

  ///
  vk::extent_3d extent() const
  {
    return _extent;
  }

  ///
  std::uint32_t mip_levels() const
  {
    return _mip_levels;
  }

  ///
  std::uint32_t array_layers() const
  {
    return _array_layers;
  }

  ///
  void allocate_storage(memory_pool pool);

  ///
  void bind_storage();

  ///
  void bind_sparse_storage(vk::locked_command_queue& queue);

  ///
  void free_storage();

  ///
  void* address(vk::subresource_layout& layout,
                vk::image_aspect_flags aspect_mask, uint32_t mip_level,
                uint32_t array_layer);

private:
  device* _device = nullptr;
  VkImage _image = nullptr;
  vk::image_type _type = vk::image_type::_2d;
  vk::format _format = vk::format::undefined;
  vk::extent_3d _extent = {1, 1, 1};
  std::uint32_t _mip_levels = 1;
  std::uint32_t _array_layers = 1;
  vk::sparse_image_memory_requirements _sparse_memory_requirements;
  memory_block _memory;
};
}

#endif
