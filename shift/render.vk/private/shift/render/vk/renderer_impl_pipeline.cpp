#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/window_impl.hpp"
#include "shift/render/vk/pass_warp.hpp"
#include "shift/render/vk/pass_text.hpp"
#include <shift/render/vk/geometry_generator.hpp>
#include "shift/render/vk/layer2/view.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/layer1/instance.hpp"
#include <shift/resource_db/repository.hpp>
#include <shift/resource_db/shader.hpp>
#include <gsl/gsl>

#include <shift/log/log.hpp>

namespace shift::render::vk
{
using namespace std::placeholders;
using namespace std::chrono_literals;
using namespace shift;

void renderer_impl::create_pipeline()
{
  auto& repository = resource_db::repository::singleton_instance();

  for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
       ++swapchain_index)
  {
    gsl::at(_prepare_command_buffers, swapchain_index) =
      vk::make_framed_shared<vk::layer1::command_buffer>(
        _device->select_command_pool(vk::layer1::queue_type::graphics, true),
        vk::command_buffer_level::primary);
    gsl::at(_primary_command_buffers, swapchain_index) =
      vk::make_framed_shared<vk::layer1::command_buffer>(
        _device->select_command_pool(vk::layer1::queue_type::graphics, true),
        vk::command_buffer_level::primary);
    gsl::at(_submit_fences, swapchain_index) =
      vk::layer1::fence{*_device, vk::fence_create_flag::none};
    gsl::at(_submitted, swapchain_index) = false;
  }

