#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/window_impl.hpp"
#include "shift/render/vk/pass_warp.hpp"
#include "shift/render/vk/pass_text.hpp"
#include <shift/render/vk/geometry_generator.hpp>
#include "shift/render/vk/layer2/view.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/layer1/instance.hpp"
#include <shift/resource/repository.hpp>
#include <shift/resource/shader.hpp>
#include <gsl/gsl>

#include <shift/log/log.hpp>

namespace shift::render::vk
{
using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace shift;

void renderer_impl::create_resources()
{
  // collect_resources(*scene.root, math::make_identity_matrix<4, 4, float>());

  //_default_context.begin();
  // for (auto* image : _scene_images)
  //{
  //  vk::format image_format =
  //    vk::format_from_resource(image->resource_image->format);
  //  /// Some textures have r8_g8_b8 format, which is not supported by Nvidia
  //  /// hardware.
  //  if (image_format == vk::format::r8_g8_b8_unorm)
  //    image_format = vk::format::r8_g8_b8_a8_unorm;
  //  else if (image_format == vk::format::r8_g8_b8_srgb)
  //    image_format = vk::format::r8_g8_b8_a8_srgb;

  //  image->image = vk::make_framed_shared<vk::layer1::image>(
  //    *_device,
  //    vk::image_create_info(
  //      /* next */ nullptr,
  //      /* flags */ vk::image_create_flag::none,
  //      /* image_type */ vk::image_type::_2d,
  //      /* format */ image_format,
  //      /* extent */
  //      vk::extent_3d{image->resource_image->mipmaps[0].width,
  //                    image->resource_image->mipmaps[0].height,
  //                    image->resource_image->mipmaps[0].depth},
  //      /* mip_levels */
  //      static_cast<std::uint32_t>(image->resource_image->mipmaps.size()),
  //      /* array_layers */ image->resource_image->array_element_count,
  //      /* samples */ vk::sample_count_flag::_1_bit,
  //      /* tiling */ vk::image_tiling::optimal,
  //      /* usage */ vk::image_usage_flag::transfer_dst_bit |
  //        vk::image_usage_flag::sampled_bit,
  //      /* sharing_mode */ vk::sharing_mode::exclusive,
  //      /* queue_family_index_count */ 0,
  //      /* queue_family_indices */ nullptr,
  //      /* initial_layout */ vk::image_layout::undefined));
  //  image->image->allocate_storage(memory_pool::resource_images);
  //  image->image->bind_storage();

  //  _default_context.command_buffer().pipeline_barrier(
  //    vk::pipeline_stage_flag::top_of_pipe_bit,
  //    vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
  //    vk::image_memory_barrier(
  //      nullptr,
  //      /* src_access_mask */ vk::access_flag::none,
  //      /* dst_access_mask */ vk::access_flag::transfer_write_bit,
  //      /* old_layout */ vk::image_layout::undefined,
  //      /* new_layout */ vk::image_layout::transfer_dst_optimal,
  //      /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //      /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //      /* image */ image->image->handle(),
  //      /* subresource_range */
  //      vk::image_subresource_range(
  //        /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //        /* base_mip_level */ 0,
  //        /* level_count */
  //        static_cast<std::uint32_t>(image->resource_image->mipmaps.size()),
  //        /* base_array_layer */ 0,
  //        /* layer_count */ 1)));

  //  // Use a staging buffer to copy linear texture data to optimized texture
  //  // memory.
  //  auto staging_buffer = vk::copy_image_to_staging_buffer(
  //    *_device, *image->resource_image, image_format);

  //  // Setup buffer copy regions for each mip level
  //  std::vector<vk::buffer_image_copy> buffer_copy_regions;
  //  std::size_t staging_buffer_offset = 0;
  //  std::uint32_t mipmap_level = 0;
  //  for (const auto& mipmap : image->resource_image->mipmaps)
  //  {
  //    buffer_copy_regions.push_back(vk::buffer_image_copy(
  //      /* buffer_offset */ staging_buffer_offset,
  //      /* buffer_row_length */ 0,
  //      /* buffer_image_height */ 0,
  //      /* image_subresource */
  //      vk::image_subresource_layers(
  //        /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //        /* mip_level */ mipmap_level,
  //        /* base_array_layer */ 0,
  //        /* layer_count */ 1),
  //      /* image_offset */ vk::offset_3d{0, 0, 0},
  //      /* image_extent */
  //      vk::extent_3d{mipmap.width, mipmap.height, mipmap.depth}));
  //    staging_buffer_offset += mipmap.width * mipmap.height * 4;
  //    ++mipmap_level;
  //  }

  //  _default_context.command_buffer().copy_buffer_to_image(
  //    *staging_buffer, *image->image, vk::image_layout::transfer_dst_optimal,
  //    buffer_copy_regions);

  //  _default_context.command_buffer().pipeline_barrier(
  //    vk::pipeline_stage_flag::transfer_bit,
  //    vk::pipeline_stage_flag::fragment_shader_bit, vk::dependency_flag::none,
  //    vk::image_memory_barrier(
  //      /* next */ nullptr,
  //      /* src_access_mask */ vk::access_flag::transfer_write_bit,
  //      /* dst_access_mask */ vk::access_flag::shader_read_bit |
  //        vk::access_flag::input_attachment_read_bit,
  //      /* old_layout */ vk::image_layout::transfer_dst_optimal,
  //      /* new_layout */ vk::image_layout::shader_read_only_optimal,
  //      /* src_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //      /* dst_queue_family_index */ VK_QUEUE_FAMILY_IGNORED,
  //      /* image */ image->image->handle(),
  //      /* subresource_range */
  //      vk::image_subresource_range(
  //        /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //        /* base_mip_level */ 0,
  //        /* level_count */
  //        static_cast<std::uint32_t>(image->resource_image->mipmaps.size()),
  //        /* base_array_layer */ 0,
  //        /* layer_count */ image->resource_image->array_element_count)));

  //  image->image_view = vk::make_framed_shared<vk::layer1::image_view>(
  //    *_device,
  //    vk::image_view_create_info(
  //      /* next */ nullptr,
  //      /* flags */ 0,
  //      /* image */ image->image->handle(),
  //      /* view_type */ vk::image_view_type::_2d,
  //      /* format */ image_format,
  //      /* components */
  //      vk::component_mapping{
  //        vk::component_swizzle::r, vk::component_swizzle::g,
  //        vk::component_swizzle::b, vk::component_swizzle::a},
  //      /* subresource_range */
  //      vk::image_subresource_range(
  //        /* aspect_mask */ vk::image_aspect_flag::color_bit,
  //        /* base_mip_level */ 0,
  //        /* level_count */
  //        static_cast<std::uint32_t>(image->resource_image->mipmaps.size()),
  //        /* base_array_layer */ 0,
  //        /* layer_count */ image->resource_image->array_element_count)),
  //    *image->image);
  //}

  // for (auto* model : _scene_models)
  //{
  //  model->instance_buffer = vk::make_framed_shared<vk::layer1::buffer>(
  //    *_device,
  //    vk::buffer_create_info(
  //      /* next */ nullptr,
  //      /* flags */ vk::buffer_create_flag::none,
  //      /* size */ model->instances.size() * sizeof(mesh_instance_data),
  //      /* usage */ vk::buffer_usage_flag::vertex_buffer_bit,
  //      /* sharing_mode */ vk::sharing_mode::exclusive,
  //      /* queue_family_index_count */ 0,
  //      /* queue_family_indices */ nullptr));
  //  model->instance_buffer->allocate_storage(memory_pool::pipeline_buffers);
  //  model->instance_buffer->bind_storage();

  //  std::memcpy(model->instance_buffer->address(0, 0),
  //  model->instances.data(),
  //              model->instances.size() * sizeof(mesh_instance_data));

  //  if (model->mesh->is_textured)
  //    _scene_models_textured.emplace_back(model);
  //  else
  //    _scene_models_untextured.emplace_back(model);
  //}

  for (auto& uniform_buffer : _scene_uniform_buffers)
  {
    uniform_buffer = vk::make_framed_shared<vk::layer1::buffer>(
      *_device, vk::buffer_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::buffer_create_flag::none,
                  /* size */ sizeof(mesh_uniform_data),
                  /* usage */ vk::buffer_usage_flag::uniform_buffer_bit,
                  /* sharing_mode */ vk::sharing_mode::exclusive,
                  /* queue_family_index_count */ 0,
                  /* queue_family_indices */ nullptr));
    uniform_buffer->allocate_storage(memory_pool::pipeline_buffers);
    uniform_buffer->bind_storage();
  }

