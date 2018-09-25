#include "shift/render/vk/layer1/image_view.h"
#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"
#include <shift/core/exception.h>
#include <cstdint>

namespace shift::render::vk::layer1
{
image_view::image_view(vk::layer1::device& device,
                       const image_view_create_info& create_info,
                       vk::layer1::image& image)
: _device(&device), _image(&image)
{
  _subresource_range = create_info.subresource_range();
  vk_check(vk::create_image_view(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_image_view));
}

image_view::~image_view()
{
  _image = nullptr;
  if (_image_view != nullptr)
  {
    vk::destroy_image_view(
      _device->handle(), _image_view,
      renderer_impl::singleton_instance().default_allocator());
    _image_view = nullptr;
  }
}
}
