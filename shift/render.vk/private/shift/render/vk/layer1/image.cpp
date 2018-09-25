#include "shift/render/vk/layer1/image.h"
#include "shift/render/vk/layer1/command_queue.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/renderer_impl.h"
#include <shift/core/exception.h>
#include <cstdint>

namespace shift::render::vk::layer1
{
image::image(vk::layer1::device& device, const image_create_info& create_info)
: _device(&device)
{
  _format = create_info.format();
  _extent = {create_info.extent().width(), create_info.extent().height(),
             create_info.extent().depth()};
  _mip_levels = create_info.mip_levels();
  _array_layers = create_info.array_layers();
  vk_check(vk::create_image(
    _device->handle(), &create_info,
    renderer_impl::singleton_instance().default_allocator(), &_image));

  if (create_info.flags() & vk::image_create_flag::sparse_residency_bit)
  {
    std::uint32_t requirements_count = 0;
    vk::get_image_sparse_memory_requirements(_device->handle(), _image,
                                             &requirements_count, nullptr);
    if (requirements_count == 1)
    {
      vk::get_image_sparse_memory_requirements(_device->handle(), _image,
                                               &requirements_count,
                                               &_sparse_memory_requirements);
    }
    if (requirements_count > 1)
    {
      std::vector<vk::sparse_image_memory_requirements> requirements;
      requirements.resize(requirements_count);
      vk::get_image_sparse_memory_requirements(
        _device->handle(), _image, &requirements_count, requirements.data());
      /// ToDo: Select a memory requirements variant that best suits our
      /// needs.
      _sparse_memory_requirements = requirements.front();
    }

    if (_sparse_memory_requirements.format_properties().flags() &
        vk::sparse_image_format_flag::single_miptail_bit)
    {
      // The image uses a single mip tail region for all array layers.
    }
    if (_sparse_memory_requirements.format_properties().flags() &
        vk::sparse_image_format_flag::aligned_mip_size_bit)
    {
      // The first mip level whose dimensions are not integer multiples of
      // the corresponding dimensions of the sparse image block begins the
      // mip tail region.
    }
  }
}

image::image(VkImage image_handle, vk::image_type type, vk::format format,
             vk::extent_3d extent, std::uint32_t mip_levels,
             std::uint32_t array_layers)
: _image(image_handle),
  _type(type),
  _format(format),
  _extent(extent),
  _mip_levels(mip_levels),
  _array_layers(array_layers)
{
}

image::~image()
{
  // Note that _device will be null if the image wasn't created from it
  // (e.g. if the image was part of a swapchain), in which case it must not
  // be destroyed.
  if (_device != nullptr && _image != nullptr)
  {
    free_storage();
    vk::destroy_image(_device->handle(), _image,
                      renderer_impl::singleton_instance().default_allocator());
    _image = nullptr;
  }
}

void image::allocate_storage(memory_pool pool)
{
  BOOST_ASSERT(_device);
  vk::memory_requirements memory_requirements;
  vk::get_image_memory_requirements(_device->handle(), _image,
                                    &memory_requirements);

  _memory = _device->memory_manager().allocate(pool, memory_requirements);
}

void image::bind_storage()
{
  BOOST_ASSERT(_memory);
  if (_memory)
  {
    vk_check(vk::bind_image_memory(_device->handle(), _image, _memory.handle(),
                                   _memory.offset()));
  }
}

void image::bind_sparse_storage(vk::locked_command_queue& queue)
{
  BOOST_ASSERT(_memory);
  if (_memory)
  {
    queue->bind_sparse(
      vk::bind_sparse_info(
        /* next */ nullptr,
        /* wait_semaphore_count */ 0,
        /* wait_semaphores */ nullptr,
        /* buffer_bind_count */ 0,
        /* buffer_binds */ nullptr,
        /* image_opaque_bind_count */ 1,
        /* image_opaque_binds */
        core::rvalue_address(vk::sparse_image_opaque_memory_bind_info(
          /* image */ _image,
          /* bind_count */ 1,
          /* binds */
          core::rvalue_address(vk::sparse_memory_bind(
            /* resource_offset */ 0,
            /* size */ _memory.size(),
            /* memory */ _memory.handle(),
            /* memory_offset */ _memory.offset(),
            /* flags */ vk::sparse_memory_bind_flag::none)))),
        /* image_bind_count */ 0,
        /* image_binds */ nullptr,
        /* signal_semaphore_count */ 0,
        /* signal_semaphores */ nullptr),
      vk::layer1::fence::null_handle);
  }
}

void image::free_storage()
{
  BOOST_ASSERT(_device);
  if (_memory)
    _device->memory_manager().free(_memory);
}

void* image::address(vk::subresource_layout& layout,
                     vk::image_aspect_flags aspect_mask, uint32_t mip_level,
                     uint32_t array_layer)
{
  BOOST_ASSERT(_device);
  vk::get_image_subresource_layout(_device->handle(), _image,
                                   core::rvalue_address(vk::image_subresource(
                                     /* aspect_mask */ aspect_mask,
                                     /* mip_level */ mip_level,
                                     /* array_layer */ array_layer)),
                                   &layout);

  return _memory.address(layout.offset(), layout.size());
}
}
