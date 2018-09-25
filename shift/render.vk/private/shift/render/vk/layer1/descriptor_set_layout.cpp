#include "shift/render/vk/layer1/descriptor_set_layout.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"
#include <shift/core/exception.h>
#include <cstdint>

namespace shift::render::vk::layer1
{
descriptor_set_layout::descriptor_set_layout(
  vk::layer1::device& device,
  const descriptor_set_layout_create_info& create_info)
: descriptor_set_layout()
{
  _device = &device;

  vk_check(vk::create_descriptor_set_layout(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(),
    &_descriptor_set_layout));
}

descriptor_set_layout::~descriptor_set_layout()
{
  if (_descriptor_set_layout != nullptr)
  {
    vk::destroy_descriptor_set_layout(
      _device->handle(), _descriptor_set_layout,
      renderer_impl::singleton_instance().default_allocator());
    _descriptor_set_layout = nullptr;
  }
}
}
