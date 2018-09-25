#ifndef SHIFT_RENDER_VK_CONTEXT_IMPL_H
#define SHIFT_RENDER_VK_CONTEXT_IMPL_H

#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/context.h"
#include "shift/render/vk/layer1/fence.h"

namespace shift::render::vk::layer1
{
class device;
class command_queue;
class command_buffer;
}

namespace shift::render::vk::layer2
{
class texture;
}

namespace shift::render::vk
{
///
class context_impl final : public context
{
public:
  ///
  context_impl();

  context_impl(const context_impl&) = delete;
  context_impl(context_impl&&) noexcept = delete;

  ///
  ~context_impl() noexcept final;

  context_impl& operator=(const context_impl&) = delete;
  context_impl& operator=(context_impl&&) noexcept = delete;

  ///
  void initialize(vk::layer1::device& device);

  ///
  void finalize();

  ///
  vk::layer1::command_buffer& command_buffer()
  {
    return *_command_buffer;
  }

  ///
  bool can_begin() final;

  ///
  void begin() final;

  ///
  void end() final;

  ///
  void wait(std::chrono::nanoseconds timeout) final;

private:
  vk::layer1::device* _device = nullptr;
  vk::shared_ptr<vk::layer1::command_buffer> _command_buffer;
  vk::layer1::fence _submit_fence;
  bool _need_to_wait = false;
};
}

#endif