  _pipeline_cache.create(*_device,
                         vk::pipeline_cache_create_info(
                           /* next */ nullptr,
                           /* flags */ vk::pipeline_cache_create_flag::none,
                           /* initial_data_size */ 0,
                           /* initial_data */ nullptr));

#pragma region reproject depth
  {
    // Create descriptor set layout and pipeline layout.
    const auto scene_layout_bindings = core::make_array(
      vk::descriptor_set_layout_binding(
        /* binding */ 0,
        /* descriptor_type */ vk::descriptor_type::uniform_buffer,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::compute_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 1,
        /* descriptor_type */ vk::descriptor_type::combined_image_sampler,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::compute_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 2,
        /* descriptor_type */ vk::descriptor_type::storage_image,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::compute_bit,
        /* immutable_samplers */ nullptr));
    _reproject_depth_descriptor_set_layout.create(
      *_device, vk::descriptor_set_layout_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::descriptor_set_layout_create_flag::none,
                  /* binding_count */ scene_layout_bindings.size(),
                  /* bindings */ scene_layout_bindings.data()));
    _reproject_depth_pipeline_layout.create(
      *_device, vk::pipeline_layout_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::pipeline_layout_create_flag::none,
                  /* set_layout_count */ 1,
                  /* set_layouts */
                  &_reproject_depth_descriptor_set_layout->handle(),
                  /* push_constant_range_count */ 0,
                  /* push_constant_ranges */ nullptr));

    auto compute_shader_resource = repository.load<resource_db::shader>(
      "resources/shaders/reproject_depth.comp.spv");
    if (!compute_shader_resource)
    {
      BOOST_THROW_EXCEPTION(
        shift::core::runtime_error()
        << shift::core::context_info("Missing shader resource"));
    }
    vk::layer1::shader_module compute_shader(
      *_device, vk::shader_module_create_info(
                  /* next */ nullptr,
                  /* flags */ {},
                  /* code_size */ compute_shader_resource->storage.size() * 4,
                  /* code */ compute_shader_resource->storage.data()));

    _reproject_depth_pipeline = vk::make_framed_shared<vk::layer1::pipeline>(
      *_device, *_pipeline_cache,
      vk::compute_pipeline_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_create_flag::none,
        /* stage */
        vk::pipeline_shader_stage_create_info(
          /* next */ nullptr,
          /* flags */ vk::pipeline_shader_stage_create_flag::none,
          /* stage */ vk::shader_stage_flag::compute_bit,
          /* module */ compute_shader.handle(),
          /* name */ "main",
          /* specialization_info */ nullptr),
        /* layout */ _reproject_depth_pipeline_layout->handle(),
        /* base_pipeline_handle */ nullptr,
        /* base_pipeline_index */ 0));

    _reproject_depth_sampler = vk::make_framed_shared<vk::layer1::sampler>(
      *_device, vk::sampler_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::sampler_create_flag::none,
                  /* mag_filter */ vk::filter::nearest,
                  /* min_filter */ vk::filter::nearest,
                  /* mipmap_mode */ vk::sampler_mipmap_mode::nearest,
                  /* address_mode_u */
                  vk::sampler_address_mode::clamp_to_edge,
                  /* address_mode_v */
                  vk::sampler_address_mode::clamp_to_edge,
                  /* address_mode_w */
                  vk::sampler_address_mode::clamp_to_edge,
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
#pragma endregion

#pragma region scene
  {
    // Create descriptor set layout and pipeline layout.
    const auto scene_layout_bindings = core::make_array(
      vk::descriptor_set_layout_binding(
        /* binding */ 0,
        /* descriptor_type */ vk::descriptor_type::uniform_buffer,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::vertex_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 1,
        /* descriptor_type */ vk::descriptor_type::sampled_image,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::fragment_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 2,
        /* descriptor_type */ vk::descriptor_type::sampler,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::fragment_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 3,
        /* descriptor_type */ vk::descriptor_type::sampled_image,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::fragment_bit,
        /* immutable_samplers */ nullptr),
      vk::descriptor_set_layout_binding(
        /* binding */ 4,
        /* descriptor_type */ vk::descriptor_type::sampler,
        /* descriptor_count */ 1,
        /* stage_flags */ vk::shader_stage_flag::fragment_bit,
        /* immutable_samplers */ nullptr));

    _scene_descriptor_set_layout.create(
      *_device, vk::descriptor_set_layout_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::descriptor_set_layout_create_flag::none,
                  /* binding_count */ scene_layout_bindings.size(),
                  /* bindings */ scene_layout_bindings.data()));
    _scene_pipeline_layout.create(
      *_device, vk::pipeline_layout_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::pipeline_layout_create_flag::none,
                  /* set_layout_count */ 1,
                  /* set_layouts */ &_scene_descriptor_set_layout->handle(),
                  /* push_constant_range_count */ 0,
                  /* push_constant_ranges */ nullptr));

    // Create render pass.
    /// ToDo: We potentially need one pipeline per multiview.
    // std::vector<vk::attachment_description> attachment_descriptions;
    // for (const auto& view :

    const auto attachment_descriptions = core::make_array(
      vk::attachment_description(
        /* flags */ vk::attachment_description_flag::none,
        /* format */ vk::format::b8_g8_r8_a8_unorm,
        /* samples */ vk::sample_count_flag::_1_bit,
        /* load_op */ vk::attachment_load_op::clear,
        /* store_op */ vk::attachment_store_op::store,
        /* stencil_load_op */ vk::attachment_load_op::dont_care,
        /* stencil_store_op */ vk::attachment_store_op::dont_care,
        /* initial_layout */ vk::image_layout::color_attachment_optimal,
        /* final_layout */ vk::image_layout::color_attachment_optimal),
      vk::attachment_description(
        /* flags */ vk::attachment_description_flag::none,
        /* format */ config::depth_format,
        /* samples */ vk::sample_count_flag::_1_bit,
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

    _scene_render_pass = vk::make_framed_shared<vk::layer1::render_pass>(
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

    auto vertex_shader_textured_resource = repository.load<resource_db::shader>(
      "resources/shaders/materials/default_textured.vert.spv");
    auto fragment_shader_textured_resource =
      repository.load<resource_db::shader>(
        "resources/shaders/materials/default_textured.frag.spv");
    auto vertex_shader_untextured_resource =
      repository.load<resource_db::shader>(
        "resources/shaders/materials/default_untextured.vert.spv");
    auto fragment_shader_untextured_resource =
      repository.load<resource_db::shader>(
        "resources/shaders/materials/default_untextured.frag.spv");
    auto vertex_shader_wireframe_resource =
      repository.load<resource_db::shader>(
        "resources/shaders/materials/default_wireframe.vert.spv");
    auto fragment_shader_wireframe_resource =
      repository.load<resource_db::shader>(
        "resources/shaders/materials/default_wireframe.frag.spv");
    if (!vertex_shader_textured_resource ||
        !fragment_shader_textured_resource ||
        !vertex_shader_untextured_resource ||
        !fragment_shader_untextured_resource ||
        !vertex_shader_wireframe_resource ||
        !fragment_shader_wireframe_resource)
    {
      BOOST_THROW_EXCEPTION(
        shift::core::runtime_error()
        << shift::core::context_info("Missing shader resource"));
    }

    vk::layer1::shader_module vertex_shader_textured(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ vertex_shader_textured_resource->storage.size() * 4,
        /* code */ vertex_shader_textured_resource->storage.data()));
    vk::layer1::shader_module fragment_shader_textured(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ fragment_shader_textured_resource->storage.size() * 4,
        /* code */ fragment_shader_textured_resource->storage.data()));
    vk::layer1::shader_module vertex_shader_untextured(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ vertex_shader_untextured_resource->storage.size() * 4,
        /* code */ vertex_shader_untextured_resource->storage.data()));
    vk::layer1::shader_module fragment_shader_untextured(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ fragment_shader_untextured_resource->storage.size() * 4,
        /* code */ fragment_shader_untextured_resource->storage.data()));
    vk::layer1::shader_module vertex_shader_wireframe(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ vertex_shader_wireframe_resource->storage.size() * 4,
        /* code */ vertex_shader_wireframe_resource->storage.data()));
    vk::layer1::shader_module fragment_shader_wireframe(
      *_device,
      vk::shader_module_create_info(
        /* next */ nullptr,
        /* flags */ {},
        /* code_size */ fragment_shader_wireframe_resource->storage.size() * 4,
        /* code */ fragment_shader_wireframe_resource->storage.data()));

    const auto shader_stages_textured = core::make_array(
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::vertex_bit,
        /* module */ vertex_shader_textured.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr),
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::fragment_bit,
        /* module */ fragment_shader_textured.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr));
    const auto shader_stages_untextured = core::make_array(
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::vertex_bit,
        /* module */ vertex_shader_untextured.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr),
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::fragment_bit,
        /* module */ fragment_shader_untextured.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr));
    const auto shader_stages_wireframe = core::make_array(
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::vertex_bit,
        /* module */ vertex_shader_wireframe.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr),
      vk::pipeline_shader_stage_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_shader_stage_create_flag::none,
        /* stage */ vk::shader_stage_flag::fragment_bit,
        /* module */ fragment_shader_wireframe.handle(),
        /* name */ "main",
        /* specialization_info */ nullptr));

    const auto vertex_bindings_textured =
      core::make_array(vk::vertex_input_binding_description(
                         /* binding */ 0u,
                         /* stride */ sizeof(math::vector3<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 1u,
                         /* stride */ sizeof(math::vector3<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 2u,
                         /* stride */ sizeof(math::vector2<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 3u,
                         /* stride */ sizeof(mesh_instance_data),
                         /* input_rate */ vk::vertex_input_rate::instance));

    const auto vertex_attributes_textured =
      core::make_array(vk::vertex_input_attribute_description(
                         /* location */ 0,
                         /* binding */ vertex_bindings_textured[0].binding(),
                         /* format */ vk::format::r32_g32_b32_sfloat,
                         /* offset */ 0),
                       vk::vertex_input_attribute_description(
                         /* location */ 1,
                         /* binding */ vertex_bindings_textured[1].binding(),
                         /* format */ vk::format::r32_g32_b32_sfloat,
                         /* offset */ 0),
                       vk::vertex_input_attribute_description(
                         /* location */ 2,
                         /* binding */ vertex_bindings_textured[2].binding(),
                         /* format */ vk::format::r32_g32_sfloat,
                         /* offset */ 0),
                       vk::vertex_input_attribute_description(
                         /* location */ 3,
                         /* binding */ vertex_bindings_textured[3].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           0 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 4,
                         /* binding */ vertex_bindings_textured[3].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           1 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 5,
                         /* binding */ vertex_bindings_textured[3].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           2 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 6,
                         /* binding */ vertex_bindings_textured[3].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           3 * sizeof(math::vector4<float>)));

    const auto vertex_input_state_textured =
      vk::pipeline_vertex_input_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_vertex_input_state_create_flag::none,
        /* vertex_binding_description_count */ vertex_bindings_textured.size(),
        /* vertex_binding_descriptions */ vertex_bindings_textured.data(),
        /* vertex_attribute_description_count */
        vertex_attributes_textured.size(),
        /* vertex_attribute_descriptions */ vertex_attributes_textured.data());

    const auto vertex_bindings_untextured =
      core::make_array(vk::vertex_input_binding_description(
                         /* binding */ 0u,
                         /* stride */ sizeof(math::vector3<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 1u,
                         /* stride */ sizeof(math::vector3<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 2u,
                         /* stride */ sizeof(mesh_instance_data),
                         /* input_rate */ vk::vertex_input_rate::instance));

    const auto vertex_attributes_untextured =
      core::make_array(vk::vertex_input_attribute_description(
                         /* location */ 0u,
                         /* binding */ vertex_bindings_untextured[0].binding(),
                         /* format */ vk::format::r32_g32_b32_sfloat,
                         /* offset */ 0u),
                       vk::vertex_input_attribute_description(
                         /* location */ 1u,
                         /* binding */ vertex_bindings_untextured[1].binding(),
                         /* format */ vk::format::r32_g32_b32_sfloat,
                         /* offset */ 0u),
                       vk::vertex_input_attribute_description(
                         /* location */ 2u,
                         /* binding */ vertex_bindings_untextured[2].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           0 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 3u,
                         /* binding */ vertex_bindings_untextured[2].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           1 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 4u,
                         /* binding */ vertex_bindings_untextured[2].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           2 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 5u,
                         /* binding */ vertex_bindings_untextured[2].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           3 * sizeof(math::vector4<float>)));

    const auto vertex_input_state_untextured =
      vk::pipeline_vertex_input_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_vertex_input_state_create_flag::none,
        /* vertex_binding_description_count */
        vertex_bindings_untextured.size(),
        /* vertex_binding_descriptions */ vertex_bindings_untextured.data(),
        /* vertex_attribute_description_count */
        vertex_attributes_untextured.size(),
        /* vertex_attribute_descriptions */
        vertex_attributes_untextured.data());

    const auto vertex_bindings_wireframe =
      core::make_array(vk::vertex_input_binding_description(
                         /* binding */ 0u,
                         /* stride */ sizeof(math::vector3<float>),
                         /* input_rate */ vk::vertex_input_rate::vertex),
                       vk::vertex_input_binding_description(
                         /* binding */ 1u,
                         /* stride */ sizeof(mesh_instance_data),
                         /* input_rate */ vk::vertex_input_rate::instance));

    const auto vertex_attributes_wireframe =
      core::make_array(vk::vertex_input_attribute_description(
                         /* location */ 0u,
                         /* binding */ vertex_bindings_wireframe[0].binding(),
                         /* format */ vk::format::r32_g32_b32_sfloat,
                         /* offset */ 0),
                       vk::vertex_input_attribute_description(
                         /* location */ 1u,
                         /* binding */ vertex_bindings_wireframe[1].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model)),
                       vk::vertex_input_attribute_description(
                         /* location */ 2u,
                         /* binding */ vertex_bindings_wireframe[1].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           1 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 3u,
                         /* binding */ vertex_bindings_wireframe[1].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           2 * sizeof(math::vector4<float>)),
                       vk::vertex_input_attribute_description(
                         /* location */ 4u,
                         /* binding */ vertex_bindings_wireframe[1].binding(),
                         /* format */ vk::format::r32_g32_b32_a32_sfloat,
                         /* offset */ offsetof(mesh_instance_data, model) +
                           3 * sizeof(math::vector4<float>)));

    const auto vertex_input_state_wireframe =
      vk::pipeline_vertex_input_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_vertex_input_state_create_flag::none,
        /* vertex_binding_description_count */ vertex_bindings_wireframe.size(),
        /* vertex_binding_descriptions */ vertex_bindings_wireframe.data(),
        /* vertex_attribute_description_count */
        vertex_attributes_wireframe.size(),
        /* vertex_attribute_descriptions */ vertex_attributes_wireframe.data());

    const auto input_assembly_state_triangles =
      vk::pipeline_input_assembly_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_input_assembly_state_create_flag::none,
        /* topology */ vk::primitive_topology::triangle_list,
        /* primitive_restart_enable */ VK_FALSE);

    const auto input_assembly_state_lines =
      vk::pipeline_input_assembly_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_input_assembly_state_create_flag::none,
        /* topology */ vk::primitive_topology::line_list,
        /* primitive_restart_enable */ VK_FALSE);

    const auto viewport_state = vk::pipeline_viewport_state_create_info(
      /* next */ nullptr,
      /* flags */ vk::pipeline_viewport_state_create_flag::none,
      /* viewport_count */ 1,  // Dynamic state.
      /* viewports */ nullptr,
      /* scissor_count */ 1,  // Dynamic state.
      /* scissors */ nullptr);

    const auto rasterization_state_filled =
      vk::pipeline_rasterization_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_rasterization_state_create_flag::none,
        /* depth_clamp_enable */ VK_FALSE,
        /* rasterizer_discard_enable */ VK_FALSE,
        /* polygon_mode */ vk::polygon_mode::fill,
        /* cull_mode */ vk::cull_mode_flag::none,  // back_bit,
        /* front_face */ vk::front_face::counter_clockwise,
        /* depth_bias_enable */ VK_FALSE,
        /* depth_bias_constant_factor */ 0.0f,
        /* depth_bias_clamp */ 0.0f,
        /* depth_bias_slope_factor */ 0.0f,
        /* line_width */ 1.0f);

    const auto rasterization_state_wireframe =
      vk::pipeline_rasterization_state_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_rasterization_state_create_flag::none,
        /* depth_clamp_enable */ VK_FALSE,
        /* rasterizer_discard_enable */ VK_FALSE,
        /* polygon_mode */ vk::polygon_mode::line,
        /* cull_mode */ vk::cull_mode_flag::none,  // back_bit,
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

    const auto depth_stencil_state =
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

    _scene_pipeline_textured = vk::make_framed_shared<vk::layer1::pipeline>(
      *_device, *_pipeline_cache,
      vk::graphics_pipeline_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_create_flag::none,
        /* stage_count */ shader_stages_textured.size(),
        /* stages */ shader_stages_textured.data(),
        /* vertex_input_state */ &vertex_input_state_textured,
        /* input_assembly_state */ &input_assembly_state_triangles,
        /* tessellation_state */ nullptr,
        /* viewport_state */ &viewport_state,
        /* rasterization_state */ &rasterization_state_filled,
        /* multisample_state */ &multisample_state,
        /* depth_stencil_state */ &depth_stencil_state,
        /* color_blend_state */ &color_blend_state,
        /* dynamic_state */ &dynamic_state,
        /* layout */ _scene_pipeline_layout->handle(),
        /* render_pass */ _scene_render_pass->handle(),
        /* subpass */ 0,
        /* base_pipeline_handle */ nullptr,
        /* base_pipeline_index */ 0));

    _scene_pipeline_untextured = vk::make_framed_shared<vk::layer1::pipeline>(
      *_device, *_pipeline_cache,
      vk::graphics_pipeline_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_create_flag::none,
        /* stage_count */ shader_stages_untextured.size(),
        /* stages */ shader_stages_untextured.data(),
        /* vertex_input_state */ &vertex_input_state_untextured,
        /* input_assembly_state */ &input_assembly_state_triangles,
        /* tessellation_state */ nullptr,
        /* viewport_state */ &viewport_state,
        /* rasterization_state */ &rasterization_state_filled,
        /* multisample_state */ &multisample_state,
        /* depth_stencil_state */ &depth_stencil_state,
        /* color_blend_state */ &color_blend_state,
        /* dynamic_state */ &dynamic_state,
        /* layout */ _scene_pipeline_layout->handle(),
        /* render_pass */ _scene_render_pass->handle(),
        /* subpass */ 0,
        /* base_pipeline_handle */ nullptr,
        /* base_pipeline_index */ 0));

    _scene_pipeline_bounding_box = vk::make_framed_shared<vk::layer1::pipeline>(
      *_device, *_pipeline_cache,
      vk::graphics_pipeline_create_info(
        /* next */ nullptr,
        /* flags */ vk::pipeline_create_flag::none,
        /* stage_count */ shader_stages_wireframe.size(),
        /* stages */ shader_stages_wireframe.data(),
        /* vertex_input_state */ &vertex_input_state_wireframe,
        /* input_assembly_state */ &input_assembly_state_lines,
        /* tessellation_state */ nullptr,
        /* viewport_state */ &viewport_state,
        /* rasterization_state */ &rasterization_state_wireframe,
        /* multisample_state */ &multisample_state,
        /* depth_stencil_state */ &depth_stencil_state,
        /* color_blend_state */ &color_blend_state,
        /* dynamic_state */ &dynamic_state,
        /* layout */ _scene_pipeline_layout->handle(),
        /* render_pass */ _scene_render_pass->handle(),
        /* subpass */ 0,
        /* base_pipeline_handle */ nullptr,
        /* base_pipeline_index */ 0));
  }