  //// Create descriptor sets.
  // for (auto* model : _scene_models)
  //{
  //  for (std::uint32_t frame_index = 0u; frame_index < config::frame_count;
  //       ++frame_index)
  //  {
  //    auto& descriptor_set = gsl::at(model->descriptor_sets, frame_index);

  //    descriptor_set = vk::make_framed_shared<vk::layer1::descriptor_set>(
  //      *_device, _descriptor_pool, *_scene_descriptor_set_layout);

  //    // Prepare descriptor set.
  //    _device->update_descriptor_set(vk::write_descriptor_set(
  //      /* next */ nullptr,
  //      /* dst_set */ descriptor_set->handle(),
  //      /* dst_binding */ 0,
  //      /* dst_array_element */ 0,
  //      /* descriptor_count */ 1,
  //      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
  //      /* image_info */ nullptr,
  //      /* buffer_info */
  //      core::rvalue_address(vk::descriptor_buffer_info(
  //        /* buffer */ gsl::at(_scene_uniform_buffers, frame_index)->handle(),
  //        /* offset */ 0,
  //        /* range */ gsl::at(_scene_uniform_buffers, frame_index)->size())),
  //      /* texel_buffer_view */ nullptr));
  //    _device->update_descriptor_set(vk::write_descriptor_set(
  //      /* next */ nullptr,
  //      /* dst_set */ descriptor_set->handle(),
  //      /* dst_binding */ 1,
  //      /* dst_array_element */ 0,
  //      /* descriptor_count */ 1,
  //      /* descriptor_type */ vk::descriptor_type::sampled_image,
  //      /* image_info */
  //      core::rvalue_address(vk::descriptor_image_info(
  //        /* sampler */ nullptr,
  //        /* image_view */
  //        model->material->albedo.first->image_view->handle(),
  //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
  //      /* buffer_info */ nullptr,
  //      /* texel_buffer_view */ nullptr));
  //    _device->update_descriptor_set(vk::write_descriptor_set(
  //      /* next */ nullptr,
  //      /* dst_set */ descriptor_set->handle(),
  //      /* dst_binding */ 2,
  //      /* dst_array_element */ 0,
  //      /* descriptor_count */ 1,
  //      /* descriptor_type */ vk::descriptor_type::sampler,
  //      /* image_info */
  //      core::rvalue_address(vk::descriptor_image_info(
  //        /* sampler */ model->material->albedo.second->sampler->handle(),
  //        /* image_view */ nullptr,
  //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
  //      /* buffer_info */ nullptr,
  //      /* texel_buffer_view */ nullptr));
  //    _device->update_descriptor_set(vk::write_descriptor_set(
  //      /* next */ nullptr,
  //      /* dst_set */ descriptor_set->handle(),
  //      /* dst_binding */ 3,
  //      /* dst_array_element */ 0,
  //      /* descriptor_count */ 1,
  //      /* descriptor_type */ vk::descriptor_type::sampled_image,
  //      /* image_info */
  //      core::rvalue_address(vk::descriptor_image_info(
  //        /* sampler */ nullptr,
  //        /* image_view */
  //        model->material->normal.first->image_view->handle(),
  //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
  //      /* buffer_info */ nullptr,
  //      /* texel_buffer_view */ nullptr));
  //    _device->update_descriptor_set(vk::write_descriptor_set(
  //      /* next */ nullptr,
  //      /* dst_set */ descriptor_set->handle(),
  //      /* dst_binding */ 4,
  //      /* dst_array_element */ 0,
  //      /* descriptor_count */ 1,
  //      /* descriptor_type */ vk::descriptor_type::sampler,
  //      /* image_info */
  //      core::rvalue_address(vk::descriptor_image_info(
  //        /* sampler */ model->material->normal.second->sampler->handle(),
  //        /* image_view */ nullptr,
  //        /* image_layout */ vk::image_layout::shader_read_only_optimal)),
  //      /* buffer_info */ nullptr,
  //      /* texel_buffer_view */ nullptr));
  //  }
  //}

