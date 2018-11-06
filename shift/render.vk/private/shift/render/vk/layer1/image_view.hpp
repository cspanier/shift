#ifndef SHIFT_RENDER_VK_LAYER1_IMAGEVIEW_HPP
#define SHIFT_RENDER_VK_LAYER1_IMAGEVIEW_HPP

#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;
class image;

///
class image_view
{
public:
  /// Constructor.
  image_view(vk::layer1::device& device,
             const image_view_create_info& create_info,
             vk::layer1::image& image);

  image_view(const image_view&) = delete;
  image_view(image_view&&) = delete;

  /// Destructor.
  ~image_view();

  image_view& operator=(const image_view&) = delete;
  image_view& operator=(image_view&&) = delete;

  ///
  VkImageView& handle() noexcept
  {
    return _image_view;
  }

  ///
  const VkImageView& handle() const noexcept
  {
    return _image_view;
  }

  /// Returns the references image.
  vk::layer1::image& image() const
  {
    return *_image;
  }

  ///
  const image_subresource_range& subresource_range() const
  {
    return _subresource_range;
  }

private:
  vk::layer1::device* _device = nullptr;
  VkImageView _image_view = nullptr;
  vk::layer1::image* _image = nullptr;
  image_subresource_range _subresource_range;
};
}

#endif
