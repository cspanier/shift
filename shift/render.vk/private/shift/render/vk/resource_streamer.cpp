#include "shift/render/vk/resource_streamer.hpp"
#include "shift/render/vk/utility.hpp"
#include "shift/render/vk/renderer.hpp"
#include "shift/render/vk/layer2/texture.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include <shift/resource_db/buffer_view.hpp>
#include <shift/resource_db/image.hpp>
#include <shift/resource_db/repository.hpp>

namespace shift::render::vk
{
resource_streamer::resource_streamer(vk::layer1::device& device)
: _device(&device)
{
}

resource_streamer::~resource_streamer() = default;

void resource_streamer::create_resources()
{
  _staging_buffer = vk::make_framed_shared<vk::layer1::buffer>(
    *_device, vk::buffer_create_info(
                /* next */ nullptr,
                /* flags */ vk::buffer_create_flag::none,
                /* size */ staging_buffer_size,
                /* usage */ vk::buffer_usage_flag::transfer_src_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr));
  _staging_buffer->allocate_storage(memory_pool::staging_buffers);
  _staging_buffer->bind_storage();
  _staging_memory = _staging_buffer->address(0);

  /// ToDo: The application should choose whether to precache all textures
  /// or not.
  // auto& repository = resource_db::repository::singleton_instance();
  // auto resources = repository.load_all(resource_db::resource_type::image);
  // for (auto& resource_base : resources)
  //{
  //  managed_image image;
  //  image.resource =
  //    std::static_pointer_cast<resource_db::image>(resource_base);
  //  image.image = _device->create_image(vk::image_create_info(
  //    /* flags */ vk::image_create_flag::sparse_binding_bit |
  //      vk::image_create_flag::sparse_residency_bit |
  //      vk::image_create_flag::sparse_aliased_bit,
  //    /* image_type */ vk::image_type::_2d,
  //    /* format */ device_image_format(
  //      format_from_resource(image.resource->format)),
  //    /* extent */ vk::extent_3d{image.resource->mipmaps[0].width,
  //                               image.resource->mipmaps[0].height,
  //                               image.resource->mipmaps[0].depth},
  //    /* mip_levels */ image.resource->mipmaps.size(),
  //    /* array_layers */ image.resource->array_element_count,
  //    /* samples */ vk::sample_count_flag::_1_bit,
  //    /* tiling */ vk::image_tiling::optimal,
  //    /* usage */ vk::image_usage_flag::transfer_dst_bit |
  //      vk::image_usage_flag::sampled_bit,
  //    /* sharing_mode */ vk::sharing_mode::exclusive,
  //    /* queue_family_index_count */ 0,
  //    /* queue_family_indices */ nullptr,
  //    /* initial_layout */ vk::image_layout::undefined));
  //}
  _staging_buffer->flush_memory();
}

void resource_streamer::destroy_resources()
{
  /// ToDo: Make sure that the buffer is not used any more!
  _staging_memory = nullptr;
  _staging_buffer.reset();
}
}