  // std::size_t instance_count = 0;
  // std::size_t sub_mesh_count = 0;
  // for (const auto* model : _scene_models)
  //{
  //  const auto* mesh = model->mesh->resource_mesh;
  //  instance_count += model->instances.size();
  //  sub_mesh_count += model->mesh->resource_mesh->sub_meshes.size();
  //  for (const auto& instance : model->instances)
  //  {
  //    _scene_bounding_boxes.instances.push_back(
  //      {instance.model *
  //       math::make_translation_matrix<4, 4, float>(mesh->bounding_box.center)
  //       * math::make_scale_matrix<4, 4>(
  //         math::make_vector_from(mesh->bounding_box.extent, 1.0f))});
  //  }
  //}
  // log::debug() << "rendering " << _scene_models.size() << " models.";
  // log::debug() << "rendering " << instance_count << " mesh instances.";
  // log::debug() << "rendering " << sub_mesh_count << " sub meshes.";

  // vk::box_generator generator;
  //// Create vertex, index, and uniform buffers.
  //_scene_bounding_boxes.vertex_buffer =
  //  vk::make_framed_shared<vk::layer1::buffer>(
  //    *_device, vk::buffer_create_info(
  //                /* next */ nullptr,
  //                /* flags */ vk::buffer_create_flag::none,
  //                /* size */ generator.vertex_count() *
  //                sizeof(vk::position_t),
  //                /* usage */ vk::buffer_usage_flag::vertex_buffer_bit,
  //                /* sharing_mode */ vk::sharing_mode::exclusive,
  //                /* queue_family_index_count */ 0,
  //                /* queue_family_indices */ nullptr));
  //_scene_bounding_boxes.vertex_buffer->allocate_storage(
  //  memory_pool::pipeline_buffers);
  //_scene_bounding_boxes.vertex_buffer->bind_storage();

