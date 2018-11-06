#include "shift/render/vk/layer1/command_pool.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include <shift/core/exception.hpp>
#include <cstdint>

namespace shift::render::vk::layer1
{
command_pool::command_pool() noexcept
{
}

command_pool::command_pool(vk::layer1::device& device,
                           std::uint32_t queue_family_index,
                           command_pool_create_flags create_flags) noexcept
: command_pool()
{
  _device = &device;

  vk_check(vk::create_command_pool(
    device.handle(),
    core::rvalue_address(vk::command_pool_create_info(
      /* next */ nullptr,
      /* flags */ create_flags,
      /* queue_family_index */ queue_family_index)),
    renderer_impl::singleton_instance().default_allocator(), &_command_pool));
}

command_pool::~command_pool()
{
  if (_command_pool != nullptr)
  {
    vk::destroy_command_pool(
      _device->handle(), _command_pool,
      renderer_impl::singleton_instance().default_allocator());
    _command_pool = nullptr;
  }
}
}
