#include "shift/render/vk/layer2/texture.hpp"
#include "shift/render/vk/layer2/material.hpp"
#include "shift/render/vk/layer1/command_buffer.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/utility.hpp"
#include "shift/render/vk/shared.hpp"

namespace shift::render::vk
{
texture::~texture() noexcept = default;
}

namespace shift::render::vk::layer2
{
texture::texture(vk::layer1::device& device, vk::format image_format,
                 vk::extent_3d extent, std::uint32_t mip_levels,
                 std::uint32_t array_layers,
                 std::shared_ptr<resource::image> source)
: _device(&device),
  _array_layers(array_layers),
  _source(std::move(source)),
  _min_valid_mip_level(mip_levels)
{
  _image = vk::make_framed_shared<vk::layer1::image>(
    *_device, vk::image_create_info(
                /* next */ nullptr,
                /* flags */ vk::image_create_flag::none,
                /* image_type */ vk::image_type::_2d,
                /* format */ image_format,
                /* extent */ extent,
                /* mip_levels */ mip_levels,
                /* array_layers */ array_layers,
                /* samples */ vk::sample_count_flag::_1_bit,
                /* tiling */ vk::image_tiling::optimal,
                /* usage */ vk::image_usage_flag::transfer_dst_bit |
                  vk::image_usage_flag::sampled_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr,
                /* initial_layout */ vk::image_layout::undefined));
  /// ToDo: Don't allocate all texture memory now but only when it's used!
  _image->allocate_storage(memory_pool::resource_images);
  _image->bind_storage();

  // Setup buffer copy regions for each mip level
  _buffer_copy_regions.reserve(mip_levels);
  auto image_format_pixel_size = vk::pixel_size(image_format);
  std::uint64_t staging_buffer_size = 0;
  auto mip_extent = extent;
  for (auto mip_level = 0u; mip_level < mip_levels; ++mip_level)
  {
    /// ToDo: Fill array with data.
    _buffer_copy_regions.emplace_back(vk::buffer_image_copy(
      /* buffer_offset */ staging_buffer_size,
      /* buffer_row_length */ 0,
      /* buffer_image_height */ 0,
      /* image_subresource */
      vk::image_subresource_layers(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* mip_level */ mip_level,
        /* base_array_layer */ 0,
        /* layer_count */ 1),
      /* image_offset */ vk::offset_3d{0, 0, 0},
      /* image_extent */ mip_extent));

    staging_buffer_size += mip_extent.width() * mip_extent.height() *
                           mip_extent.depth() * image_format_pixel_size;
    mip_extent.width(std::max(mip_extent.width() >> 1u, 1u));
    mip_extent.height(std::max(mip_extent.height() >> 1u, 1u));
    mip_extent.depth(std::max(mip_extent.depth() >> 1u, 1u));
  }

  // create staging buffer in shared memory.
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

  // _image_view = _device->create_image_view(
  //  vk::image_view_create_info(
  //    /* next */ nullptr,
  //    /* flags */ 0,
  //    /* image */ image->image->handle(),
  //    /* view_type */ vk::image_view_type::_2d,
  //    /* format */ image_format,
  //    /* components */
  //    vk::component_mapping{
  //      vk::component_swizzle::r, vk::component_swizzle::g,
  //      vk::component_swizzle::b, vk::component_swizzle::a},
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */
  //      static_cast<std::uint32_t>(image->resource_image->mipmaps.size()),
  //      /* base_array_layer */ 0,
  //      /* layer_count */ image->resource_image->array_layers)),
  //  *image->image);
}

texture::~texture() noexcept
{
}

const std::shared_ptr<resource::image>& texture::source_image() const
{
  return _source;
}

std::uint32_t texture::mip_levels() noexcept
{
  return static_cast<std::uint32_t>(_buffer_copy_regions.size());
}

std::uint32_t texture::array_layers() noexcept
{
  return _array_layers;
}

void texture::copy_to_staging_buffer(std::uint32_t mip_level_begin,
                                     std::uint32_t mip_level_end)
{
  BOOST_ASSERT(_source != nullptr);

  // Use a staging buffer to copy linear texture data to optimized texture
  // memory.
  vk::copy_image_to_staging_buffer(*_source, *_staging_buffer, _image->format(),
                                   mip_level_begin, mip_level_end);
}

void texture::copy_to_device_memory(vk::layer1::command_buffer& command_buffer,
                                    std::uint32_t mip_level_begin,
                                    std::uint32_t mip_level_end)
{
  BOOST_ASSERT(mip_level_begin < mip_level_end);
  BOOST_ASSERT(mip_level_begin < _image->mip_levels());
  BOOST_ASSERT(mip_level_end <= _image->mip_levels());
  auto mip_level_count = mip_level_end - mip_level_begin;

  command_buffer.copy_buffer_to_image(
    *_staging_buffer, *_image, vk::image_layout::transfer_dst_optimal,
    mip_level_count, &_buffer_copy_regions[mip_level_begin]);
}

std::uint32_t texture::min_valid_mip_level() const
{
  return _min_valid_mip_level;
}

void texture::min_valid_mip_level(std::uint32_t new_mip_level)
{
  _min_valid_mip_level = new_mip_level;
}
}