  //_scene_bounding_boxes.index_buffer =
  //  vk::make_framed_shared<vk::layer1::buffer>(
  //    *_device, vk::buffer_create_info(
  //                /* next */ nullptr,
  //                /* flags */ vk::buffer_create_flag::none,
  //                /* size */ generator.index_count() * sizeof(vk::index_t),
  //                /* usage */ vk::buffer_usage_flag::index_buffer_bit,
  //                /* sharing_mode */ vk::sharing_mode::exclusive,
  //                /* queue_family_index_count */ 0,
  //                /* queue_family_indices */ nullptr));
  //_scene_bounding_boxes.index_buffer->allocate_storage(
  //  memory_pool::pipeline_buffers);
  //_scene_bounding_boxes.index_buffer->bind_storage();

  // generator.generate(static_cast<vk::position_t*>(
  //                     _scene_bounding_boxes.vertex_buffer->address(0, 0)),
  //                   nullptr, nullptr,
  //                   static_cast<vk::index_t*>(
  //                     _scene_bounding_boxes.index_buffer->address(0, 0)));

  //_scene_bounding_boxes.instance_buffer =
  //  vk::make_framed_shared<vk::layer1::buffer>(
  //    *_device, vk::buffer_create_info(
  //                /* next */ nullptr,
  //                /* flags */ vk::buffer_create_flag::none,
  //                /* size */ _scene_bounding_boxes.instances.size() *
  //                  sizeof(mesh_instance_data),
  //                /* usage */ vk::buffer_usage_flag::vertex_buffer_bit,
  //                /* sharing_mode */ vk::sharing_mode::exclusive,
  //                /* queue_family_index_count */ 0,
  //                /* queue_family_indices */ nullptr));
  //_scene_bounding_boxes.instance_buffer->allocate_storage(
  //  memory_pool::pipeline_buffers);
  //_scene_bounding_boxes.instance_buffer->bind_storage();

  // std::memcpy(
  //  _scene_bounding_boxes.instance_buffer->address(0, 0),
  //  _scene_bounding_boxes.instances.data(),
  //  _scene_bounding_boxes.instances.size() * sizeof(mesh_instance_data));

  // for (std::uint32_t frame_index = 0u; frame_index < config::frame_count;
  //     ++frame_index)
  //{
  //  auto& descriptor_set =
  //    gsl::at(_scene_bounding_boxes.descriptor_sets, frame_index);
  //  descriptor_set = vk::make_framed_shared<vk::layer1::descriptor_set>(
  //    *_device, _descriptor_pool, *_scene_descriptor_set_layout);