#pragma endregion
  if (_enable_text)
    _pass_text->create_pipeline(*_pipeline_cache);
  // if (_enable_warping)
  //  _pass_warp->create_pipeline(*_pipeline_cache);

  for (auto& multiview : _multiviews)
  {
#pragma region reproject depth
    auto& reproject_depth = multiview->reproject_depth;

    reproject_depth.uniform_buffer = vk::make_framed_shared<vk::layer1::buffer>(
      *_device, vk::buffer_create_info(
                  /* next */ nullptr,
                  /* flags */ vk::buffer_create_flag::none,
                  /* size */ sizeof(global_uniform_data_t),
                  /* usage */ vk::buffer_usage_flag::uniform_buffer_bit,
                  /* sharing_mode */ vk::sharing_mode::exclusive,
                  /* queue_family_index_count */ 0,
                  /* queue_family_indices */ nullptr));
    reproject_depth.uniform_buffer->allocate_storage(
      memory_pool::pipeline_buffers);
    reproject_depth.uniform_buffer->bind_storage();

    // Prepare descriptor set.
    reproject_depth.descriptor_set =
      vk::make_framed_shared<vk::layer1::descriptor_set>(
        *_device, _descriptor_pool, *_reproject_depth_descriptor_set_layout);
    _device->update_descriptor_set(vk::write_descriptor_set(
      /* next */ nullptr,
      /* dst_set */ reproject_depth.descriptor_set->handle(),
      /* dst_binding */ 0,
      /* dst_array_element */ 0,
      /* descriptor_count */ 1,
      /* descriptor_type */ vk::descriptor_type::uniform_buffer,
      /* image_info */ nullptr,
      /* buffer_info */
      core::rvalue_address(vk::descriptor_buffer_info(
        /* buffer */ reproject_depth.uniform_buffer->handle(),
        /* offset */ 0,
        /* range */ reproject_depth.uniform_buffer->size())),
      /* texel_buffer_view */ nullptr));
    _device->update_descriptor_set(vk::write_descriptor_set(
      /* next */ nullptr,
      /* dst_set */ reproject_depth.descriptor_set->handle(),
      /* dst_binding */ 1,
      /* dst_array_element */ 0,
      /* descriptor_count */ 1,
      /* descriptor_type */ vk::descriptor_type::combined_image_sampler,
      /* image_info */
      core::rvalue_address(vk::descriptor_image_info(
        /* sampler */ _reproject_depth_sampler->handle(),
        /* image_view */ multiview->depth_image_view->handle(),
        /* image_layout */
        vk::image_layout::depth_stencil_read_only_optimal)),
      /* buffer_info */ nullptr,
      /* texel_buffer_view */ nullptr));
    _device->update_descriptor_set(vk::write_descriptor_set(
      /* next */ nullptr,
      /* dst_set */ reproject_depth.descriptor_set->handle(),
      /* dst_binding */ 2,
      /* dst_array_element */ 0,
      /* descriptor_count */ 1,
      /* descriptor_type */ vk::descriptor_type::storage_image,
      /* image_info */
      core::rvalue_address(vk::descriptor_image_info(
        /* sampler */ nullptr,
        /* image_view */ reproject_depth.r32ui_image_view->handle(),
        /* image_layout */ vk::image_layout::general)),
      /* buffer_info */ nullptr,
      /* texel_buffer_view */ nullptr));

    for (auto& view : multiview->views)
    {
      view->reproject_depth.last_inv_view_proj =
        math::make_identity_matrix<4, 4, float>();
    }
#pragma endregion

    // Create framebuffers.
    auto frame_size = multiview->scene_image->extent();

    const auto scene_attachments =
      core::make_array(multiview->scene_image_view->handle(),
                       multiview->depth_image_view->handle());
    multiview->scene_framebuffer =
      vk::make_framed_shared<vk::layer1::framebuffer>(
        *_device, vk::framebuffer_create_info(
                    /* next */ nullptr,
                    /* flags */ vk::framebuffer_create_flag::none,
                    /* render_pass */ _scene_render_pass->handle(),
                    /* attachment_count */ scene_attachments.size(),
                    /* attachments */ scene_attachments.data(),
                    /* width */ frame_size.width(),
                    /* height */ frame_size.height(),
                    /* layers */ 1u));

    if (_enable_text)
    {
      multiview->text_framebuffer =
        vk::make_framed_shared<vk::layer1::framebuffer>(
          *_device, vk::framebuffer_create_info(
                      /* next */ nullptr,
                      /* flags */ vk::framebuffer_create_flag::none,
                      /* render_pass */ _pass_text->render_pass().handle(),
                      /* attachment_count */ scene_attachments.size(),
                      /* attachments */ scene_attachments.data(),
                      /* width */ frame_size.width(),
                      /* height */ frame_size.height(),
                      /* layers */ 1u));
    }

    // if (_enable_warping)
    //{
    //  /// ToDo: warp post-processing effect requires scene_image_view as
    //  /// shader input (update_descriptor_set). how?

    //  const auto warp_attachments =
    //    core::make_array(multiview->swapchain_image_view->handle());
    //  multiview->warp_framebuffer =
    //    vk::make_framed_shared<vk::layer1::framebuffer>(
    // *_device, vk::framebuffer_create_info(
    //      /* flags */ vk::framebuffer_create_flag::none,
    //      /* render_pass */ _pass_warp->render_pass().handle(),
    //      /* attachment_count */ warp_attachments.size(),
    //      /* attachments */ warp_attachments.data(),
    //      /* width */ frame_size.width,
    //      /* height */ frame_size.height,
    //      /* layers */ 1u));
    //}
  }
}

