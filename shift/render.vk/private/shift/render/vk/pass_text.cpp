#include "shift/render/vk/pass_text.h"
#include "shift/render/vk/layer1/shader_module.h"
#include "shift/render/vk/layer1/device.h"
#include <shift/resource/repository.h>
#include <shift/resource/shader.h>
#include <shift/resource/font.h>
#include <shift/core/exception.h>
#include <shift/core/algorithm.h>
#include <shift/core/at_exit_scope.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <chrono>

namespace shift::render::vk
{
pass_text::pass_text(vk::layer1::device& device) : pass(device)
{
}

void pass_text::create_resource_descriptions()
{
}

void pass_text::destroy_resource_descriptions()
{
}

void pass_text::create_pipeline(vk::layer1::pipeline_cache& pipeline_cache)
{
  auto& repository = resource::repository::singleton_instance();

  // Create descriptor set layout and pipeline layout.
  const auto text_layout_bindings =
    core::make_array(vk::descriptor_set_layout_binding(
      /* binding */ 0,
      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
      /* descriptor_count */ 1,
      /* stage_flags */ vk::shader_stage_flag::vertex_bit,
      /* immutable_samplers */ nullptr));

  _descriptor_set_layout.create(
    *_device, vk::descriptor_set_layout_create_info(
                /* next */ nullptr,
                /* flags */ vk::descriptor_set_layout_create_flag::none,
                /* binding_count */ text_layout_bindings.size(),
                /* bindings */ text_layout_bindings.data()));
  _pipeline_layout.create(*_device,
                          vk::pipeline_layout_create_info(
                            /* next */ nullptr,
                            /* flags */ vk::pipeline_layout_create_flag::none,
                            /* set_layout_count */ 1,
                            /* set_layouts */ &_descriptor_set_layout->handle(),
                            /* push_constant_range_count */ 0,
                            /* push_constant_ranges */ nullptr));

  // Create render pass.
  const auto attachment_descriptions = core::make_array(
    vk::attachment_description(
      /* flags */ vk::attachment_description_flag::none,
      /* format */ vk::format::b8_g8_r8_a8_unorm,
      /* samples */ config::multisample_count,
      /* load_op */ vk::attachment_load_op::load,
      /* store_op */ vk::attachment_store_op::store,
      /* stencil_load_op */ vk::attachment_load_op::dont_care,
      /* stencil_store_op */ vk::attachment_store_op::dont_care,
      /* initial_layout */ vk::image_layout::color_attachment_optimal,
      /* final_layout */ vk::image_layout::color_attachment_optimal),
    vk::attachment_description(
      /* flags */ vk::attachment_description_flag::none,
      /* format */ config::depth_format,
      /* samples */ config::multisample_count,
      /* load_op */ vk::attachment_load_op::clear,
      /* store_op */ vk::attachment_store_op::store,
      /* stencil_load_op */ vk::attachment_load_op::dont_care,
      /* stencil_store_op */ vk::attachment_store_op::dont_care,
      /* initial_layout */ vk::image_layout::depth_stencil_attachment_optimal,
      /* final_layout */ vk::image_layout::depth_stencil_attachment_optimal));

  const auto color_references = core::make_array(vk::attachment_reference(
    /* attachment */ 0,
    /* layout */ vk::image_layout::color_attachment_optimal));

  const auto depth_reference = vk::attachment_reference(
    /* attachment */ 1,
    /* layout */ vk::image_layout::depth_stencil_attachment_optimal);

  const auto subpass_descriptions = core::make_array(vk::subpass_description(
    /* flags */ vk::subpass_description_flag::none,
    /* pipeline_bind_point */ vk::pipeline_bind_point::graphics,
    /* input_attachment_count */ 0,
    /* input_attachments */ nullptr,
    /* color_attachment_count */
    static_cast<std::uint32_t>(color_references.size()),
    /* color_attachments */ color_references.data(),
    /* resolve_attachments */ nullptr,
    /* depth_stencil_attachment */ &depth_reference,
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

  auto solid_vertex_shader_resource =
    repository.load<resource::shader>("public/shaders/glyphsolid.vert.spv");
  auto solid_fragment_shader_resource =
    repository.load<resource::shader>("public/shaders/glyphsolid.frag.spv");
  auto curve_vertex_shader_resource =
    repository.load<resource::shader>("public/shaders/glyphcurve.vert.spv");
  auto curve_geometry_shader_resource =
    repository.load<resource::shader>("public/shaders/glyphcurve.geom.spv");
  auto curve_fragment_shader_resource =
    repository.load<resource::shader>("public/shaders/glyphcurve.frag.spv");
  if (!solid_vertex_shader_resource || !solid_fragment_shader_resource ||
      !curve_vertex_shader_resource || !curve_geometry_shader_resource ||
      !curve_fragment_shader_resource)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Missing shader resource"));
  }

  vk::layer1::shader_module solid_vertex_shader(
    *_device,
    vk::shader_module_create_info(
      /* next */ nullptr,
      /* flags */ 0,
      /* code_size */ solid_vertex_shader_resource->storage.size() * 4,
      /* code */ solid_vertex_shader_resource->storage.data()));
  vk::layer1::shader_module solid_fragment_shader(
    *_device,
    vk::shader_module_create_info(
      /* next */ nullptr,
      /* flags */ 0,
      /* code_size */ solid_fragment_shader_resource->storage.size() * 4,
      /* code */ solid_fragment_shader_resource->storage.data()));
  vk::layer1::shader_module curve_vertex_shader(
    *_device,
    vk::shader_module_create_info(
      /* next */ nullptr,
      /* flags */ 0,
      /* code_size */ curve_vertex_shader_resource->storage.size() * 4,
      /* code */ curve_vertex_shader_resource->storage.data()));
  vk::layer1::shader_module curve_geometry_shader(
    *_device,
    vk::shader_module_create_info(
      /* next */ nullptr,
      /* flags */ 0,
      /* code_size */ curve_geometry_shader_resource->storage.size() * 4,
      /* code */ curve_geometry_shader_resource->storage.data()));
  vk::layer1::shader_module curve_fragment_shader(
    *_device,
    vk::shader_module_create_info(
      /* next */ nullptr,
      /* flags */ 0,
      /* code_size */ curve_fragment_shader_resource->storage.size() * 4,
      /* code */ curve_fragment_shader_resource->storage.data()));

  const auto solid_shader_stages =
    core::make_array(vk::pipeline_shader_stage_create_info(
                       /* next */ nullptr,
                       /* flags */ vk::pipeline_shader_stage_create_flag::none,
                       /* stage */ vk::shader_stage_flag::vertex_bit,
                       /* module */ solid_vertex_shader.handle(),
                       /* name */ "main",
                       /* specialization_info */ nullptr),
                     vk::pipeline_shader_stage_create_info(
                       /* next */ nullptr,
                       /* flags */ vk::pipeline_shader_stage_create_flag::none,
                       /* stage */ vk::shader_stage_flag::fragment_bit,
                       /* module */ solid_fragment_shader.handle(),
                       /* name */ "main",
                       /* specialization_info */ nullptr));

  const auto curve_shader_stages =
    core::make_array(vk::pipeline_shader_stage_create_info(
                       /* next */ nullptr,
                       /* flags */ vk::pipeline_shader_stage_create_flag::none,
                       /* stage */ vk::shader_stage_flag::vertex_bit,
                       /* module */ curve_vertex_shader.handle(),
                       /* name */ "main",
                       /* specialization_info */ nullptr),
                     vk::pipeline_shader_stage_create_info(
                       /* next */ nullptr,
                       /* flags */ vk::pipeline_shader_stage_create_flag::none,
                       /* stage */ vk::shader_stage_flag::geometry_bit,
                       /* module */ curve_geometry_shader.handle(),
                       /* name */ "main",
                       /* specialization_info */ nullptr),
                     vk::pipeline_shader_stage_create_info(
                       /* next */ nullptr,
                       /* flags */ vk::pipeline_shader_stage_create_flag::none,
                       /* stage */ vk::shader_stage_flag::fragment_bit,
                       /* module */ curve_fragment_shader.handle(),
                       /* name */ "main",
                       /* specialization_info */ nullptr));

  const auto vertex_bindings =
    core::make_array(vk::vertex_input_binding_description(
                       /* binding */ 0,
                       /* stride */ sizeof(glyph_vertex),
                       /* input_rate */ vk::vertex_input_rate::vertex),
                     vk::vertex_input_binding_description(
                       /* binding */ 1,
                       /* stride */ sizeof(glyph_instance),
                       /* input_rate */ vk::vertex_input_rate::instance));

  const auto vertex_attributes =
    core::make_array(vk::vertex_input_attribute_description(
                       /* location */ 0,
                       /* binding */ vertex_bindings[0].binding(),
                       /* format */ vk::format::r32_g32_sfloat,
                       /* offset */ offsetof(glyph_vertex, position)),
                     vk::vertex_input_attribute_description(
                       /* location */ 1,
                       /* binding */ vertex_bindings[1].binding(),
                       /* format */ vk::format::r32_g32_b32_sfloat,
                       /* offset */ offsetof(glyph_instance, translation)),
                     vk::vertex_input_attribute_description(
                       /* location */ 2,
                       /* binding */ vertex_bindings[1].binding(),
                       /* format */ vk::format::r32_sfloat,
                       /* offset */ offsetof(glyph_instance, scale)),
                     vk::vertex_input_attribute_description(
                       /* location */ 3,
                       /* binding */ vertex_bindings[1].binding(),
                       /* format */ vk::format::r32_g32_b32_a32_sfloat,
                       /* offset */ offsetof(glyph_instance, color)));

  const auto vertex_input_state = vk::pipeline_vertex_input_state_create_info(
    /* next */ nullptr,
    /* flags */ vk::pipeline_vertex_input_state_create_flag::none,
    /* vertex_binding_description_count */ vertex_bindings.size(),
    /* vertex_binding_descriptions */ vertex_bindings.data(),
    /* vertex_attribute_description_count */ vertex_attributes.size(),
    /* vertex_attribute_descriptions */ vertex_attributes.data());

  const auto input_assembly_state =
    vk::pipeline_input_assembly_state_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_input_assembly_state_create_flag::none,
      /* topology */ vk::primitive_topology::triangle_list,
      /* primitive_restart_enable */ VK_FALSE);

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
    /* rasterization_samples */ config::multisample_count,
    /* sample_shading_enable */ VK_FALSE,
    /* min_sample_shading */ 0.0f,
    /* sample_mask */ nullptr,
    /* alpha_to_coverage_enable */ VK_FALSE,
    /* alpha_to_one_enable */ VK_FALSE);

