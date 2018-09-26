#include "shift/render/vk/pass_warp.h"
#include <shift/render/vk/renderer.h>
#include "shift/render/vk/layer1/shader_module.h"
#include "shift/render/vk/layer1/device.h"
#include <shift/resource/repository.h>
#include <shift/resource/shader.h>
#include <shift/core/exception.h>
#include <shift/core/algorithm.h>
#include <boost/filesystem.hpp>

namespace shift::render::vk
{
pass_warp::pass_warp(vk::layer1::device& device) : pass(device)
{
}

void pass_warp::create_resource_descriptions()
{
}

void pass_warp::destroy_resource_descriptions()
{
}

void pass_warp::create_pipeline(vk::layer1::pipeline_cache& pipeline_cache)
{
  auto& repository = resource::repository::singleton_instance();

  // Create descriptor set layout and pipeline layout.
  const auto warp_layout_bindings = core::make_array(
    vk::descriptor_set_layout_binding(
      /* binding */ 0,
      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
      /* descriptor_count */ 1,
      /* stage_flags */ vk::shader_stage_flag::vertex_bit,
      /* immutable_samplers */ nullptr),
    vk::descriptor_set_layout_binding(
      /* binding */ 1,
      /* descriptor_type */ vk::descriptor_type::combined_image_sampler,
      /* descriptor_count */ 1,
      /* stage_flags */ vk::shader_stage_flag::fragment_bit,
      /* immutable_samplers */ nullptr));

  _descriptor_set_layout.create(
    *_device, vk::descriptor_set_layout_create_info(
                /* next */ nullptr,
                /* flags */ vk::descriptor_set_layout_create_flag::none,
                /* binding_count */ warp_layout_bindings.size(),
                /* bindings */ warp_layout_bindings.data()));
  _pipeline_layout.create(*_device,
                          vk::pipeline_layout_create_info(
                            /* next */ nullptr,
                            /* flags */ vk::pipeline_layout_create_flag::none,
                            /* set_layout_count */ 1,
                            /* set_layouts */ &_descriptor_set_layout->handle(),
                            /* push_constant_range_count */ 0,
                            /* push_constant_ranges */ nullptr));

  // Create render pass.
  const auto attachment_descriptions =
    core::make_array(vk::attachment_description(
      /* flags */ vk::attachment_description_flag::none,
      /* format */ vk::format::b8_g8_r8_a8_srgb,
      /* samples */ vk::sample_count_flag::_1_bit,
      /* load_op */ vk::attachment_load_op::clear,
      /* store_op */ vk::attachment_store_op::store,
      /* stencil_load_op */ vk::attachment_load_op::dont_care,
      /* stencil_store_op */ vk::attachment_store_op::dont_care,
      /* initial_layout */ vk::image_layout::color_attachment_optimal,
      /* final_layout */ vk::image_layout::color_attachment_optimal));

  const auto color_references = core::make_array(vk::attachment_reference(
    /* attachment */ 0,
    /* layout */ vk::image_layout::color_attachment_optimal));

  const auto subpass_descriptions = core::make_array(vk::subpass_description(
    /* flags */ vk::subpass_description_flag::none,
    /* pipeline_bind_point */ vk::pipeline_bind_point::graphics,
    /* input_attachment_count */ 0,
    /* input_attachments */ nullptr,
    /* color_attachment_count */ color_references.size(),
    /* color_attachments */ color_references.data(),
    /* resolve_attachments */ nullptr,
    /* depth_stencil_attachment */ nullptr,
    /* preserve_attachment_count */ 0,
    /* preserve_attachments */ nullptr));

  _render_pass = vk::make_framed_shared<vk::layer1::render_pass>(
    *_device, vk::render_pass_create_info(
                /* next */ nullptr,
                /* flags */ vk::render_pass_create_flag::none,
                /* attachment_count */ attachment_descriptions.size(),
                /* attachments */ attachment_descriptions.data(),
                /* subpass_count */ subpass_descriptions.size(),
                /* subpasses */ subpass_descriptions.data(),
                /* dependency_count */ 0,
                /* dependencies */ nullptr));

  // Pass viewport and scissor rects dynamically to command buffer.
  const auto dynamic_states =
    core::make_array(vk::dynamic_state::viewport, vk::dynamic_state::scissor);
  const auto dynamic_state = vk::pipeline_dynamic_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_dynamic_state_create_flag::none,
    /* dynamic_state_count */ dynamic_states.size(),
    /* dynamic_states */ dynamic_states.data());

