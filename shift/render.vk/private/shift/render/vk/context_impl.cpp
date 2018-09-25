#include "shift/render/vk/context_impl.h"
#include "shift/render/vk/renderer_impl.h"
#include "shift/render/vk/layer1/command_buffer.h"
#include "shift/render/vk/layer1/command_queue.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/utility.h"
#include <chrono>
#include <algorithm>

namespace shift::render::vk
{
context::~context() noexcept
{
}

void context::release()
{
  if (_reference_counter.fetch_sub(1, std::memory_order_release) == 1)
  {
    std::atomic_thread_fence(std::memory_order_acquire);
    renderer_impl::singleton_instance().destroy_context(this);
  }
}

context_impl::context_impl() = default;

context_impl::~context_impl() noexcept
{
}

void context_impl::initialize(vk::layer1::device& device)
{
  BOOST_ASSERT(_device == nullptr);

  _device = &device;

  _command_buffer = vk::make_framed_shared<vk::layer1::command_buffer>(
    _device->select_command_pool(vk::layer1::queue_type::graphics, true),
    vk::command_buffer_level::primary);

  _submit_fence = vk::layer1::fence(*_device, vk::fence_create_flag::none);
}

void context_impl::finalize()
{
  _submit_fence = vk::layer1::fence();
  if (_command_buffer)
  {
    /// ToDo: Is it ok to simply delete the object?
    _command_buffer.reset();
  }
  _device = nullptr;
}

bool context_impl::can_begin()
{
  return !_need_to_wait || _submit_fence.status();
}

void context_impl::begin()
{
  using namespace std::chrono_literals;

  wait(1s);

  _command_buffer->begin(vk::command_buffer_begin_info(
    /* next */ nullptr,
    /* flags */ vk::command_buffer_usage_flag::one_time_submit_bit,
    /* inheritance_info */ nullptr));
}

void context_impl::end()
{
  if (_command_buffer->empty())
    return;
  _command_buffer->end();

  _device->graphics_queue()->submit(
    vk::submit_info(
      /* next */ nullptr,
      /* wait_semaphore_count */ 0,
      /* wait_semaphores */ nullptr,
      /* wait_dst_stage_mask */ nullptr,
      /* command_buffer_count */ 1,
      /* command_buffers */ &_command_buffer->handle(),
      /* signal_semaphore_count */ 0,
      /* signal_semaphores */ nullptr),
    _submit_fence.handle());
  _need_to_wait = true;
}

void context_impl::wait(std::chrono::nanoseconds timeout)
{
  if (_need_to_wait)
  {
    /// ToDo: Handle timeout.
    _submit_fence.wait(timeout);
    _submit_fence.reset();
    _need_to_wait = false;
  }
}
}
