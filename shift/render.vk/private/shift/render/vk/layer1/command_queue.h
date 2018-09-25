#ifndef SHIFT_RENDER_VK_LAYER1_COMMANDQUEUE_H
#define SHIFT_RENDER_VK_LAYER1_COMMANDQUEUE_H

#include <vector>
#include "shift/render/vk/shared.h"
#include "shift/render/vk/layer1/fence.h"

namespace shift::render::vk::layer1
{
class device;
class command_pool;

///
class command_queue
{
public:
  /// Constructor.
  command_queue(vk::layer1::device& device, std::uint32_t family_index,
                std::uint32_t index);

  command_queue(const command_queue&) = delete;

  /// Move constructor.
  command_queue(command_queue&& other) noexcept;

  /// Destructor.
  ~command_queue();

  command_queue& operator=(const command_queue&) = delete;

  /// Move assignment operator.
  command_queue& operator=(command_queue&& other) noexcept;

  ///
  VkQueue& handle() noexcept
  {
    return _command_queue;
  }

  ///
  const VkQueue& handle() const noexcept
  {
    return _command_queue;
  }

  ///
  std::uint32_t family_index() const
  {
    return _family_index;
  }

  ///
  std::uint32_t index() const
  {
    return _index;
  }

  ///
  void submit(const vk::submit_info& submit_info, const VkFence fence_handle)
  {
    vk_check(vk::queue_submit(_command_queue, 1, &submit_info, fence_handle));
  }

  ///
  template <std::uint32_t SubmitInfoCount>
  void submit(const std::array<vk::submit_info, SubmitInfoCount>& submit_infos,
              const VkFence fence_handle)
  {
    vk_check(vk::queue_submit(_command_queue,
                              static_cast<std::uint32_t>(submit_infos.size()),
                              submit_infos.data(), fence_handle));
  }

  ///
  void bind_sparse(std::uint32_t bind_info_count,
                   const vk::bind_sparse_info* bind_info, VkFence fence_handle);

  ///
  void bind_sparse(const vk::bind_sparse_info& bind_info,
                   const VkFence fence_handle)
  {
    bind_sparse(1, &bind_info, fence_handle);
  }

  ///
  vk::result present(const vk::present_info_khr& present_info);

  ///
  void wait_idle();

private:
  device* _device = nullptr;
  std::uint32_t _family_index;
  std::uint32_t _index;
  VkQueue _command_queue = nullptr;
};
}

#endif