void renderer_impl::destroy_pipeline()
{
  for (auto& multiview : _multiviews)
  {
    // if (_enable_warping)
    //  multiview->warp_framebuffer.reset();
    multiview->text_framebuffer.reset();
    multiview->scene_framebuffer.reset();

    multiview->reproject_depth.descriptor_set.reset();
    multiview->reproject_depth.uniform_buffer.reset();
  }

  // if (_enable_warping)
  //  _pass_warp->destroy_pipeline();
  if (_enable_text)
    _pass_text->destroy_pipeline();

  _scene_pipeline_bounding_box.reset();
  _scene_pipeline_untextured.reset();
  _scene_pipeline_textured.reset();
  _scene_render_pass.reset();
  _scene_pipeline_layout.reset();
  _scene_descriptor_set_layout.reset();

  _reproject_depth_sampler.reset();
  _reproject_depth_pipeline.reset();
  _reproject_depth_pipeline_layout.reset();
  _reproject_depth_descriptor_set_layout.reset();

  _pipeline_cache.reset();

  for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
       ++swapchain_index)
  {
    _submitted[swapchain_index] = false;
    _submit_fences[swapchain_index].reset();
    gsl::at(_primary_command_buffers, swapchain_index).reset();
    gsl::at(_prepare_command_buffers, swapchain_index).reset();
  }
}
}