  auto vertex_shader_resource =
    repository.load<resource::shader>("public/shaders/warp.vert.spv");
  auto tess_ctrl_shader_resource =
    repository.load<resource::shader>("public/shaders/warp.tesc.spv");
  auto tess_eval_shader_resource =
    repository.load<resource::shader>("public/shaders/warp.tese.spv");
  auto geometry_shader_resource =
    repository.load<resource::shader>("public/shaders/warp.geom.spv");
  auto fragment_shader_resource =
    repository.load<resource::shader>("public/shaders/warp.frag.spv");
  if (!vertex_shader_resource || !tess_ctrl_shader_resource ||
      !tess_eval_shader_resource || !geometry_shader_resource ||
      !fragment_shader_resource)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Missing shader resource"));
  }

  core::stack_ptr<vk::layer1::shader_module> vertex_shader;
  vertex_shader.create(
    *_device, vk::shader_module_create_info(
                /* next */ nullptr,
                /* flags */ 0,
                /* code_size */ vertex_shader_resource->storage.size() * 4,
                /* code */ vertex_shader_resource->storage.data()));
  core::stack_ptr<vk::layer1::shader_module> tessellation_control_shader;
  tessellation_control_shader.create(
    *_device, vk::shader_module_create_info(
                /* next */ nullptr,
                /* flags */ 0,
                /* code_size */ tess_ctrl_shader_resource->storage.size() * 4,
                /* code */ tess_ctrl_shader_resource->storage.data()));
  core::stack_ptr<vk::layer1::shader_module> tessellation_evaluation_shader;
  tessellation_evaluation_shader.create(
    *_device, vk::shader_module_create_info(
                /* next */ nullptr,
                /* flags */ 0,
                /* code_size */ tess_eval_shader_resource->storage.size() * 4,
                /* code */ tess_eval_shader_resource->storage.data()));
  core::stack_ptr<vk::layer1::shader_module> geometry_shader;
  geometry_shader.create(
    *_device, vk::shader_module_create_info(
                /* next */ nullptr,
                /* flags */ 0,
                /* code_size */ geometry_shader_resource->storage.size() * 4,
                /* code */ geometry_shader_resource->storage.data()));
  core::stack_ptr<vk::layer1::shader_module> fragment_shader;
  fragment_shader.create(
    *_device, vk::shader_module_create_info(
                /* next */ nullptr,
                /* flags */ 0,
                /* code_size */ fragment_shader_resource->storage.size() * 4,
                /* code */ fragment_shader_resource->storage.data()));

  const auto shader_stages = core::make_array(
    vk::pipeline_shader_stage_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_shader_stage_create_flag::none,
      /* stage */ vk::shader_stage_flag::vertex_bit,
      /* module */ vertex_shader->handle(),
      /* name */ "main",
      /* specialization_info */ nullptr),
    vk::pipeline_shader_stage_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_shader_stage_create_flag::none,
      /* stage */ vk::shader_stage_flag::tessellation_control_bit,
      /* module */ tessellation_control_shader->handle(),
      /* name */ "main",
      /* specialization_info */ nullptr),
    vk::pipeline_shader_stage_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_shader_stage_create_flag::none,
      /* stage */ vk::shader_stage_flag::tessellation_evaluation_bit,
      /* module */ tessellation_evaluation_shader->handle(),
      /* name */ "main",
      /* specialization_info */ nullptr),
    vk::pipeline_shader_stage_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_shader_stage_create_flag::none,
      /* stage */ vk::shader_stage_flag::geometry_bit,
      /* module */ geometry_shader->handle(),
      /* name */ "main",
      /* specialization_info */ nullptr),
    vk::pipeline_shader_stage_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_shader_stage_create_flag::none,
      /* stage */ vk::shader_stage_flag::fragment_bit,
      /* module */ fragment_shader->handle(),
      /* name */ "main",
      /* specialization_info */ nullptr));

  const auto vertex_input_state = vk::pipeline_vertex_input_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_vertex_input_state_create_flag::none,
    /* vertex_binding_description_count */ 0,
    /* vertex_binding_descriptions */ nullptr,
    /* vertex_attribute_description_count */ 0,
    /* vertex_attribute_descriptions */ nullptr);

  const auto input_assembly_state =
    vk::pipeline_input_assembly_state_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_input_assembly_state_create_flag::none,
      /* topology */ vk::primitive_topology::patch_list,
      /* primitive_restart_enable */ VK_FALSE);

  const auto tessellation_state = vk::pipeline_tessellation_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_tessellation_state_create_flag::none,
    /* patch_control_points */ 16);

  const auto viewport_state = vk::pipeline_viewport_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_viewport_state_create_flag::none,
    /* viewport_count */ 1,  // Dynamic state.
    /* viewports */ nullptr,
    /* scissor_count */ 1,  // Dynamic state.
    /* scissors */ nullptr);

  const auto rasterization_state = vk::pipeline_rasterization_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_rasterization_state_create_flag::none,
    /* depth_clamp_enable */ VK_FALSE,
    /* rasterizer_discard_enable */ VK_FALSE,
    /* polygon_mode */ vk::polygon_mode::fill,
    /* cull_mode */ vk::cull_mode_flag::back_bit,
    /* front_face */ vk::front_face::counter_clockwise,
    /* depth_bias_enable */ VK_FALSE,
    /* depth_bias_constant_factor */ 0.0f,
    /* depth_bias_clamp */ 0.0f,
    /* depth_bias_slope_factor */ 0.0f,
    /* line_width */ 1.0f);

  const auto multisample_state = vk::pipeline_multisample_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_multisample_state_create_flag::none,
    /* rasterization_samples */ vk::sample_count_flag::_1_bit,
    /* sample_shading_enable */ VK_FALSE,
    /* min_sample_shading */ 0.0f,
    /* sample_mask */ nullptr,
    /* alpha_to_coverage_enable */ VK_FALSE,
    /* alpha_to_one_enable */ VK_FALSE);

  const auto color_blend_attachment_states =
    core::make_array(vk::pipeline_color_blend_attachment_state(
      /* blend_enable */ VK_FALSE,
      /* src_color_blend_factor */ vk::blend_factor::one,
      /* dst_color_blend_factor */ vk::blend_factor::zero,
      /* color_blend_op */ vk::blend_op::add,
      /* src_alpha_blend_factor */ vk::blend_factor::one,
      /* dst_alpha_blend_factor */ vk::blend_factor::zero,
      /* alpha_blend_op */ vk::blend_op::add,
      /* color_write_mask */ vk::color_component_flag::r_bit |
        vk::color_component_flag::g_bit | vk::color_component_flag::b_bit |
        vk::color_component_flag::a_bit));

  const auto color_blend_state = vk::pipeline_color_blend_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_color_blend_state_create_flag::none,
    /* logic_op_enable */ VK_FALSE,
    /* logic_op */ vk::logic_op::clear_op,
    /* attachment_count */ color_blend_attachment_states.size(),
    /* attachments */ color_blend_attachment_states.data(),
    /* blend_constants */ {0.0f, 0.0f, 0.0f, 0.0f});

  _pipeline = vk::make_framed_shared<vk::layer1::pipeline>(
    *_device, pipeline_cache,
    vk::graphics_pipeline_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_create_flag::none,
      /* stage_count */ shader_stages.size(),
      /* stages */ shader_stages.data(),
      /* vertex_input_state */ &vertex_input_state,
      /* input_assembly_state */ &input_assembly_state,
      /* tessellation_state */ &tessellation_state,
      /* viewport_state */ &viewport_state,
      /* rasterization_state */ &rasterization_state,
      /* multisample_state */ &multisample_state,
      /* depth_stencil_state */ nullptr,
      /* color_blend_state */ &color_blend_state,
      /* dynamic_state */ &dynamic_state,
      /* layout */ _pipeline_layout->handle(),
      /* render_pass */ _render_pass->handle(),
      /* subpass */ 0,
      /* base_pipeline_handle */ nullptr,
      /* base_pipeline_index */ 0));

  fragment_shader.reset();
  geometry_shader.reset();
  tessellation_evaluation_shader.reset();
  tessellation_control_shader.reset();
  vertex_shader.reset();

  _image_sampler = vk::make_framed_shared<vk::layer1::sampler>(
    *_device, vk::sampler_create_info(
                /* next */ nullptr,
                /* flags */ vk::sampler_create_flag::none,
                /* mag_filter */ vk::filter::linear,
                /* min_filter */ vk::filter::linear,
                /* mipmap_mode */ vk::sampler_mipmap_mode::nearest,
                /* address_mode_u */ vk::sampler_address_mode::repeat,
                /* address_mode_v */ vk::sampler_address_mode::repeat,
                /* address_mode_w */ vk::sampler_address_mode::repeat,
                /* mip_lod_bias */ 0.0f,
                /* anisotropy_enable */ VK_TRUE,
                /* max_anisotropy */ 1.0f,
                /* compare_enable */ VK_FALSE,
                /* compare_op */ vk::compare_op::never,
                /* min_lod */ 0.0f,
                /* max_lod */ 1.0f,
                /* border_color */ vk::border_color::float_transparent_black,
                /* unnormalized_coordinates */ VK_FALSE));
}