  //  // Prepare descriptor set.
  //  _device->update_descriptor_set(vk::write_descriptor_set(
  //    /* next */ nullptr,
  //    /* dst_set */ descriptor_set->handle(),
  //    /* dst_binding */ 0,
  //    /* dst_array_element */ 0,
  //    /* descriptor_count */ 1,
  //    /* descriptor_type */ vk::descriptor_type::uniform_buffer,
  //    /* image_info */ nullptr,
  //    /* buffer_info */
  //    core::rvalue_address(vk::descriptor_buffer_info(
  //      /* buffer */ gsl::at(_scene_uniform_buffers, frame_index)->handle(),
  //      /* offset */ 0,
  //      /* range */ gsl::at(_scene_uniform_buffers, frame_index)->size())),
  //    /* texel_buffer_view */ nullptr));
  //}

  //_default_context.command_buffer().pipeline_barrier(
  //  vk::pipeline_stage_flag::host_bit,
  //  vk::pipeline_stage_flag::vertex_input_bit, vk::dependency_flag::none,
  //  vk::buffer_memory_barrier(
  //    /* next */ nullptr,
  //    /* src_access_mask */ vk::access_flag::host_write_bit,
  //    /* dst_access_mask */ vk::access_flag::vertex_attribute_read_bit,
  //    /* src_queue_family_index */ _device->graphics_queue_family_index(),
  //    /* dst_queue_family_index */ _device->graphics_queue_family_index(),
  //    /* buffer */ _scene_bounding_boxes.vertex_buffer->handle(),
  //    /* offset */ 0,
  //    /* size */ VK_WHOLE_SIZE));

  //_default_context.command_buffer().pipeline_barrier(
  //  vk::pipeline_stage_flag::host_bit,
  //  vk::pipeline_stage_flag::vertex_input_bit, vk::dependency_flag::none,
  //  vk::buffer_memory_barrier(
  //    /* next */ nullptr,
  //    /* src_access_mask */ vk::access_flag::host_write_bit,
  //    /* dst_access_mask */ vk::access_flag::index_read_bit,
  //    /* src_queue_family_index */ _device->graphics_queue_family_index(),
  //    /* dst_queue_family_index */ _device->graphics_queue_family_index(),
  //    /* buffer */ _scene_bounding_boxes.index_buffer->handle(),
  //    /* offset */ 0,
  //    /* size */ VK_WHOLE_SIZE));
  // if (_enable_text)
  //  _pass_text->create_scene_resources(scene);
  //// if (_enable_warping)
  ////  _pass_warp->create_resources(scene);

  //_default_context.end();
  //_device->wait_idle();
}

void renderer_impl::destroy_resources()
{
  //// Wait until all work has been processed so we can safely destroy stuff.
  //_device->wait_idle();
  // for (std::uint32_t swapchain_index = 0; swapchain_index <
  // _swapchain_length;
  //     ++swapchain_index)
  //{
  //  gsl::at(_submitted, swapchain_index) = false;
  //}

  // for (auto& descriptor_set : _scene_bounding_boxes.descriptor_sets)
  //  descriptor_set.reset();
  //_scene_bounding_boxes.instance_buffer.reset();
  //_scene_bounding_boxes.index_buffer.reset();
  //_scene_bounding_boxes.vertex_buffer.reset();
  //_scene_bounding_boxes.instances.clear();

  for (auto& uniform_buffer : _scene_uniform_buffers)
    uniform_buffer.reset();

  //// if (_enable_warping)
  ////  _pass_warp->destroy_resources();
  // if (_enable_text)
  //  _pass_text->destroy_scene_resources();

  //_scene_models_untextured.clear();
  //_scene_models_textured.clear();
  //_scene_models.clear();
  //_scene_meshes.clear();
  //_scene_materials.clear();
  //_scene_samplers.clear();
  //_scene_images.clear();
  //_scene_model_pool.clear();
  //_scene_mesh_pool.clear();
  //_scene_material_pool.clear();
  //_scene_sampler_pool.clear();
  //_scene_image_pool.clear();
}
}
