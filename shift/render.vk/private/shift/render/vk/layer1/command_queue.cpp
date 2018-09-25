#include "shift/render/vk/layer1/command_queue.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/layer1/instance.h"
#include <shift/core/exception.h>
#include <cstdint>

namespace shift::render::vk::layer1
{
command_queue::command_queue(vk::layer1::device& device,
                             std::uint32_t family_index, std::uint32_t index)
: _device(&device), _family_index(family_index), _index(index)
{
  vk::get_device_queue(_device->handle(), _family_index, _index,
                       &_command_queue);
}

command_queue::command_queue(command_queue&& other) noexcept
: _device(other._device),
  _family_index(other._family_index),
  _index(other._index),
  _command_queue(other._command_queue)
{
}

command_queue::~command_queue()
{
  // The queue is part of the device and does not need to be destroyed
  // separately.
  _command_queue = nullptr;
}

command_queue& command_queue::operator=(command_queue&& other) noexcept
{
  _device = other._device;
  _family_index = other._family_index;
  _index = other._index;
  _command_queue = other._command_queue;
  return *this;
}

void command_queue::bind_sparse(std::uint32_t bind_info_count,
                                const vk::bind_sparse_info* bind_info,
                                const VkFence fence_handle)
{
  /// ToDo: Access to queue must be externally synchronized. Shall we add
  /// locks here or in the surrounding code?
  vk_check(vk::queue_bind_sparse(_command_queue, bind_info_count, bind_info,
                                 fence_handle));
}

vk::result command_queue::present(const vk::present_info_khr& present_info)
{
  return _device->queue_present(_command_queue, &present_info);
}

void command_queue::wait_idle()
{
  /// ToDo: Access to queue must be externally synchronized. Shall we add
  /// locks here or in the surrounding code?
  vk_check(vk::queue_wait_idle(_command_queue));
}
}
