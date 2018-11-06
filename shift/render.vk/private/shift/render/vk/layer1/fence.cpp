#include "shift/render/vk/layer1/fence.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk::layer1
{
fence::fence(vk::layer1::device& device, vk::fence_create_flags flags) : fence()
{
  _device = &device;

  vk_check(vk::create_fence(
    _device->handle(),
    core::rvalue_address(vk::fence_create_info(
      /* next */ nullptr,
      /* flags */ flags)),
    renderer_impl::singleton_instance().default_allocator(), &_fence));
}

fence::fence(fence&& other) noexcept
: _device(other._device), _fence(other._fence)
{
  other._fence = nullptr;
}

fence::~fence()
{
  if (_fence != nullptr)
  {
    vk::destroy_fence(_device->handle(), _fence,
                      renderer_impl::singleton_instance().default_allocator());
    _fence = nullptr;
  }
}

fence& fence::operator=(fence&& other) noexcept
{
  if (_fence)
  {
    vk::destroy_fence(_device->handle(), _fence,
                      renderer_impl::singleton_instance().default_allocator());
  }
  _device = other._device;
  _fence = other._fence;
  other._fence = nullptr;
  return *this;
}

bool fence::status() const
{
  return vk::get_fence_status(_device->handle(), _fence) == vk::result::success;
}

bool fence::wait(std::chrono::nanoseconds timeout)
{
  return vk::wait_for_fences(_device->handle(), 1u, &_fence, VK_TRUE,
                             timeout.count()) == vk::result::success;
}

void fence::reset()
{
  vk::reset_fences(_device->handle(), 1u, &_fence);
}
}
