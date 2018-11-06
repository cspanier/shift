#include "shift/render/vk/layer1/semaphore.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
semaphore::semaphore(vk::layer1::device& device) : semaphore()
{
  _device = &device;

  vk::semaphore_create_info create_info;
  create_info.flags(0);
  vk_check(vk::create_semaphore(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_semaphore));
}

semaphore::semaphore(semaphore&& other) noexcept
: _device(other._device), _semaphore(other._semaphore)
{
  other._semaphore = nullptr;
}

semaphore::~semaphore()
{
  reset();
}

semaphore& semaphore::operator=(semaphore&& other) noexcept
{
  if (_semaphore != nullptr)
  {
    vk::destroy_semaphore(
      _device->handle(), _semaphore,
      renderer_impl::singleton_instance().default_allocator());
  }
  _device = other._device;
  _semaphore = other._semaphore;
  other._semaphore = nullptr;
  return *this;
}

void semaphore::reset()
{
  if (_semaphore != nullptr)
  {
    vk::destroy_semaphore(
      _device->handle(), _semaphore,
      renderer_impl::singleton_instance().default_allocator());
    _semaphore = nullptr;
  }
}
}
