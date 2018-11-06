#ifndef SHIFT_RENDER_VK_LAYER1_FENCE_HPP
#define SHIFT_RENDER_VK_LAYER1_FENCE_HPP

#include <chrono>
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk::layer1
{
class device;

///
struct fence
{
public:
  /// Default constructor.
  fence() = default;

  fence(const fence&) = delete;

  /// Constructor.
  fence(vk::layer1::device& device, vk::fence_create_flags flags);

  /// Move constructor.
  fence(fence&& other) noexcept;

  /// Destructor.
  ~fence();

  fence& operator=(const fence&) = delete;

  /// Move assignment operator.
  fence& operator=(fence&& other) noexcept;

  ///
  VkFence& handle() noexcept
  {
    return _fence;
  }

  ///
  const VkFence& handle() const noexcept
  {
    return _fence;
  }

  ///
  bool status() const;

  ///
  bool wait(std::chrono::nanoseconds timeout);

  ///
  void reset();

  /// A non-valid fence object handle which may be passed to certain Vulkan
  /// functions that explicitely allow it.
  static constexpr VkFence null_handle = nullptr;

private:
  device* _device = nullptr;
  VkFence _fence = null_handle;
};
}

#endif
