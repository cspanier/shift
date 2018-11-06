#include "shift/render/vk/layer1/render_pass.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
render_pass::render_pass(vk::layer1::device& device,
                         const render_pass_create_info& create_info)
: _device(&device)
{
  vk_check(vk::create_render_pass(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_render_pass));
}

render_pass::~render_pass()
{
  if (_render_pass != nullptr)
  {
    vk::destroy_render_pass(
      _device->handle(), _render_pass,
      renderer_impl::singleton_instance().default_allocator());
    _render_pass = nullptr;
  }
}
}
