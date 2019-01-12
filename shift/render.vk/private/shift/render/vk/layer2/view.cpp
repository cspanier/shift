#include "shift/render/vk/layer2/view.hpp"
#include "shift/render/vk/pass.hpp"
#include "shift/render/vk/renderer_impl.hpp"
// #include "shift/render/vk/application.hpp"
#include "shift/render/vk/context_impl.hpp"
#include "shift/render/vk/layer1/device.hpp"

namespace shift::render::vk::layer2
{
void multiview::create_frame_data(math::vector2<std::uint32_t> absolute_size,
                                  vk::context_impl& context)
{
  absolute_size = (absolute_size + math::make_vector_from(3u, 3u)) / 4 * 4;
  /// ToDo: Improve error handling.
  BOOST_ASSERT(absolute_size.x % 4 == 0 && absolute_size.y % 4 == 0);
  if (absolute_size.x % 4 != 0 || absolute_size.y % 4 != 0)
  {
    BOOST_THROW_EXCEPTION(core::logic_error());
  }

  auto* device = renderer_impl::singleton_instance().device();

  prepared = vk::layer1::semaphore(*device);
  //// Create multisample render target image and image view.
  // multisample_image =
  // device->create_image(vk::image_create_info(
  //  /* flags */ vk::image_create_flag::none,
  //  /* image_type */ vk::image_type::_2d,
  //  /* format */ vk::format::b8_g8_r8_a8_unorm,
  //  /* extent */ vk::extent_3d{absolute_size.width, absolute_size.height,
  //  1u},
  //  /* mip_levels */ 1,
  //  /* array_layers */ 1,
  //  /* samples */ samples,
  //  /* tiling */ vk::image_tiling::optimal,
  //  /* usage */ vk::image_usage_flag::transfer_src_bit |
  //    vk::image_usage_flag::color_attachment_bit,
  //  /* sharing_mode */ vk::sharing_mode::exclusive,
  //  /* queue_family_index_count */ 0,
  //  /* queue_family_indices */ nullptr,
  //  /* initial_layout */ vk::image_layout::undefined));

  // multisample_image->allocate_storage(
  //  vk::memory_property_flag::device_local_bit);
  // multisample_image->bind_storage();

  // context.command_buffer().pipeline_barrier(
  //  vk::pipeline_stage_flag::top_of_pipe_bit,
  //  vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
  //  vk::image_memory_barrier(
  //    /* src_access_mask */ vk::access_flag::none,
  //    /* dst_access_mask */ vk::access_flag::transfer_read_bit,
  //    /* old_layout */ vk::image_layout::undefined,
  //    /* new_layout */ vk::image_layout::transfer_src_optimal,
  //    /* src_queue_family_index */
  //    device->graphics_queue_family_index(),
  //    /* dst_queue_family_index */
  //    device->graphics_queue_family_index(),
  //    /* image */ multisample_image->handle(),
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)));

  // multisample_image_view = device->create_image_view(
  //  vk::image_view_create_info(
  //    /* flags */ 0,
  //    /* image */ multisample_image->handle(),
  //    /* view_type */ vk::image_view_type::_2d,
  //    /* format */ vk::format::b8_g8_r8_a8_unorm,
  //    /* components */
  //    vk::component_mapping{
  //      vk::component_swizzle::identity, vk::component_swizzle::identity,
  //      vk::component_swizzle::identity, vk::component_swizzle::identity},
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)),
  //  *multisample_image);

  // Create scene render target image and image view.
  scene_image = vk::make_framed_shared<vk::layer1::image>(
    *device, vk::image_create_info(
               /* next */ nullptr,
               /* flags */ vk::image_create_flag::none,
               /* image_type */ vk::image_type::_2d,
               /* format */ vk::format::b8_g8_r8_a8_unorm,
               /* extent */
               vk::extent_3d{absolute_size.x, absolute_size.y, 1u},
               /* mip_levels */ 1,
               /* array_layers */ 1,
               /* samples */ vk::sample_count_flag::_1_bit,
               /* tiling */ vk::image_tiling::optimal,
               /* usage */ vk::image_usage_flag::transfer_src_bit |
                 vk::image_usage_flag::color_attachment_bit,
               /* sharing_mode */ vk::sharing_mode::exclusive,
               /* queue_family_index_count */ 0,
               /* queue_family_indices */ nullptr,
               /* initial_layout */ vk::image_layout::undefined));

  scene_image->allocate_storage(memory_pool::pipeline_images);
  scene_image->bind_storage();

  context.command_buffer().pipeline_barrier(
    vk::pipeline_stage_flag::top_of_pipe_bit,
    vk::pipeline_stage_flag::color_attachment_output_bit,
    vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::none,
      /* dst_access_mask */ vk::access_flag::color_attachment_read_bit |
        vk::access_flag::color_attachment_write_bit,
      /* old_layout */ vk::image_layout::undefined,
      /* new_layout */ vk::image_layout::color_attachment_optimal,
      /* src_queue_family_index */
      device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      device->graphics_queue_family_index(),
      /* image */ scene_image->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)));

  scene_image_view = vk::make_framed_shared<vk::layer1::image_view>(
    *device,
    vk::image_view_create_info(
      /* next */ nullptr,
      /* flags */ vk::image_view_create_flag::none,
      /* image */ scene_image->handle(),
      /* view_type */ vk::image_view_type::_2d,
      /* format */ vk::format::b8_g8_r8_a8_unorm,
      /* components */
      vk::component_mapping{
        vk::component_swizzle::identity, vk::component_swizzle::identity,
        vk::component_swizzle::identity, vk::component_swizzle::identity},
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)),
    *scene_image);

  // multisample_depth_image =
  //  device->create_image(vk::image_create_info(
  //    /* flags */ vk::image_create_flag::none,
  //    /* image_type */ vk::image_type::_2d,
  //    /* format */ config::depth_format,
  //    /* extent */ vk::extent_3d{absolute_size.width, absolute_size.height,
  //    1u},
  //    /* mip_levels */ 1,
  //    /* array_layers */ 1,
  //    /* samples */ samples,
  //    /* tiling */ vk::image_tiling::optimal,
  //    /* usage */ vk::image_usage_flag::depth_stencil_attachment_bit,
  //    /* sharing_mode */ vk::sharing_mode::exclusive,
  //    /* queue_family_index_count */ 0,
  //    /* queue_family_indices */ nullptr,
  //    /* initial_layout */ vk::image_layout::undefined));
  // multisample_depth_image->allocate_storage(
  //  vk::memory_property_flag::device_local_bit);
  // multisample_depth_image->bind_storage();

  // context.command_buffer().pipeline_barrier(
  //  vk::pipeline_stage_flag::top_of_pipe_bit,
  //  vk::pipeline_stage_flag::late_fragment_tests_bit,
  //  vk::dependency_flag::none, vk::image_memory_barrier(
  //    /* src_access_mask */ vk::access_flag::none,
  //    /* dst_access_mask */
  //    vk::access_flag::depth_stencil_attachment_write_bit,
  //    /* old_layout */ vk::image_layout::undefined,
  //    /* new_layout */ vk::image_layout::depth_stencil_attachment_optimal,
  //    /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //    /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //    /* image */ multisample_depth_image->handle(),
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::depth_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)));

  // multisample_depth_image_view =
  // device->create_image_view(
  //  vk::image_view_create_info(
  //    /* flags */ 0,
  //    /* image */ multisample_depth_image->handle(),
  //    /* view_type */ vk::image_view_type::_2d,
  //    /* format */ config::depth_format,
  //    /* components */
  //    vk::component_mapping{
  //      vk::component_swizzle::identity, vk::component_swizzle::identity,
  //      vk::component_swizzle::identity, vk::component_swizzle::identity},
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::depth_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)),
  //  *multisample_depth_image);

  depth_image = vk::make_framed_shared<vk::layer1::image>(
    *device, vk::image_create_info(
               /* next */ nullptr,
               /* flags */ vk::image_create_flag::none,
               /* image_type */ vk::image_type::_2d,
               /* format */ config::depth_format,
               /* extent */
               vk::extent_3d{absolute_size.x, absolute_size.y, 1u},
               /* mip_levels */ 1,
               /* array_layers */ 1,
               /* samples */ vk::sample_count_flag::_1_bit,
               /* tiling */ vk::image_tiling::optimal,
               /* usage */ vk::image_usage_flag::depth_stencil_attachment_bit |
                 vk::image_usage_flag::sampled_bit,
               /* sharing_mode */ vk::sharing_mode::exclusive,
               /* queue_family_index_count */ 0,
               /* queue_family_indices */ nullptr,
               /* initial_layout */ vk::image_layout::undefined));
  depth_image->allocate_storage(memory_pool::pipeline_images);
  depth_image->bind_storage();

  context.command_buffer().pipeline_barrier(
    vk::pipeline_stage_flag::top_of_pipe_bit,
    vk::pipeline_stage_flag::late_fragment_tests_bit, vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::none,
      /* dst_access_mask */
      vk::access_flag::depth_stencil_attachment_write_bit,
      /* old_layout */ vk::image_layout::undefined,
      /* new_layout */ vk::image_layout::depth_stencil_attachment_optimal,
      /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
      /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
      /* image */ depth_image->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::depth_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)));

  depth_image_view = vk::make_framed_shared<vk::layer1::image_view>(
    *device,
    vk::image_view_create_info(
      /* next */ nullptr,
      /* flags */ vk::image_view_create_flag::none,
      /* image */ depth_image->handle(),
      /* view_type */ vk::image_view_type::_2d,
      /* format */ config::depth_format,
      /* components */
      vk::component_mapping{
        vk::component_swizzle::identity, vk::component_swizzle::identity,
        vk::component_swizzle::identity, vk::component_swizzle::identity},
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::depth_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)),
    *depth_image);

  // Create r32ui image to store 4x4 down-scaled reprojected depth.
  reproject_depth.r32ui_image = vk::make_framed_shared<vk::layer1::image>(
    *device, vk::image_create_info(
               /* next */ nullptr,
               /* flags */ vk::image_create_flag::none,
               /* image_type */ vk::image_type::_2d,
               /* format */ vk::format::r32_uint,
               /* extent */
               vk::extent_3d{absolute_size.x / 4, absolute_size.y / 4, 1u},
               /* mip_levels */ 1,
               /* array_layers */ 1,
               /* samples */ vk::sample_count_flag::_1_bit,
               /* tiling */ vk::image_tiling::optimal,
               /* usage */ vk::image_usage_flag::storage_bit |
                 vk::image_usage_flag::transfer_dst_bit,
               /* sharing_mode */ vk::sharing_mode::exclusive,
               /* queue_family_index_count */ 0,
               /* queue_family_indices */ nullptr,
               /* initial_layout */ vk::image_layout::undefined));

  reproject_depth.r32ui_image->allocate_storage(memory_pool::pipeline_images);
  reproject_depth.r32ui_image->bind_storage();

  context.command_buffer().pipeline_barrier(
    vk::pipeline_stage_flag::top_of_pipe_bit,
    vk::pipeline_stage_flag::compute_shader_bit, vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::none,
      /* dst_access_mask */ vk::access_flag::shader_write_bit,
      /* old_layout */ vk::image_layout::undefined,
      /* new_layout */ vk::image_layout::general,  // transfer_dst_optimal,
      /* src_queue_family_index */
      device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      device->graphics_queue_family_index(),
      /* image */ reproject_depth.r32ui_image->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)));

  reproject_depth.r32ui_image_view =
    vk::make_framed_shared<vk::layer1::image_view>(
      *device,
      vk::image_view_create_info(
        /* next */ nullptr,
        /* flags */ vk::image_view_create_flag::none,
        /* image */ reproject_depth.r32ui_image->handle(),
        /* view_type */ vk::image_view_type::_2d,
        /* format */ vk::format::r32_uint,
        /* components */
        vk::component_mapping{
          vk::component_swizzle::identity, vk::component_swizzle::zero,
          vk::component_swizzle::zero, vk::component_swizzle::zero},
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::color_bit,
          /* base_mip_level */ 0,
          /* level_count */ 1,
          /* base_array_layer */ 0,
          /* layer_count */ 1)),
      *reproject_depth.r32ui_image);

  // Create r8g8b8a8 image to visualize reproject_depth.r32ui_image.
  reproject_depth.debug_image = vk::make_framed_shared<vk::layer1::image>(
    *device, vk::image_create_info(
               /* next */ nullptr,
               /* flags */ vk::image_create_flag::none,
               /* image_type */ vk::image_type::_2d,
               /* format */ vk::format::b8_g8_r8_a8_unorm,
               /* extent */
               vk::extent_3d{absolute_size.x / 4, absolute_size.y / 4, 1u},
               /* mip_levels */ 1,
               /* array_layers */ 1,
               /* samples */ vk::sample_count_flag::_1_bit,
               /* tiling */ vk::image_tiling::optimal,
               /* usage */ vk::image_usage_flag::storage_bit |
                 vk::image_usage_flag::transfer_dst_bit,
               /* sharing_mode */ vk::sharing_mode::exclusive,
               /* queue_family_index_count */ 0,
               /* queue_family_indices */ nullptr,
               /* initial_layout */ vk::image_layout::undefined));

  reproject_depth.debug_image->allocate_storage(memory_pool::pipeline_images);
  reproject_depth.debug_image->bind_storage();

  context.command_buffer().pipeline_barrier(
    vk::pipeline_stage_flag::top_of_pipe_bit,
    vk::pipeline_stage_flag::compute_shader_bit, vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::none,
      /* dst_access_mask */ vk::access_flag::shader_write_bit,
      /* old_layout */ vk::image_layout::undefined,
      /* new_layout */ vk::image_layout::general,
      /* src_queue_family_index */
      device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      device->graphics_queue_family_index(),
      /* image */ reproject_depth.debug_image->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)));

  // reproject_depth.debug_image_view =
  // vk::make_framed_shared<vk::layer1::image_view>(
  // *device,
  //  vk::image_view_create_info(
  //    /* flags */ 0,
  //    /* image */ reproject_depth.debug_image->handle(),
  //    /* view_type */ vk::image_view_type::_2d,
  //    /* format */ vk::format::b8_g8_r8_a8_unorm,
  //    /* components */
  //    vk::component_mapping{
  //      vk::component_swizzle::identity, vk::component_swizzle::zero,
  //      vk::component_swizzle::zero, vk::component_swizzle::zero},
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)),
  //  *reproject_depth.debug_image);

  // Create final depth image.
  reproject_depth.depth_image = vk::make_framed_shared<vk::layer1::image>(
    *device, vk::image_create_info(
               /* next */ nullptr,
               /* flags */ vk::image_create_flag::none,
               /* image_type */ vk::image_type::_2d,
               /* format */ vk::format::b8_g8_r8_a8_unorm,
               /* extent */
               vk::extent_3d{absolute_size.x / 4, absolute_size.y / 4, 1u},
               /* mip_levels */ 1,
               /* array_layers */ 1,
               /* samples */ vk::sample_count_flag::_1_bit,
               /* tiling */ vk::image_tiling::optimal,
               /* usage */ vk::image_usage_flag::storage_bit |
                 vk::image_usage_flag::transfer_dst_bit,
               /* sharing_mode */ vk::sharing_mode::exclusive,
               /* queue_family_index_count */ 0,
               /* queue_family_indices */ nullptr,
               /* initial_layout */ vk::image_layout::undefined));

  reproject_depth.depth_image->allocate_storage(memory_pool::pipeline_images);
  reproject_depth.depth_image->bind_storage();

  context.command_buffer().pipeline_barrier(
    vk::pipeline_stage_flag::top_of_pipe_bit,
    vk::pipeline_stage_flag::compute_shader_bit, vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::none,
      /* dst_access_mask */ vk::access_flag::shader_write_bit,
      /* old_layout */ vk::image_layout::undefined,
      /* new_layout */ vk::image_layout::general,
      /* src_queue_family_index */
      device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      device->graphics_queue_family_index(),
      /* image */ reproject_depth.depth_image->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0,
        /* level_count */ 1,
        /* base_array_layer */ 0,
        /* layer_count */ 1)));

  // reproject_depth.depth_image_view = device->create_image_view(
  //  vk::image_view_create_info(
  //    /* flags */ 0,
  //    /* image */ reproject_depth.depth_image->handle(),
  //    /* view_type */ vk::image_view_type::_2d,
  //    /* format */ vk::format::b8_g8_r8_a8_unorm,
  //    /* components */
  //    vk::component_mapping{
  //      vk::component_swizzle::identity, vk::component_swizzle::zero,
  //      vk::component_swizzle::zero, vk::component_swizzle::zero},
  //    /* subresource_range */
  //    vk::image_subresource_range(
  //      /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //      /* base_mip_level */ 0,
  //      /* level_count */ 1,
  //      /* base_array_layer */ 0,
  //      /* layer_count */ 1)),
  //  *reproject_depth.depth_image);

  reproject_depth.finished = vk::layer1::semaphore(*device);
}

void multiview::destroy_frame_data()
{
  reproject_depth.finished.reset();
  // reproject_depth.depth_image_view.reset();
  reproject_depth.depth_image.reset();
  // reproject_depth.debug_image_view.reset();
  reproject_depth.debug_image.reset();
  reproject_depth.r32ui_image_view.reset();
  reproject_depth.r32ui_image.reset();
  reproject_depth.uniform_buffer.reset();
  reproject_depth.descriptor_set.reset();
  reproject_depth.command_buffer.reset();

  depth_image_view.reset();
  depth_image.reset();
  // multisample_depth_image_view.reset();
  // multisample_depth_image.reset();
  scene_image_view.reset();
  scene_image.reset();
  // multisample_image_view.reset();
  // multisample_image.reset();
  prepared.reset();
}
}
