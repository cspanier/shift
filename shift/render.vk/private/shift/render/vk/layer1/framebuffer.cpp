#include "shift/render/vk/layer1/framebuffer.hpp"
#include "shift/render/vk/layer1/pipeline_cache.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
framebuffer::framebuffer(vk::layer1::device& device,
                         const framebuffer_create_info& create_info)
: _device(&device)
{
  vk_check(vk::create_framebuffer(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_framebuffer));
}

framebuffer::~framebuffer()
{
  if (_framebuffer != nullptr)
  {
    vk::destroy_framebuffer(
      _device->handle(), _framebuffer,
      renderer_impl::singleton_instance().default_allocator());
    _framebuffer = nullptr;
  }
}
}
