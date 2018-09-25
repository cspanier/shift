#ifndef SHIFT_RENDER_VK_LAYER1_SWAPCHAIN_H
#define SHIFT_RENDER_VK_LAYER1_SWAPCHAIN_H

#include <boost/intrusive_ptr.hpp>
#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/layer1/semaphore.h"

namespace shift::render::vk::layer1
{
class instance;
class device;
class surface;
class image;

class swapchain
{
public:
  /// Default constructor.
  swapchain() = default;

  /// Constructor.
  swapchain(vk::layer1::device& device, vk::layer1::surface& surface,
            vk::extent_2d extent,
            std::uint32_t desired_number_of_swapchain_images,
            vk::layer1::swapchain* old_swapchain);

  swapchain(const swapchain&) = delete;

  /// Move constructor.
  swapchain(swapchain&& other) noexcept;

  /// Destructor.
  ~swapchain();

  swapchain& operator=(const swapchain&) = delete;

  /// Move assignment operator.
  swapchain& operator=(swapchain&& other) noexcept;

  ///
  vk::format surface_format() const noexcept;

  ///
  VkSwapchainKHR& handle() noexcept
  {
    return _swapchain;
  }

  ///
  const VkSwapchainKHR& handle() const noexcept
  {
    return _swapchain;
  }

  ///
  vk::extent_2d extent() const
  {
    return _extent;
  }

  ///
  std::uint32_t image_count() const;

  ///
  std::tuple<vk::result, std::uint32_t, VkSemaphore> acquire_next_image(
    std::uint64_t timeout, VkFence fence);

  ///
  vk::shared_ptr<vk::layer1::image> image(std::uint32_t swapchain_index) const;

private:
  vk::layer1::device* _device = nullptr;
  vk::layer1::surface* _surface = nullptr;
  VkSwapchainKHR _swapchain = nullptr;
  vk::extent_2d _extent;
  std::uint32_t _image_count = 0;
  std::uint32_t _semaphore_index = 0;
  vk::multi_buffered<VkImage> _image_handles = {nullptr, nullptr};
  vk::multi_buffered<vk::layer1::semaphore> _present_complete_semaphores;
};
}

#endif
