#ifndef SHIFT_RENDER_VK_LAYER2_TEXTURE_H
#define SHIFT_RENDER_VK_LAYER2_TEXTURE_H

#include <memory>
#include "shift/render/vk/types.h"
#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/texture.h"
#include "shift/render/vk/layer1/buffer.h"
#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/image_view.h"
#include "shift/render/vk/layer2/object.h"

namespace shift::resource
{
class image;
}

namespace shift::render::vk::layer1
{
class device;
class command_buffer;
}

namespace shift::render::vk::layer2
{
///
enum class texture_state
{
  unavailable,
  queued,
  uploading,
  available
};

///
class texture final : public vk::texture, public child_object
{
public:
  /// Constructor.
  /// @param source
  ///   An optional data source.
  texture(vk::layer1::device& device, vk::format image_format,
          vk::extent_3d extent, std::uint32_t mip_levels,
          std::uint32_t array_layers, std::shared_ptr<resource::image> source);

  texture(const texture&) = delete;
  texture(texture&&) = delete;
  ~texture() noexcept final;
  texture& operator=(const texture&) = delete;
  texture& operator=(texture&&) = delete;

  ///
  vk::layer1::image& image() const
  {
    return *_image;
  }

  ///
  vk::layer1::image_view& image_view() const
  {
    return *_image_view;
  }

  ///
  const std::shared_ptr<resource::image>& source_image() const;

  ///
  std::uint32_t mip_levels() noexcept final;

  ///
  std::uint32_t array_layers() noexcept final;

  /// Copies a range of mip levels from the assigned image resource to the
  /// staging buffer.
  /// @pre
  ///   The texture has been assigned a valid image resource at construction.
  void copy_to_staging_buffer(std::uint32_t mip_level_begin,
                              std::uint32_t mip_level_end);

  /// Copies a range of mip levels from the staging buffer to device memory.
  /// @pre
  ///   All mip levels in the mip level range have been loaded to the staging
  ///   buffer using copy_to_staging_buffer.
  void copy_to_device_memory(vk::layer1::command_buffer& command_buffer,
                             std::uint32_t mip_level_begin,
                             std::uint32_t mip_level_end);

  ///
  std::uint32_t min_valid_mip_level() const;

  ///
  void min_valid_mip_level(std::uint32_t new_mip_level);

private:
  vk::layer1::device* _device;
  vk::shared_ptr<vk::layer1::image> _image;
  vk::shared_ptr<vk::layer1::buffer> _staging_buffer;
  vk::shared_ptr<vk::layer1::image_view> _image_view;
  std::uint32_t _array_layers;
  std::shared_ptr<resource::image> _source;
  std::vector<vk::buffer_image_copy> _buffer_copy_regions;
  std::uint32_t _min_valid_mip_level;
};
}

#endif
