#include "shift/render/vk/layer1/descriptor_pool.h"
#include "shift/render/vk/layer1/descriptor_set.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"

namespace shift::render::vk::layer1
{
descriptor_pool::descriptor_pool(vk::layer1::device& device,
                                 const descriptor_pool_create_info& create_info)
: _device(&device),
  _supports_free(create_info.flags() &
                 vk::descriptor_pool_create_flag::free_descriptor_set_bit)
{
  vk_check(vk::create_descriptor_pool(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(),
    &_descriptor_pool));
}

descriptor_pool::~descriptor_pool()
{
  if (_descriptor_pool != nullptr)
  {
    vk::destroy_descriptor_pool(
      _device->handle(), _descriptor_pool,
      renderer_impl::singleton_instance().default_allocator());
    _descriptor_pool = nullptr;
  }
}
}
