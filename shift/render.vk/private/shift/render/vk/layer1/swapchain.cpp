#include "shift/render/vk/layer1/swapchain.h"
#include "shift/render/vk/layer1/surface.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"
#include <shift/core/exception.h>
#include <cstdint>

namespace shift::render::vk::layer1
{
swapchain::swapchain(vk::layer1::device& device, vk::layer1::surface& surface,
                     vk::extent_2d extent,
                     std::uint32_t desired_number_of_swapchain_images,
                     vk::layer1::swapchain* old_swapchain)
: swapchain()
{
  _device = &device;
  _surface = &surface;

  // Check the surface capabilities and formats
  vk::surface_capabilities_khr surface_capabilities;
  _surface->capabilities(surface_capabilities);

  std::vector<vk::present_mode_khr> available_present_modes;
  _surface->available_present_modes(available_present_modes);

  // If mailbox mode is available, use it, as is the lowest-latency
  // non-tearing mode.  If not, try IMMEDIATE which will usually be
  // available, and is fastest (though it tears).  If not, fall back to FIFO
  // which is always available.
  vk::present_mode_khr present_mode =
    vk::present_mode_khr::present_mode_fifo_khr;
  for (const auto available_present_mode : available_present_modes)
  {
    if (available_present_mode ==
        vk::present_mode_khr::present_mode_mailbox_khr)
    {
      present_mode = available_present_mode;
      break;
    }
    else if (available_present_mode ==
             vk::present_mode_khr::present_mode_immediate_khr)
    {
      present_mode = available_present_mode;
    }
  }

  // width and height are either both -1, or both not -1.
  if (surface_capabilities.current_extent().width() ==
      std::numeric_limits<std::uint32_t>::max())
  {
    // If the surface size is undefined, the size is set to
    // the size of the images requested.
    _extent = extent;
  }
  else
  {
    // If the surface size is defined, the swap chain size must match.
    _extent = surface_capabilities.current_extent();
  }

  if (desired_number_of_swapchain_images <
      surface_capabilities.min_image_count())
  {
    // Application must adjust to more images than requested.
    desired_number_of_swapchain_images = surface_capabilities.min_image_count();
  }
  if ((surface_capabilities.max_image_count() > 0) &&
      (desired_number_of_swapchain_images >
       surface_capabilities.max_image_count()))
  {
    // Application must settle for fewer images than desired:
    desired_number_of_swapchain_images = surface_capabilities.max_image_count();
  }

  vk::surface_transform_flag_khr pre_transform =
    (surface_capabilities.supported_transforms() &
     vk::surface_transform_flag_khr::identity_bit_khr)
      ? vk::surface_transform_flag_khr::identity_bit_khr
      : surface_capabilities.current_transform();

  vk_check(_device->create_swapchain(
    vk::swapchain_create_info_khr(
      /* next */ nullptr,
      /* flags */ vk::swapchain_create_flag_khr::none,
      /* surface */ _surface->handle(),
      /* min_image_count */ desired_number_of_swapchain_images,
      /* image_format */ _surface->format(),
      /* image_color_space */ _surface->color_space(),
      /* image_extent */ _extent,
      /* image_array_layers */ 1,
      /* image_usage */ vk::image_usage_flag::color_attachment_bit |
        vk::image_usage_flag::transfer_dst_bit,
      /* image_sharing_mode */ vk::sharing_mode::exclusive,
      /* queue_family_index_count */ 0,
      /* queue_family_indices */ nullptr,
      /* pre_transform */ pre_transform,
      /* composite_alpha */ vk::composite_alpha_flag_khr::opaque_bit_khr,
      /* present_mode */ present_mode,
      /* clipped */ true,
      /* old_swapchain */ old_swapchain ? old_swapchain->handle() : nullptr),
    renderer_impl::singleton_instance().default_allocator(), &_swapchain));

  vk_check(_device->swapchain_images(_swapchain, &_image_count, nullptr));
  BOOST_ASSERT(_image_count <= _image_handles.size());
  vk_check(_device->swapchain_images(_swapchain, &_image_count,
                                     _image_handles.data()));

  for (auto& present_complete_semaphore : _present_complete_semaphores)
    present_complete_semaphore = vk::layer1::semaphore(device);
}

swapchain::swapchain(swapchain&& other) noexcept
: _device(std::move(other._device)),
  _surface(std::move(other._surface)),
  _swapchain(std::move(other._swapchain)),
  _extent(std::move(other._extent)),
  _image_count(std::move(other._image_count)),
  _semaphore_index(std::move(other._semaphore_index))
{
  for (std::uint32_t i = 0u; i != _image_count; ++i)
    _image_handles[i] = std::move(other._image_handles[i]);
  for (std::size_t i = 0u; i != _present_complete_semaphores.size(); ++i)
  {
    _present_complete_semaphores[i] =
      std::move(other._present_complete_semaphores[i]);
  }

  other._swapchain = nullptr;
}

swapchain::~swapchain()
{
  if (_swapchain != nullptr)
  {
    _device->destroy_swapchain(
      _swapchain, renderer_impl::singleton_instance().default_allocator());
    _swapchain = nullptr;
  }
}

swapchain& swapchain::operator=(swapchain&& other) noexcept
{
  if (_swapchain)
  {
    _device->destroy_swapchain(
      _swapchain, renderer_impl::singleton_instance().default_allocator());
  }

  _device = std::move(other._device);
  _surface = std::move(other._surface);
  _swapchain = std::move(other._swapchain);
  _extent = std::move(other._extent);
  _image_count = std::move(other._image_count);
  _semaphore_index = std::move(other._semaphore_index);

  for (std::uint32_t i = 0u; i != _image_count; ++i)
    _image_handles[i] = std::move(other._image_handles[i]);
  for (std::size_t i = 0u; i != _present_complete_semaphores.size(); ++i)
  {
    _present_complete_semaphores[i] =
      std::move(other._present_complete_semaphores[i]);
  }
  other._swapchain = nullptr;

  return *this;
}

vk::format swapchain::surface_format() const noexcept
{
  return _surface->format();
}

std::uint32_t swapchain::image_count() const
{
  return _image_count;
}

std::tuple<vk::result, std::uint32_t, VkSemaphore>
swapchain::acquire_next_image(std::uint64_t timeout, VkFence fence)
{
  auto semaphore_index = _semaphore_index + 1;
  _semaphore_index = (semaphore_index < _image_count) ? semaphore_index : 0u;

  std::uint32_t swapchain_index = std::numeric_limits<std::uint32_t>::max();
  VkSemaphore semaphore_handle =
    _present_complete_semaphores[_semaphore_index].handle();
  auto result = _device->acquire_next_image(
    _swapchain, timeout, semaphore_handle, fence, swapchain_index);
  return std::make_tuple(result, swapchain_index, semaphore_handle);
}

vk::shared_ptr<vk::layer1::image> swapchain::image(
  std::uint32_t swapchain_index) const
{
  if (swapchain_index < _image_count)
  {
    return vk::make_framed_shared<vk::layer1::image>(
      _image_handles[swapchain_index], vk::image_type::_2d, surface_format(),
      vk::extent_3d{extent().width(), extent().height(), 1u}, 1u, 1u);
  }
  else
    return nullptr;
}
}