  const auto solid_depth_stencil_state =
    vk::pipeline_depth_stencil_state_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_depth_stencil_state_create_flag::none,
      /* depth_test_enable */ VK_TRUE,
      /* depth_write_enable */ VK_TRUE,
      /* depth_compare_op */ vk::compare_op::less_or_equal,
      /* depth_bounds_test_enable */ VK_FALSE,
      /* stencil_test_enable */ VK_FALSE,
      /* front */
      vk::stencil_op_state(
        /* fail_op */ vk::stencil_op::keep,
        /* pass_op */ vk::stencil_op::keep,
        /* depth_fail_op */ vk::stencil_op::keep,
        /* compare_op */ vk::compare_op::never,
        /* compare_mask */ 0,
        /* write_mask */ 0,
        /* reference */ 0),
      /* back */
      vk::stencil_op_state(
        /* fail_op */ vk::stencil_op::keep,
        /* pass_op */ vk::stencil_op::keep,
        /* depth_fail_op */ vk::stencil_op::keep,
        /* compare_op */ vk::compare_op::never,
        /* compare_mask */ 0,
        /* write_mask */ 0,
        /* reference */ 0),
      /* min_depth_bounds */ 0.0f,
      /* max_depth_bounds */ 0.0f);

  const auto curve_depth_stencil_state =
    vk::pipeline_depth_stencil_state_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_depth_stencil_state_create_flag::none,
      /* depth_test_enable */ VK_FALSE,
      /* depth_write_enable */ VK_FALSE,
      /* depth_compare_op */ vk::compare_op::less_or_equal,
      /* depth_bounds_test_enable */ VK_FALSE,
      /* stencil_test_enable */ VK_FALSE,
      /* front */
      vk::stencil_op_state(
        /* fail_op */ vk::stencil_op::keep,
        /* pass_op */ vk::stencil_op::keep,
        /* depth_fail_op */ vk::stencil_op::keep,
        /* compare_op */ vk::compare_op::never,
        /* compare_mask */ 0,
        /* write_mask */ 0,
        /* reference */ 0),
      /* back */
      vk::stencil_op_state(
        /* fail_op */ vk::stencil_op::keep,
        /* pass_op */ vk::stencil_op::keep,
        /* depth_fail_op */ vk::stencil_op::keep,
        /* compare_op */ vk::compare_op::never,
        /* compare_mask */ 0,
        /* write_mask */ 0,
        /* reference */ 0),
      /* min_depth_bounds */ 0.0f,
      /* max_depth_bounds */ 0.0f);

  const auto color_blend_attachment_states =
    core::make_array(vk::pipeline_color_blend_attachment_state(
      /* blend_enable */ VK_TRUE,
      /* src_color_blend_factor */ vk::blend_factor::src_alpha,
      /* dst_color_blend_factor */ vk::blend_factor::one_minus_src_alpha,
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

  _solid_pipeline = vk::make_framed_shared<vk::layer1::pipeline>(
    *_device, pipeline_cache,
    vk::graphics_pipeline_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_create_flag::none,
      /* stage_count */ solid_shader_stages.size(),
      /* stages */ solid_shader_stages.data(),
      /* vertex_input_state */ &vertex_input_state,
      /* input_assembly_state */ &input_assembly_state,
      /* tessellation_state */ nullptr,
      /* viewport_state */ &viewport_state,
      /* rasterization_state */ &rasterization_state,
      /* multisample_state */ &multisample_state,
      /* depth_stencil_state */ &solid_depth_stencil_state,
      /* color_blend_state */ &color_blend_state,
      /* dynamic_state */ &dynamic_state,
      /* layout */ _pipeline_layout->handle(),
      /* render_pass */ _render_pass->handle(),
      /* subpass */ 0,
      /* base_pipeline_handle */ nullptr,
      /* base_pipeline_index */ 0));

  _curve_pipeline = vk::make_framed_shared<vk::layer1::pipeline>(
    *_device, pipeline_cache,
    vk::graphics_pipeline_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_create_flag::none,
      /* stage_count */ curve_shader_stages.size(),
      /* stages */ curve_shader_stages.data(),
      /* vertex_input_state */ &vertex_input_state,
      /* input_assembly_state */ &input_assembly_state,
      /* tessellation_state */ nullptr,
      /* viewport_state */ &viewport_state,
      /* rasterization_state */ &rasterization_state,
      /* multisample_state */ &multisample_state,
      /* depth_stencil_state */ &curve_depth_stencil_state,
      /* color_blend_state */ &color_blend_state,
      /* dynamic_state */ &dynamic_state,
      /* layout */ _pipeline_layout->handle(),
      /* render_pass */ _render_pass->handle(),
      /* subpass */ 0,
      /* base_pipeline_handle */ nullptr,
      /* base_pipeline_index */ 0));

  _font_resource =
    repository
      .load<resource::font>("public/fonts/google/aldrich/Aldrich-Regular.font")
      .get_shared();
  if (!_font_resource)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Missing shader resource"));
  }

  // _font_resource->glyphs[0].solid_primitive;

  // std::set<const shift::resource::buffer*> vertex_buffers;
  if (_font_resource->mesh->vertex_attributes.size() != 1)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unsupported font vertex layout."));
  }
  auto& vertex_attribute = _font_resource->mesh->vertex_attributes.front();
  if (!vertex_attribute.vertex_buffer_view.buffer)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error() << shift::core::context_info(
        "Font meshes without vertex buffers are not supported."));
  }
  if (vertex_attribute.usage != resource::vertex_attribute_usage::position ||
      vertex_attribute.component_type !=
        resource::vertex_attribute_component_type::float32 ||
      vertex_attribute.data_type != resource::vertex_attribute_data_type::vec2)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unsupported font vertex layout."));
  }
  auto& vertex_buffer = *vertex_attribute.vertex_buffer_view.buffer;

  if (!_font_resource->mesh->index_buffer_view.buffer)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error() << shift::core::context_info(
        "Font meshes without index buffers are not supported."));
  }
  if (_font_resource->mesh->index_data_type !=
      resource::vertex_index_data_type::uint32)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unsupported font index format."));
  }
  auto& index_buffer = *_font_resource->mesh->index_buffer_view.buffer;

  _vertex_buffer = vk::make_framed_shared<vk::layer1::buffer>(
    *_device, vk::buffer_create_info(
                /* next */ nullptr,
                /* flags */ vk::buffer_create_flag::none,
                /* size */ vertex_buffer.storage.size(),
                /* usage */ vk::buffer_usage_flag::vertex_buffer_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr));
  _vertex_buffer->allocate_storage(memory_pool::pipeline_buffers);
  if (void* data = _vertex_buffer->address(0, 0))
  {
    std::memcpy(data, vertex_buffer.storage.data(),
                vertex_buffer.storage.size());
  }

  _index_buffer = vk::make_framed_shared<vk::layer1::buffer>(
    *_device, vk::buffer_create_info(
                /* next */ nullptr,
                /* flags */ vk::buffer_create_flag::none,
                /* size */ index_buffer.storage.size(),
                /* usage */ vk::buffer_usage_flag::index_buffer_bit,
                /* sharing_mode */ vk::sharing_mode::exclusive,
                /* queue_family_index_count */ 0,
                /* queue_family_indices */ nullptr));
  _index_buffer->allocate_storage(memory_pool::pipeline_buffers);
  if (void* data = _index_buffer->address(0, 0))
    std::memcpy(data, index_buffer.storage.data(), index_buffer.storage.size());
}