void pass_warp::destroy_pipeline()
{
  _image_sampler.reset();
  _pipeline.reset();
  _render_pass.reset();
  _pipeline_layout.reset();
  _descriptor_set_layout.reset();
}

void pass_warp::create_resources(resource::scene& /*scene*/)
{
  /// ToDo...
}

void pass_warp::destroy_resources()
{
  /// ToDo...
}

void pass_warp::update(std::uint32_t /*buffer_index*/)
{
}
}
//#include "shift/render/vk/render_passes/warp_instance.h"
//#include "shift/render/vk/render_passes/warp_schematic.h"
//#include "vk/cube_application.h"
//#include <shift/resource/repository.h>
//#include <shift/resource/shader.h>
//#include <shift/core/exception.h>
//#include <shift/core/algorithm.h>
//#include <shift/core/at_exit_scope.h>
//#include <boost/filesystem.hpp>
//#include <fstream>
//#include <chrono>
//
// namespace shift::render::vk::render_passes
//{
// render_pass_warp_instance::render_pass_warp_instance(
//  cube_application& application, vk::device& device,
//  render_pass_warp_class& technique)
//: render_pass_instance(application, device, true), _technique(technique)
//{
//}
//
// void render_pass_warp_instance::pre_update(std::uint32_t buffer_index,
//                                           std::chrono::milliseconds
//                                           frame_time)
//{
//  auto* mapped_data = static_cast<warp_uniform_data*>(
//    _uniform_buffer->address(0, sizeof(warp_uniform_data)));
//
//  for (unsigned int i = 0, patch = 0; patch < patches; ++patch)
//  {
//    for (unsigned int row = 0; row != rows; ++row)
//    {
//      for (unsigned int column = 0; column != columns; ++column, ++i)
//      {
//        mapped_data->vertices[i].position = math::make_vector_from(
//          (column + patch * (columns - 1.0f)) / (columns * patches - patches)
//          *
//              2.0f -
//            1.0f  // +
//          // sinf(frame_time.count() / 1000.0f *
//          //     sinf((patch + 1) * 22.1f + (row + 1) * 12.7f +
//          //          (column + 1) * 15.3f)) *
//          //  0.2f / patches
//          ,
//          row / (rows - 1.0f) * 2.0f - 1.0f  // +
//          // sinf(frame_time.count() / 1000.0f *
//          //     sinf((patch + 1) * 27.1f + (row + 1) * 18.9f +
//          //          (column + 1) * 11.4f)) *
//          //  0.2f
//          ,
//          0.0f, 1.0f);
//        mapped_data->vertices[i].textureCoord = math::make_vector_from(
//          (column + patch * (columns - 1.0f)) / (columns * patches - patches),
//          1.0f - row / (rows - 1.0f), 0.0f, 0.0f);
//      }
//    }
//  }
//
//  render_pass_instance::pre_update(buffer_index, frame_time);
//}
//
// void render_pass_warp_instance::update()
//{
//}
//
// void render_pass_warp_instance::create_resources()
//{
//  render_pass_instance::create_resources();
//
//  // Create buffers.
//  _uniform_buffer = _device->create_buffer(vk::buffer_create_info(
//    /* flags */ vk::buffer_create_flag::none,
//    /* size */ sizeof(warp_uniform_data),
//    /* usage */ vk::buffer_usage_flag::uniform_buffer_bit,
//    /* sharing_mode */ vk::sharing_mode::exclusive,
//    /* queue_family_index_count */ 0,
//    /* queue_family_indices */ nullptr));
//  _uniform_buffer->allocate_storage(
//    vk::memory_property_flag::host_visible_bit |
//    vk::memory_property_flag::host_coherent_bit);
//
//  // Create framebuffer and descriptor sets.
//  const auto& swapchain = _device->window()->swapchain();
//  _framebuffers.reserve(swapchain.image_count());
//
//  _descriptor_set =
//    _device->create_descriptor_set(vk::descriptor_set_allocate_info(
//      /* descriptor_pool */ _application.descriptor_pool().handle(),
//      /* descriptor_set_count */ 1,
//      /* set_layouts */ &_technique.descriptor_set_layout().handle()));
//}
//
// void render_pass_warp_instance::destroy_resources()
//{
//  _descriptor_set.reset();
//  _uniform_buffer.reset();
//  render_pass_instance::destroy_resources();
//}
//
// void render_pass_warp_instance::create_framebuffers(
//  const std::vector<vk::shared_ptr<vk::image_view>>& source_image_views,
//  const std::vector<vk::shared_ptr<vk::image_view>>& target_image_views)
//{
//  render_pass_instance::create_framebuffers();
//
//  const auto& swapchain = _device->window()->swapchain();
//  _framebuffers.reserve(swapchain.image_count());
//  for (std::uint32_t i = 0; i < swapchain.image_count(); ++i)
//  {
//    // Prepare descriptor sets.
//    const auto uniform_buffer_info =
//      core::make_array(vk::descriptor_buffer_info(
//        /* buffer */ _uniform_buffer->handle(),
//        /* offset */ 0,
//        /* range */ _uniform_buffer->size()));
//
//    const auto descriptor_image_info =
//      core::make_array(vk::descriptor_image_info(
//        /* sampler */ _technique.image_sampler().handle(),
//        /* image_view */ source_image_views[i]->handle(),
//        /* image_layout */ vk::image_layout::general));
//
//    _device->update_descriptor_set(vk::write_descriptor_set(
//      /* dst_set */ _descriptor_set->handle(),
//      /* dst_binding */ 0,
//      /* dst_array_element */ 0,
//      /* descriptor_count */ uniform_buffer_info.size(),
//      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
//      /* image_info */ nullptr,
//      /* buffer_info */ uniform_buffer_info.data(),
//      /* texel_buffer_view */ nullptr));
//    _device->update_descriptor_set(vk::write_descriptor_set(
//      /* dst_set */ _descriptor_set->handle(),
//      /* dst_binding */ 1,
//      /* dst_array_element */ 0,
//      /* descriptor_count */ descriptor_image_info.size(),
//      /* descriptor_type */ vk::descriptor_type::combined_image_sampler,
//      /* image_info */ descriptor_image_info.data(),
//      /* buffer_info */ nullptr,
//      /* texel_buffer_view */ nullptr));
//
//    // Prepare framebuffer.
//    _framebuffers.push_back(
//      _device->create_framebuffer(vk::framebuffer_create_info(
//        /* flags */ vk::framebuffer_create_flag::none,
//        /* render_pass */ _technique.render_pass().handle(),
//        /* attachment_count */ 1,
//        /* attachments */ &target_image_views[i]->handle(),
//        /* width */ target_image_views[i]->image().extent().width(),
//        /* height */ target_image_views[i]->image().extent().height(),
//        /* layers */ 1u)));
//  }
//}
//
// void render_pass_warp_instance::destroy_framebuffers()
//{
//  _framebuffers.clear();
//  render_pass_instance::destroy_framebuffers();
//}
//
// void render_pass_warp_instance::execute(
//  vk::command_buffer& primary_command_buffer, std::uint32_t current_buffer,
//  const vk::rect_2d& render_area)
//{
//  const auto clear_values = core::make_array(vk::clear_value(
//    vk::clear_color_value(core::make_array(0.0f, 0.0f, 0.0f, 0.0f))));
//
//  primary_command_buffer.begin_render_pass(
//    _technique.render_pass(), *_framebuffers[current_buffer], render_area,
//    clear_values.size(), clear_values.data(),
//    vk::subpass_contents::secondary_command_buffers);
//  primary_command_buffer.execute_commands(*_command_buffers[current_buffer]);
//  primary_command_buffer.end_render_pass();
//}
//
// void render_pass_warp_instance::build_command_buffer(std::uint32_t
// buffer_index)
//{
//  /// ToDo: Memory leak! Stash object on a graveyard.
//  _command_buffers[buffer_index].reset();
//  _command_buffers[buffer_index] = _device->create_command_buffer(
//    queue_type::graphics, _transient_command_buffer,
//    vk::command_buffer_level::secondary);
//
//  _command_buffers[buffer_index]->begin(vk::command_buffer_begin_info(
//    /* flags */ vk::command_buffer_usage_flag::render_pass_continue_bit |
//      vk::command_buffer_usage_flag::simultaneous_use_bit,
//    /* inheritance_info */ core::rvalue_address(
//      vk::command_buffer_inheritance_info(
//        /* render_pass */ _technique.render_pass().handle(),
//        /* subpass */ 0,
//        /* framebuffer */ _framebuffers[buffer_index]->handle(),
//        /* occlusion_query_enable */ VK_FALSE,
//        /* query_flags */ vk::query_control_flag::none,
//        /* pipeline_statistics */ vk::query_pipeline_statistic_flag::none))));
//
//  _command_buffers[buffer_index]->bind_pipeline(
//    vk::pipeline_bind_point::graphics, _technique.pipeline());
//  _command_buffers[buffer_index]->bind_descriptor_sets(
//    vk::pipeline_bind_point::graphics, _technique.pipeline_layout(), 0,
//    core::make_array(_descriptor_set));
//
//  auto window_size = _application.primary_window().size();
//  const auto viewports = core::make_array(vk::viewport(
//    /* x */ 0.0f,
//    /* y */ 0.0f,
//    /* width */ static_cast<float>(window_size.width),
//    /* height */ static_cast<float>(window_size.height),
//    /* min_depth */ 0.0f,
//    /* max_depth */ 1.0f));
//  _command_buffers[buffer_index]->set_viewports(0, viewports);
//
//  const auto scissors = core::make_array(vk::rect_2d(
//    /* offset */ vk::offset_2d{0, 0},
//    /* extent */ vk::extent_2d{window_size.width, window_size.height}));
//  _command_buffers[buffer_index]->set_scissors(0, scissors);
//
//  _command_buffers[buffer_index]->draw(patches * rows * columns, 1, 0, 0);
//  _command_buffers[buffer_index]->end();
//}
//}
