#ifndef SHIFT_RENDER_VK_WINDOW_IMPL_H
#define SHIFT_RENDER_VK_WINDOW_IMPL_H

#include <shift/core/types.h>
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/window.h"
#include "shift/render/vk/layer1/semaphore.h"
#include "shift/render/vk/layer1/image.h"

namespace shift::render::vk::layer1
{
class physical_device;
}

namespace shift::render::vk
{
///
class window_impl : public vk::window
{
public:
  /// Constructor.
  window_impl(math::vector2<std::int32_t> initial_position,
              math::vector2<std::uint32_t> initial_size,
              vk::window_flags flags);

  /// Destructor.
  ~window_impl() override;

  /// Assigns a new or resets an existing physical device.
  void physical_device(const vk::layer1::physical_device* new_physical_device)
  {
    _physical_device = new_physical_device;
  }

  /// Returns the assigned physical device.
  const vk::layer1::physical_device& physical_device()
  {
    BOOST_ASSERT(_physical_device);
    return *_physical_device;
  }

  ///
  VkSemaphore present_complete_semaphore()
  {
    return _present_complete_semaphore;
  }

  ///
  vk::pipeline_stage_flag present_stage_flags()
  {
    return _present_stage_flags;
  }

  ///
  vk::access_flags present_access_flags()
  {
    return _present_access_flags;
  }

  ///
  vk::image_layout present_image_layout()
  {
    return _present_image_layout;
  }

  /// Creates a window surface.
  /// @pre
  ///   destroy_surface has to be called before calling create_surface a second
  ///   time.
  virtual void create_surface() = 0;

  /// Clears any resources allocated using create_surface.
  /// @remarks
  ///   This method may be called without prior calling create_surface.
  virtual void destroy_surface() = 0;

  /// Creates a swapchain or a series of images.
  /// @return
  ///   Returns the current swapchain buffer index.
  virtual std::uint32_t create_swapchain(vk::layer1::device& device) = 0;

  /// Clears any resources allocated using create_swapchain.
  virtual void destroy_swapchain() = 0;

  /// Returns the number of images in the internal swapchain.
  /// @pre
  ///   Only valid between calls to create_swapchain and destroy_swapchain.
  virtual std::uint32_t image_count() const = 0;

  /// Returns an image from the internal swapchain.
  /// @param image_index
  ///   0 <= image_index < image_count.
  virtual vk::shared_ptr<vk::layer1::image> image(
    std::uint32_t image_index) const = 0;

  /// Acquires a new image from the window's swapchain.
  /// @return
  ///   Returns the new current swapchain buffer index.
  virtual std::uint32_t acquire_next_image() = 0;

  /// @remarks
  ///   In case of a device loss the method will call window::on_resize.
  /// @param wait_semaphore_count
  ///   Number of semaphores to wait on before the internal swapchain images may
  ///   be presented. You may set this parameter to zero if the API does not
  ///   require any semaphores.
  /// @param wait_semaphores
  ///   A pointer to an array of wait_semaphore_count semaphore handles. You may
  ///   set this parameter to nullptr if the API does not require any
  ///   semaphores.
  /// @pre
  ///   You need to wait for all passed wait_semaphores in order to present
  ///   anything.
  /// @post
  ///   present_complete_semaphore will be signalled once the window system
  ///   completes presenting.
  /// @return
  ///   Returns whether the images could be presented.
  virtual bool present(uint32_t wait_semaphore_count,
                       std::uint64_t* wait_semaphores,
                       std::uint32_t current_buffer) = 0;

protected:
  const vk::layer1::physical_device* _physical_device = nullptr;
  vk::layer1::device* _device = nullptr;
  VkSemaphore _present_complete_semaphore = nullptr;
  vk::pipeline_stage_flag _present_stage_flags = vk::pipeline_stage_flag::none;
  vk::access_flags _present_access_flags = vk::access_flag::none;
  vk::image_layout _present_image_layout = vk::image_layout::general;
};
}

#endif