void pass_text::destroy_pipeline()
{
  _index_buffer.reset();
  _vertex_buffer.reset();
  _curve_pipeline.reset();
  _solid_pipeline.reset();
  _render_pass.reset();
  _pipeline_layout.reset();
  _descriptor_set_layout.reset();
}

void pass_text::create_scene_resources(resource::scene& scene)
{
  /// ToDo...
}

void pass_text::destroy_scene_resources()
{
  /// ToDo...
}

void pass_text::update(vk::layer2::multiview& multiview,
                       std::uint32_t frame_index)
{
  /// ToDo...
}
}

//#include "shift/render/vk/render_passes/text_instance.h"
//#include "shift/render/vk/render_passes/text_schematic.h"
//#include "vk/cube_application.h"
//#include <shift/resource/repository.h>
//#include <shift/resource/shader.h>
//#include <shift/resource/font.h>
//#include <shift/core/exception.h>
//#include <shift/core/algorithm.h>
//#include <boost/filesystem.hpp>
//
// namespace shift::render::vk::render_passes
//{
// render_pass_text_instance::render_pass_text_instance(
//  cube_application& application, vk::device& device,
//  render_pass_text_class& technique)
//: render_pass_instance(application, device, true), _technique(technique)
//{
//  _text =
//    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do\n"
//    "eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut\n"
//    "enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
//    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in\n"
//    "reprehenderit in voluptate velit esse cillum dolore eu fugiat\n"
//    "nulla pariatur. Excepteur sint occaecat cupidatat non proident, \n"
//    "sunt in culpa qui officia deserunt mollit anim id est laborum.\n";
//}
//
// void render_pass_text_instance::pre_update(std::uint32_t buffer_index,
//                                           std::chrono::milliseconds
//                                           frame_time)
//{
//  auto window_size = _application.primary_window().size();
//  auto projection_matrix = math::make_orthographic_projection_matrix<float>(
//    0.0f, window_size.width, window_size.height, 0.0f, 0.0f, 1.0f);
//  auto model_matrix = math::make_translation_matrix<4, 4, float>(
//    math::make_vector_from(50.0f, 100.0f, 0.0f));
//
//  auto* uniforms = static_cast<render_pass_text_class::glyph_uniform*>(
//    _uniform_buffer->address());
//  if (uniforms)
//  {
//    uniforms->mvp = projection_matrix * model_matrix;
//  }
//
//  render_pass_instance::pre_update(buffer_index, frame_time);
//}
//
// void render_pass_text_instance::create_resources()
//{
//  render_pass_instance::create_resources();
//
//  // Create buffers.
//  _instance_buffer = _device->create_buffer(vk::buffer_create_info(
//    /* flags */ vk::buffer_create_flag::none,
//    /* size */ max_glyphs * sizeof(render_pass_text_class::glyph_instance),
//    /* usage */ vk::buffer_usage_flag::vertex_buffer_bit,
//    /* sharing_mode */ vk::sharing_mode::exclusive,
//    /* queue_family_index_count */ 0,
//    /* queue_family_indices */ nullptr));
//  _instance_buffer->allocate_storage(
//    vk::memory_property_flag::host_visible_bit |
//    vk::memory_property_flag::host_coherent_bit);
//
//  _uniform_buffer = _device->create_buffer(vk::buffer_create_info(
//    /* flags */ vk::buffer_create_flag::none,
//    /* size */ sizeof(render_pass_text_class::glyph_uniform),
//    /* usage */ vk::buffer_usage_flag::uniform_buffer_bit,
//    /* sharing_mode */ vk::sharing_mode::exclusive,
//    /* queue_family_index_count */ 0,
//    /* queue_family_indices */ nullptr));
//  _uniform_buffer->allocate_storage(
//    vk::memory_property_flag::host_visible_bit |
//    vk::memory_property_flag::host_coherent_bit);
//
//  _solid_draw_buffer = _device->create_buffer(vk::buffer_create_info(
//    /* flags */ vk::buffer_create_flag::none,
//    /* size */ max_glyphs * sizeof(VkDrawIndexedIndirectCommand),
//    /* usage */ vk::buffer_usage_flag::indirect_buffer_bit,
//    /* sharing_mode */ vk::sharing_mode::exclusive,
//    /* queue_family_index_count */ 0,
//    /* queue_family_indices */ nullptr));
//  _solid_draw_buffer->allocate_storage(
//    vk::memory_property_flag::host_visible_bit |
//    vk::memory_property_flag::host_coherent_bit);
//
//  _curve_draw_buffer = _device->create_buffer(vk::buffer_create_info(
//    /* flags */ vk::buffer_create_flag::none,
//    /* size */ max_glyphs * sizeof(VkDrawIndexedIndirectCommand),
//    /* usage */ vk::buffer_usage_flag::indirect_buffer_bit,
//    /* sharing_mode */ vk::sharing_mode::exclusive,
//    /* queue_family_index_count */ 0,
//    /* queue_family_indices */ nullptr));
//  _curve_draw_buffer->allocate_storage(
//    vk::memory_property_flag::host_visible_bit |
//    vk::memory_property_flag::host_coherent_bit);
//
//  auto window_size = _application.primary_window().size();
//  auto projection_matrix = math::make_orthographic_projection_matrix<float>(
//    0.0f, window_size.width, window_size.height, 0.0f, 0.0f, 1.0f);
//
//  // Populate draw buffers.
//  auto* instances = static_cast<render_pass_text_class::glyph_instance*>(
//    _instance_buffer->address());
//  auto* solid_draws =
//    static_cast<VkDrawIndexedIndirectCommand*>(_solid_draw_buffer->address());
//  auto* curve_draws =
//    static_cast<VkDrawIndexedIndirectCommand*>(_curve_draw_buffer->address());
//  if (instances && solid_draws && curve_draws)
//  {
//    static constexpr float scale = 16.0f;
//    auto position = math::make_vector_from(0.0f, 0.0f, 0.0f);
//    std::uint32_t num_glyphs = 0;
//
//    auto& font = _technique.font_resource();
//    auto& vertex_attribute = font.mesh->vertex_attributes.front();
//    for (auto c : _text)
//    {
//      auto glyph = font.glyphs.find(c);
//      if (glyph != font.glyphs.end())
//      {
//        if (c != ' ')
//        {
//          instances[num_glyphs].translation = position;
//          instances[num_glyphs].scale = scale;
//          instances[num_glyphs].color =
//            math::make_vector_from(0.2f, 0.5f, 0.8f, 1.0f);
//
//          if (glyph->second.solid_sub_mesh_id)
//          {
//            if (glyph->second.solid_sub_mesh_id >
//            font.mesh->sub_meshes.size())
//            {
//              BOOST_THROW_EXCEPTION(
//                shift::core::runtime_error()
//                << shift::core::context_info("Mesh index out of bounds."));
//            }
//            const auto& solid_sub_mesh =
//              font.mesh->sub_meshes[glyph->second.solid_sub_mesh_id - 1];
//
//            solid_draws[num_glyphs].indexCount = solid_sub_mesh.index_count;
//            solid_draws[num_glyphs].instanceCount = 1;
//            solid_draws[num_glyphs].firstIndex = solid_sub_mesh.first_index;
//            solid_draws[num_glyphs].vertexOffset =
//            solid_sub_mesh.vertex_offset;
//            solid_draws[num_glyphs].firstInstance = num_glyphs;
//          }
//          else
//          {
//            solid_draws[num_glyphs].indexCount = 0;
//            solid_draws[num_glyphs].instanceCount = 0;
//            solid_draws[num_glyphs].firstIndex = 0;
//            solid_draws[num_glyphs].vertexOffset = 0;
//            solid_draws[num_glyphs].firstInstance = num_glyphs;
//          }
//
//          if (glyph->second.curved_sub_mesh_id)
//          {
//            if (glyph->second.curved_sub_mesh_id >
//            font.mesh->sub_meshes.size())
//            {
//              BOOST_THROW_EXCEPTION(
//                shift::core::runtime_error()
//                << shift::core::context_info("Mesh index out of bounds."));
//            }
//            const auto& curved_sub_mesh =
//              font.mesh->sub_meshes[glyph->second.curved_sub_mesh_id - 1];
//
//            curve_draws[num_glyphs].indexCount = curved_sub_mesh.index_count;
//            curve_draws[num_glyphs].instanceCount = 1;
//            curve_draws[num_glyphs].firstIndex = curved_sub_mesh.first_index;
//            curve_draws[num_glyphs].vertexOffset =
//              curved_sub_mesh.vertex_offset;
//            curve_draws[num_glyphs].firstInstance = num_glyphs;
//          }
//          else
//          {
//            curve_draws[num_glyphs].indexCount = 0;
//            curve_draws[num_glyphs].instanceCount = 0;
//            curve_draws[num_glyphs].firstIndex = 0;
//            curve_draws[num_glyphs].vertexOffset = 0;
//            curve_draws[num_glyphs].firstInstance = num_glyphs;
//          }
//
//          if (++num_glyphs >= max_glyphs)
//            break;
//        }
//        position.x += glyph->second.advance_x * scale;
//        if (position.x >= window_size.width - 100.0f)
//        {
//          position.x = 0;
//          position.y += (font.ascent - font.descent + font.line_gap) * scale;
//        }
//      }
//    }
//    _num_glyphs = num_glyphs;
//  }
//
//  _application.init_command_buffer().pipeline_barrier(
//    vk::pipeline_stage_flag::top_of_pipe_bit,
//    vk::pipeline_stage_flag::top_of_pipe_bit, 0,
//    vk::buffer_memory_barrier(
//      /* src_access_mask */ vk::access_flag::host_write_bit,
//      /* dst_access_mask */ vk::access_flag::vertex_attribute_read_bit,
//      /* src_queue_family_index */ _device->present_queue_family_index(),
//      /* dst_queue_family_index */ _device->graphics_queue_family_index(),
//      /* buffer */ _solid_draw_buffer->handle(),
//      /* offset */ 0,
//      /* size */ VK_WHOLE_SIZE));
//  _application.init_command_buffer().pipeline_barrier(
//    vk::pipeline_stage_flag::top_of_pipe_bit,
//    vk::pipeline_stage_flag::top_of_pipe_bit, 0,
//    vk::buffer_memory_barrier(
//      /* src_access_mask */ vk::access_flag::host_write_bit,
//      /* dst_access_mask */ vk::access_flag::vertex_attribute_read_bit,
//      /* src_queue_family_index */ _device->present_queue_family_index(),
//      /* dst_queue_family_index */ _device->graphics_queue_family_index(),
//      /* buffer */ _curve_draw_buffer->handle(),
//      /* offset */ 0,
//      /* size */ VK_WHOLE_SIZE));
//
//  const auto& swapchain = _device->window()->swapchain();
//  _framebuffers.reserve(swapchain.image_count());
//
//  _text_descriptor_set =
//    _device->create_descriptor_set(vk::descriptor_set_allocate_info(
//      /* descriptor_pool */ _application.descriptor_pool().handle(),
//      /* descriptor_set_count */ 1,
//      /* set_layouts */ &_technique.descriptor_set_layout().handle()));
//
//  // Prepare descriptor set.
//  _device->update_descriptor_set(vk::write_descriptor_set(
//    /* dst_set */ _text_descriptor_set->handle(),
//    /* dst_binding */ 0,
//    /* dst_array_element */ 0,
//    /* descriptor_count */ 1,
//    /* descriptor_type */ vk::descriptor_type::uniform_buffer,
//    /* image_info */ nullptr,
//    /* buffer_info */
//    core::rvalue_address(vk::descriptor_buffer_info(
//      /* buffer */ _uniform_buffer->handle(),
//      /* offset */ 0,
//      /* range */ _uniform_buffer->size())),
//    /* texel_buffer_view */ nullptr));
//}
//
// void render_pass_text_instance::destroy_resources()
//{
//  _text_descriptor_set.reset();
//  _curve_draw_buffer.reset();
//  _solid_draw_buffer.reset();
//  _uniform_buffer.reset();
//  _instance_buffer.reset();
//  render_pass_instance::destroy_resources();
//}
//
// void render_pass_text_instance::create_framebuffers(
//  const std::vector<boost::intrusive_ptr<vk::image_view>>& target_image_views)
//{
//  render_pass_instance::create_framebuffers();
//
//  auto window_size = _application.primary_window().size();
//  const auto& swapchain = _device->window()->swapchain();
//  _framebuffers.reserve(swapchain.image_count());
//  for (std::uint32_t i = 0; i < swapchain.image_count(); ++i)
//  {
//    auto attachments =
//      core::make_array(target_image_views[i]->handle(),
//                       _application.multisample_depth_image_view().handle());
//
//    // Prepare framebuffer.
//    _framebuffers.push_back(
//      _device->create_framebuffer(vk::framebuffer_create_info(
//        /* flags */ vk::framebuffer_create_flag::none,
//        /* render_pass */ _technique.render_pass().handle(),
//        /* attachment_count */ attachments.size(),
//        /* attachments */ attachments.data(),
//        /* width */ window_size.width,
//        /* height */ window_size.height,
//        /* layers */ 1)));
//  }
//}
//
// void render_pass_text_instance::destroy_framebuffers()
//{
//  _framebuffers.clear();
//  render_pass_instance::destroy_framebuffers();
//}
//
// void render_pass_text_instance::execute(
//  vk::command_buffer& primary_command_buffer, std::uint32_t current_buffer,
//  const vk::rect_2d& render_area)
//{
//  const auto clear_values = core::make_array(
//    vk::clear_value{
//      vk::clear_color_value{core::make_array(0.2f, 0.2f, 0.2f, 0.2f)}},
//    vk::clear_value{vk::clear_depth_stencil_value(
//      /* depth */ 1.0f,
//      /* stencil */ 0)});
//
//  primary_command_buffer.begin_render_pass(
//    _technique.render_pass(), *_framebuffers[current_buffer], render_area,
//    clear_values.size(), clear_values.data(),
//    vk::subpass_contents::secondary_command_buffers);
//  primary_command_buffer.execute_commands(*_command_buffers[current_buffer]);
//  primary_command_buffer.end_render_pass();
//}
//
// void render_pass_text_instance::build_command_buffer(std::uint32_t
// buffer_index)
//{
//  /// ToDo: Memory leak! Stash object on a graveyard.
//  _command_buffers[buffer_index].reset();
//  _command_buffers[buffer_index] = _device->create_command_buffer(
//    queue_type::graphics, _transient_command_buffer,
//    vk::command_buffer_level::secondary);
//
//  auto window_size = _application.primary_window().size();
//  const auto viewports = core::make_array(vk::viewport(
//    /* x */ 0.0f,
//    /* y */ 0.0f,
//    /* width */ static_cast<float>(window_size.width),
//    /* height */ static_cast<float>(window_size.height),
//    /* min_depth */ 0.0f,
//    /* max_depth */ 1.0f));
//
//  const auto scissors = core::make_array(vk::rect_2d(
//    /* offset */ vk::offset_2d{0, 0},
//    /* extent */ vk::extent_2d{window_size.width, window_size.height}));
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
//    vk::pipeline_bind_point::graphics, _technique.solid_pipeline());
//
//  _command_buffers[buffer_index]->bind_vertex_buffers(
//    0,
//    core::make_array<vk::buffer*>(&_technique.vertex_buffer(),
//                                  _instance_buffer.get()),
//    {0, 0});
//  _command_buffers[buffer_index]->bind_index_buffer(_technique.index_buffer(),
//                                                    0,
//                                                    vk::index_type::uint32);
//  _command_buffers[buffer_index]->bind_descriptor_sets(
//    vk::pipeline_bind_point::graphics, _technique.pipeline_layout(), 0,
//    core::make_array(_text_descriptor_set));
//
//  _command_buffers[buffer_index]->set_viewports(0, viewports);
//  _command_buffers[buffer_index]->set_scissors(0, scissors);
//
//  _command_buffers[buffer_index]->draw_indexed_indirect(
//    *_solid_draw_buffer, 0, _num_glyphs,
//    sizeof(VkDrawIndexedIndirectCommand));
//
//  _command_buffers[buffer_index]->bind_pipeline(
//    vk::pipeline_bind_point::graphics, _technique.curve_pipeline());
//
//  _command_buffers[buffer_index]->draw_indexed_indirect(
//    *_curve_draw_buffer, 0, _num_glyphs,
//    sizeof(VkDrawIndexedIndirectCommand));
//
//  _command_buffers[buffer_index]->end();
//}
//}
