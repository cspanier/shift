#include "shift/render/vk/layer1/shader_module.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
shader_module::shader_module(vk::layer1::device& device,
                             const shader_module_create_info& create_info)
: shader_module()
{
  _device = &device;

  vk_check(vk::create_shader_module(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_shader_module));
}

shader_module::~shader_module()
{
  if (_shader_module != nullptr)
  {
    vk::destroy_shader_module(
      _device->handle(), _shader_module,
      renderer_impl::singleton_instance().default_allocator());
    _shader_module = nullptr;
  }
}
}
