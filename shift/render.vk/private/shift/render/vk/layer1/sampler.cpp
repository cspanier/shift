#include "shift/render/vk/layer1/sampler.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
sampler::sampler(vk::layer1::device& device,
                 const sampler_create_info& create_info)
: _device(&device)
{
  vk_check(vk::create_sampler(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_sampler));
}

sampler::~sampler()
{
  if (_sampler != nullptr)
  {
    vk::destroy_sampler(
      _device->handle(), _sampler,
      renderer_impl::singleton_instance().default_allocator());
    _sampler = nullptr;
  }
}
}
