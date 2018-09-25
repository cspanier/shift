#include "shift/render/vk/renderer_impl.h"
#include "shift/render/vk/window_glfw.h"
#include "shift/render/vk/pass_warp.h"
#include "shift/render/vk/pass_text.h"
#include <shift/render/vk/context_impl.h>
#include <shift/render/vk/geometry_generator.h>
#include "shift/render/vk/layer2/world.h"
#include "shift/render/vk/layer2/camera.h"
#include "shift/render/vk/layer2/model.h"
#include "shift/render/vk/layer2/mesh.h"
#include "shift/render/vk/layer2/material.h"
#include "shift/render/vk/layer2/texture.h"
#include "shift/render/vk/layer2/buffer.h"
#include "shift/render/vk/layer2/view.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/layer1/instance.h"
#include <shift/resource/repository.h>
#include <shift/resource/scene.h>
#include <shift/log/log.h>
#include <boost/functional/hash/hash.hpp>

namespace std
{
template <>
struct hash<shift::resource::sampler>
{
  inline std::size_t operator()(const shift::resource::sampler& sampler) const
  {
    using sampler_address_mode_t =
      std::underlying_type_t<shift::resource::sampler_address_mode>;
    std::size_t result = 0;

    boost::hash_combine(
      result, std::hash<sampler_address_mode_t>{}(
                static_cast<sampler_address_mode_t>(sampler.address_mode_u)));
    boost::hash_combine(
      result, std::hash<sampler_address_mode_t>{}(
                static_cast<sampler_address_mode_t>(sampler.address_mode_v)));
    boost::hash_combine(
      result, std::hash<sampler_address_mode_t>{}(
                static_cast<sampler_address_mode_t>(sampler.address_mode_w)));
    boost::hash_combine(result, std::hash<float>{}(sampler.max_anisotropy));
    boost::hash_combine(result, std::hash<float>{}(sampler.min_lod));
    boost::hash_combine(result, std::hash<float>{}(sampler.max_lod));
    return result;
  }
};
}

namespace shift::render::vk
{
using namespace std::placeholders;
using namespace std::chrono_literals;

renderer_impl::renderer_impl(std::uint32_t worker_count,
                             const char* application_name,
                             std::uint32_t application_version,
                             vk::debug_layers debug_layers,
                             vk::debug_logs debug_logs)
: _worker_count(worker_count),
  _instance(vk::application_info(
              /* next */ nullptr,
              /* application_name */ application_name,
              /* application_version */ application_version,
              /* engine_name */ "shift.render.vk",
              /* engine_version */ 1,
              /* api_version */ VK_API_VERSION_1_1),
            debug_layers, debug_logs, nullptr)
{
}

renderer_impl::~renderer_impl() noexcept = default;

void renderer_impl::release()
{
  // Destroy the singleton instance.
  singleton_destroy();
}

const std::vector<vk::layer1::physical_device*>&
renderer_impl::available_physical_devices()
{
  return _instance.available_physical_devices();
}

vk::window* renderer_impl::create_glfw_window(
  math::vector2<std::int32_t> initial_position,
  math::vector2<std::uint32_t> initial_size, vk::window_flags flags)
{
  if (!_window)
  {
    _window =
      std::make_unique<vk::window_glfw>(initial_position, initial_size, flags);
  }
  return _window.get();
}

void renderer_impl::destroy_window(vk::window* window)
{
  if (_window.get() == window)
    _window.reset();
}

void renderer_impl::initialize(vk::layer1::physical_device& physical_device)
{
  /// ToDo: Use a better condition to check whether initialize has already been
  /// called.
  if (_device)
    return;

  _physical_device = &physical_device;
  if (_window)
  {
    _window->physical_device(_physical_device);
    _window->create_surface();
  }

  _device = std::make_unique<vk::layer1::device>(
    _instance, physical_device, dynamic_cast<window_glfw*>(_window.get()));

  _quit = false;
  _worker_id = 0u;
  _device->create_worker_resources(_worker_id);
  _workers.reserve(_worker_count - 1);
  for (std::uint32_t worker_id = 1u; worker_id < _worker_count; ++worker_id)
  {
    _workers.emplace_back(
      std::bind(&renderer_impl::process_worker, this, worker_id));
  }

  if (_enable_text)
    _pass_text = std::make_unique<pass_text>(*_device);
  // if (_enable_warping)
  //  _pass_warp = std::make_unique<pass_warp>(*_device);

  _default_context.initialize(*_device);
  {
    std::lock_guard lock(_context_lock);
    for (auto& context : _contexts)
      context->initialize(*_device);
  }
  if (_frame_end_semaphore)
    _frame_end_semaphore.reset();
  _frame_end_semaphore = vk::layer1::semaphore(*_device);

  _default_context.begin();
  create_swapchain();
  create_descriptor_pool();
  create_pipeline();
  _default_context.end();
}

void renderer_impl::finalize()
{
  if (!_device)
    return;

  {
    std::lock_guard render_graph_lock(_render_graph_mutex);
    _models.clear();
    _materials.clear();
    _samplers.clear();
    _textures.clear();
    _meshes.clear();
    _buffers.clear();
  }

  // Simply wait for all work to finish.
  _device->wait_idle();

  destroy_pipeline();
  destroy_descriptor_pool();
  destroy_swapchain();

  _frame_end_semaphore.reset();
  {
    std::lock_guard lock(_context_lock);
    for (auto& context : _contexts)
      context->finalize();
  }
  _default_context.finalize();

  // Destroy resources created during renderer_impl::update.
  for (auto& multiview : _multiviews)
  {
    multiview->reproject_depth.command_buffer.reset();
    for (auto& view_command_buffer : multiview->command_buffers)
      view_command_buffer.reset();
  }

  destroy_resources();
  // if (_enable_warping)
  //  _pass_warp.reset();
  if (_enable_text)
    _pass_text.reset();
  if (_window)
  {
    _window->destroy_surface();
    _window->physical_device(nullptr);
  }
  _physical_device = nullptr;

  bool repeat;
  do
  {
    repeat = shared_object_queue::collect() || fenced_object_queue::collect();
    for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
         ++swapchain_index)
    {
      repeat |= framed_object_queue::collect(swapchain_index);
    }
  } while (repeat);

  _quit = true;
  _worker_condition.notify_all();
  // Wait for worker threads to quit.
  for (auto& worker : _workers)
    worker.join();
  _device->destroy_worker_resources(_worker_id);

  _device.reset();
}

void renderer_impl::wait_idle()
{
  if (_device)
    _device->wait_idle();
}

vk::layer2::world* renderer_impl::create_world()
{
  auto new_world = std::make_unique<vk::layer2::world>();
  auto* result = new_world.get();
  _worlds.emplace_back(std::move(new_world));
  return result;
}

void renderer_impl::destroy_world(vk::layer2::world* world)
{
  for (auto world_iterator = _worlds.begin(); world_iterator != _worlds.end();
       ++world_iterator)
  {
    if (world_iterator->get() == world)
    {
      _worlds.erase(world_iterator);
      return;
    }
  }
}

void renderer_impl::destroy_all_worlds()
{
  BOOST_ASSERT(_multiviews.empty());
}

vk::layer2::view* renderer_impl::create_view(
  const view_create_param& create_param)
{
  BOOST_ASSERT(_allow_view_changes);

  auto new_multiview = std::make_unique<vk::layer2::multiview>();
  auto new_view = std::make_unique<vk::layer2::view>();
  new_view->multiview = new_multiview.get();
  new_view->relative_position = create_param.relative_position;
  new_view->camera = create_param.camera;

  auto* result = new_view.get();
  new_multiview->views.emplace_back(std::move(new_view));
  new_multiview->relative_size = create_param.relative_size;
  _multiviews.emplace_back(std::move(new_multiview));
  return result;
}

void renderer_impl::destroy_view(vk::layer2::view* view)
{
  BOOST_ASSERT(_allow_view_changes);
  for (auto multiview_iterator = _multiviews.begin();
       multiview_iterator != _multiviews.end(); ++multiview_iterator)
  {
    if ((*multiview_iterator)->views.size() == 1 &&
        (*multiview_iterator)->views.front().get() == view)
    {
      _multiviews.erase(multiview_iterator);
      return;
    }
  }

  /// ToDo: Throw exception.
}

vk::layer2::multiview* renderer_impl::create_multiview(
  const multiview_create_param& create_param)
{
  BOOST_ASSERT(_allow_view_changes);
  BOOST_ASSERT(create_param.view_create_params.size() > 1u);
  BOOST_ASSERT(
    create_param.views.empty() ||
    (create_param.view_create_params.size() == create_param.views.size()));

  auto new_multiview = std::make_unique<vk::layer2::multiview>();
  std::ptrdiff_t view_index = 0;
  for (auto& view_create_param : create_param.view_create_params)
  {
    auto new_view = std::make_unique<vk::layer2::view>();
    new_view->multiview = new_multiview.get();
    new_view->relative_position = view_create_param.relative_position;
    new_view->camera = view_create_param.camera;

    if (!create_param.views.empty())
      create_param.views[view_index] = new_view.get();
    new_multiview->views.emplace_back(std::move(new_view));
    if (view_index == 0u)
      new_multiview->relative_size = view_create_param.relative_size;
    else
    {
      BOOST_ASSERT(new_multiview->relative_size ==
                   view_create_param.relative_size);
    }
    ++view_index;
  }

  auto* result = new_multiview.get();
  _multiviews.emplace_back(std::move(new_multiview));
  return result;
}

void renderer_impl::destroy_multiview(vk::layer2::multiview* multiview)
{
  BOOST_ASSERT(_allow_view_changes);
  for (auto multiview_iterator = _multiviews.begin();
       multiview_iterator != _multiviews.end(); ++multiview_iterator)
  {
    if (multiview_iterator->get() == multiview)
    {
      _multiviews.erase(multiview_iterator);
      return;
    }
  }

  /// ToDo: Throw exception.
}

void renderer_impl::destroy_all_views()
{
  BOOST_ASSERT(_allow_view_changes);
  _multiviews.clear();
}

boost::intrusive_ptr<vk::context> renderer_impl::create_context()
{
  auto context = std::make_unique<vk::context_impl>();
  if (_device != nullptr)
    context->initialize(*_device);

  std::lock_guard lock(_context_lock);
  _contexts.emplace_back(context.get());
  return boost::intrusive_ptr<vk::context>(context.release(), false);
}

void renderer_impl::destroy_context(gsl::owner<vk::context*> context)
{
  auto* actual_context = dynamic_cast<context_impl*>(context);
  BOOST_ASSERT(actual_context != nullptr);
  if (actual_context == nullptr)
    return;

  if (_device != nullptr)
    actual_context->finalize();

  {
    std::lock_guard lock(_context_lock);
    _contexts.erase(std::remove_if(_contexts.begin(), _contexts.end(),
                                   [&](const auto* other) {
                                     return other == actual_context;
                                   }),
                    _contexts.end());
  }

  delete context;
}

boost::intrusive_ptr<vk::buffer> renderer_impl::create_buffer(
  vk::buffer_usage_flags usage,
  std::shared_ptr<resource::buffer>& source_buffer)
{
  std::lock_guard render_graph_lock(_render_graph_mutex);
  return create_buffer(usage, source_buffer, render_graph_lock);
}

boost::intrusive_ptr<vk::mesh> renderer_impl::create_mesh(
  const std::shared_ptr<resource::mesh>& source_mesh)
{
  std::lock_guard render_graph_lock(_render_graph_mutex);
  return create_mesh(source_mesh, render_graph_lock);
}

boost::intrusive_ptr<vk::texture> renderer_impl::create_texture(
  std::shared_ptr<resource::image>& source_texture)
{
  std::lock_guard render_graph_lock(_render_graph_mutex);
  return create_texture(source_texture, render_graph_lock);
}

boost::intrusive_ptr<vk::material> renderer_impl::create_material(
  const std::shared_ptr<resource::material>& source_material)
{
  std::lock_guard render_graph_lock(_render_graph_mutex);
  return create_material(source_material, render_graph_lock);
}

boost::intrusive_ptr<vk::model> renderer_impl::create_model(
  const std::shared_ptr<resource::mesh>& source_mesh,
  const std::shared_ptr<resource::material>& source_material)
{
  std::lock_guard render_graph_lock(_render_graph_mutex);
  return create_model(source_mesh, source_material, render_graph_lock);
}

void renderer_impl::begin_resize()
{
  _default_context.begin();
  // Wait until all work has been processed so we can safely destroy stuff.
  _device->wait_idle();
  for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
       ++swapchain_index)
  {
    _submitted[swapchain_index] = false;
  }

  destroy_pipeline();
  // destroy_descriptor_pool();
  destroy_swapchain();

  bool repeat;
  do
  {
    repeat = shared_object_queue::collect() || fenced_object_queue::collect();
    for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
         ++swapchain_index)
    {
      repeat |= framed_object_queue::collect(swapchain_index);
    }
  } while (repeat);
}

void renderer_impl::end_resize()
{
  create_swapchain();
  // create_descriptor_pool();
  create_pipeline();
  _default_context.end();
}

void renderer_impl::begin_frame()
{
  update_transfer_jobs();
}

void renderer_impl::end_frame()
{
  // Wait until the next swapchain's image is ready.
  if (_submitted[_swapchain_index])
  {
    switch (_device->wait(*_submit_fences[_swapchain_index], 1s))
    {
    case vk::result::success:
      break;

    case vk::result::timeout:
      log::warning() << "timeout on submit fence!";
      /// ToDo: What to do? Simply skip frame for now.
      return;

    case vk::result::error_device_lost:
      log::warning() << "device lost!";
      /// ToDo: Refactor device loss code path. A simple resize is most likely
      /// not enough because we lost the whole device.
      _window->on_resize(_window->size());
      return;

    case vk::result::error_out_of_device_memory:
      log::error() << "out of device memory!";
      /// ToDo: What to do? Simply skip frame for now.
      return;

    case vk::result::error_out_of_host_memory:
      log::error() << "out of host memory!";
      /// ToDo: What to do? Simply skip frame for now.
      return;

    default:
      BOOST_ASSERT(false);
    }
    _submitted[_swapchain_index] = false;
    _submit_fences[_swapchain_index]->reset();
  }
  auto frame_index = _frame_counter % config::frame_count;

  shared_object_queue::collect();
  framed_object_queue::collect(_swapchain_index);
  fenced_object_queue::collect();

#pragma region Wait for and prepare swapchain image.
  auto& prepare_command_buffer = *_prepare_command_buffers[_swapchain_index];
  prepare_command_buffer.begin(vk::command_buffer_begin_info(
    /* next */ nullptr,
    /* flags */ vk::command_buffer_usage_flag::none,
    /* inheritance_info */ nullptr));
  prepare_command_buffer.pipeline_barrier(
    _window->present_stage_flags(), vk::pipeline_stage_flag::transfer_bit,
    vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ _window->present_access_flags(),
      /* dst_access_mask */ vk::access_flag::transfer_write_bit,
      /* old_layout */ _window->present_image_layout(),
      /* new_layout */ vk::image_layout::transfer_dst_optimal,
      /* src_queue_family_index */
      _device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      _device->graphics_queue_family_index(),
      /* image */ _swapchain_images[_swapchain_index]->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0u,
        /* level_count */ 1u,
        /* base_array_layer */ 0u,
        /* layer_count */ 1u)));
  for (auto& multiview : _multiviews)
  {
    // Prepare multiview->depth_image to be read from reproject_depth compute
    // shader.
    prepare_command_buffer.pipeline_barrier(
      vk::pipeline_stage_flag::early_fragment_tests_bit,
      vk::pipeline_stage_flag::compute_shader_bit, vk::dependency_flag::none,
      vk::image_memory_barrier(
        /* next */ nullptr,
        /* src_access_mask */
        vk::access_flag::depth_stencil_attachment_write_bit,
        /* dst_access_mask */ vk::access_flag::shader_read_bit,
        /* old_layout */ vk::image_layout::depth_stencil_attachment_optimal,
        /* new_layout */ vk::image_layout::depth_stencil_read_only_optimal,
        /* src_queue_family_index */ _device->graphics_queue_family_index(),
        /* dst_queue_family_index */ _device->graphics_queue_family_index(),
        /* image */ multiview->depth_image->handle(),
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::depth_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1u)));
  }
  prepare_command_buffer.end();

  const auto wait_dst_stage_masks = core::make_array<vk::pipeline_stage_flags>(
    vk::pipeline_stage_flag::bottom_of_pipe_bit);
  /// ToDo: Get rid of these dynamic allocations!
  std::vector<VkSemaphore> prepared_semaphores;
  std::vector<VkSemaphore> view_end_semaphores;
  prepared_semaphores.reserve(_multiviews.size());
  view_end_semaphores.reserve(_multiviews.size());
  for (const auto& multiview : _multiviews)
  {
    prepared_semaphores.push_back(multiview->prepared.handle());
    view_end_semaphores.push_back(multiview->reproject_depth.finished.handle());
  }

  const auto present_complete_semaphores =
    core::make_array(_window->present_complete_semaphore());
  _device->graphics_queue()->submit(
    vk::submit_info(
      /* next */ nullptr,
      /* wait_semaphore_count */ present_complete_semaphores.size(),
      /* wait_semaphores */ present_complete_semaphores.data(),
      /* wait_dst_stage_mask */ wait_dst_stage_masks.data(),
      /* command_buffer_count */ 1u,
      /* command_buffers */ &prepare_command_buffer.handle(),
      /* signal_semaphore_count */
      static_cast<std::uint32_t>(prepared_semaphores.size()),
      /* signal_semaphores */ prepared_semaphores.data()),
    vk::layer1::fence::null_handle);
#pragma endregion

#pragma region reproject depth
  for (auto& multiview : _multiviews)
  {
    auto& reproject_depth = multiview->reproject_depth;

    auto source_extent = multiview->depth_image->extent();
    auto source_size =
      math::make_vector_from(source_extent.width(), source_extent.height());
    auto target_size = source_size / 4;

    auto* mapped_data = static_cast<global_uniform_data_t*>(
      reproject_depth.uniform_buffer->address());
    mapped_data->last_inv_view_proj =
      math::make_identity_matrix<4, 4,
                                 float>();  // multiview->last_inv_view_proj;
    mapped_data->view_proj = math::make_identity_matrix<4, 4, float>();
    // projection_matrix() * view_matrix();
    mapped_data->source_size = math::static_cast_vector<float>(source_size);
    mapped_data->target_size = math::static_cast_vector<float>(target_size);
    for (auto view_index = 0u; view_index < multiview->views.size();
         ++view_index)
    {
      multiview->views[view_index]->reproject_depth.last_inv_view_proj =
        math::inverse(mapped_data->view_proj);
    }

    if (!reproject_depth.command_buffer)
    {
      reproject_depth.command_buffer =
        vk::make_framed_shared<vk::layer1::command_buffer>(
          _device->select_command_pool(vk::layer1::queue_type::compute, false),
          vk::command_buffer_level::primary);
      reproject_depth.command_buffer->begin(vk::command_buffer_begin_info(
        /* next */ nullptr,
        /* flags */ vk::command_buffer_usage_flag::simultaneous_use_bit,
        /* inheritance_info */ nullptr));

      reproject_depth.command_buffer->pipeline_barrier(
        vk::pipeline_stage_flag::compute_shader_bit,
        vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
        vk::image_memory_barrier(
          /* next */ nullptr,
          /* src_access_mask */ vk::access_flag::shader_write_bit,
          /* dst_access_mask */ vk::access_flag::transfer_write_bit,
          /* old_layout */ vk::image_layout::general,
          /* new_layout */ vk::image_layout::transfer_dst_optimal,
          /* src_queue_family_index */
          _device->compute_queue_family_index(),
          /* dst_queue_family_index */
          _device->compute_queue_family_index(),
          /* image */ reproject_depth.r32ui_image->handle(),
          /* subresource_range */
          vk::image_subresource_range(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* base_mip_level */ 0,
            /* level_count */ 1,
            /* base_array_layer */ 0,
            /* layer_count */ 1)));

      reproject_depth.command_buffer->clear_color_image(
        *reproject_depth.r32ui_image, vk::image_layout::transfer_dst_optimal,
        vk::clear_color_value{core::make_array(0u, 0u, 0u, 0u)}, 1,
        core::rvalue_address(vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::color_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1u)));

      reproject_depth.command_buffer->pipeline_barrier(
        vk::pipeline_stage_flag::transfer_bit,
        vk::pipeline_stage_flag::compute_shader_bit, vk::dependency_flag::none,
        vk::image_memory_barrier(
          /* next */ nullptr,
          /* src_access_mask */ vk::access_flag::transfer_write_bit,
          /* dst_access_mask */ vk::access_flag::shader_write_bit,
          /* old_layout */ vk::image_layout::transfer_dst_optimal,
          /* new_layout */ vk::image_layout::general,
          /* src_queue_family_index */
          _device->compute_queue_family_index(),
          /* dst_queue_family_index */
          _device->compute_queue_family_index(),
          /* image */ reproject_depth.r32ui_image->handle(),
          /* subresource_range */
          vk::image_subresource_range(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* base_mip_level */ 0,
            /* level_count */ 1,
            /* base_array_layer */ 0,
            /* layer_count */ 1)));

      // reproject_depth.command_buffer->pipeline_barrier(
      //  vk::pipeline_stage_flag::early_fragment_tests_bit,
      //  vk::pipeline_stage_flag::compute_shader_bit,
      //  vk::dependency_flag::none, vk::image_memory_barrier(
      //    /* next */ nullptr,
      //    /* src_access_mask */
      //    vk::access_flag::depth_stencil_attachment_write_bit,
      //    /* dst_access_mask */ vk::access_flag::shader_read_bit,
      //    /* old_layout */ vk::image_layout::depth_stencil_attachment_optimal,
      //    /* new_layout */ vk::image_layout::depth_stencil_read_only_optimal,
      //    /* src_queue_family_index */ _device->graphics_queue_family_index(),
      //    /* dst_queue_family_index */ _device->compute_queue_family_index(),
      //    /* image */ multiview->depth_image->handle(),
      //    /* subresource_range */
      //    vk::image_subresource_range(
      //      /* aspect_mask */ vk::image_aspect_flag::depth_bit,
      //      /* base_mip_level */ 0u,
      //      /* level_count */ 1u,
      //      /* base_array_layer */ 0u,
      //      /* layer_count */ 1u)));

      reproject_depth.command_buffer->bind_pipeline(
        vk::pipeline_bind_point::compute, *_reproject_depth_pipeline);

      reproject_depth.command_buffer->bind_descriptor_sets(
        vk::pipeline_bind_point::compute, *_reproject_depth_pipeline_layout, 0,
        core::make_array(reproject_depth.descriptor_set));

      reproject_depth.command_buffer->dispatch(target_size.x / 16u,
                                               target_size.y / 16u, 1u);

      // reproject_depth.command_buffer->pipeline_barrier(
      //  vk::pipeline_stage_flag::compute_shader_bit,
      //  vk::pipeline_stage_flag::late_fragment_tests_bit,
      //  vk::dependency_flag::none,
      //  vk::image_memory_barrier(
      //    /* next */ nullptr,
      //    /* src_access_mask */ vk::access_flag::shader_read_bit,
      //    /* dst_access_mask */
      //    vk::access_flag::depth_stencil_attachment_write_bit,
      //    /* old_layout */ vk::image_layout::depth_stencil_read_only_optimal,
      //    /* new_layout */ vk::image_layout::depth_stencil_attachment_optimal,
      //    /* src_queue_family_index */ _device->compute_queue_family_index(),
      //    /* dst_queue_family_index */ _device->graphics_queue_family_index(),
      //    /* image */ multiview->depth_image->handle(),
      //    /* subresource_range */
      //    vk::image_subresource_range(
      //      /* aspect_mask */ vk::image_aspect_flag::depth_bit,
      //      /* base_mip_level */ 0u,
      //      /* level_count */ 1u,
      //      /* base_array_layer */ 0u,
      //      /* layer_count */ 1u)));

      reproject_depth.command_buffer->end();
    }
    _device->compute_queue()->submit(
      vk::submit_info(
        /* next */ nullptr,
        /* wait_semaphore_count */
        static_cast<std::uint32_t>(prepared_semaphores.size()),
        /* wait_semaphores */ prepared_semaphores.data(),
        /* wait_dst_stage_mask */ wait_dst_stage_masks.data(),
        /* command_buffer_count */ 1u,
        /* command_buffers */
        &multiview->reproject_depth.command_buffer->handle(),
        /* signal_semaphore_count */ 1u,
        /* signal_semaphores */ &multiview->reproject_depth.finished.handle()),
      vk::layer1::fence::null_handle);
  }
#pragma endregion

  auto& primary_command_buffer = *_primary_command_buffers[_swapchain_index];
  primary_command_buffer.begin(vk::command_buffer_begin_info(
    /* next */ nullptr,
    /* flags */ vk::command_buffer_usage_flag::none,
    /* inheritance_info */ nullptr));
  for (auto& multiview : _multiviews)
  {
    auto frame_size = multiview->scene_image->extent();

    // Transform multiview->depth_image back into attachment_optimal layout.
    primary_command_buffer.pipeline_barrier(
      vk::pipeline_stage_flag::compute_shader_bit,
      vk::pipeline_stage_flag::late_fragment_tests_bit,
      vk::dependency_flag::none,
      vk::image_memory_barrier(
        /* next */ nullptr,
        /* src_access_mask */ vk::access_flag::shader_read_bit,
        /* dst_access_mask */
        vk::access_flag::depth_stencil_attachment_write_bit,
        /* old_layout */ vk::image_layout::depth_stencil_read_only_optimal,
        /* new_layout */ vk::image_layout::depth_stencil_attachment_optimal,
        /* src_queue_family_index */ _device->graphics_queue_family_index(),
        /* dst_queue_family_index */ _device->graphics_queue_family_index(),
        /* image */ multiview->depth_image->handle(),
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::depth_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1u)));

#pragma region scene
    auto extent = multiview->scene_image->extent();

    auto* mapped_data = static_cast<mesh_uniform_data*>(
      _scene_uniform_buffers[frame_index]->address());
    mapped_data->view[0] = _view_matrix;
    mapped_data->projection[0] = _projection_matrix;

    auto& view_command_buffer = multiview->command_buffers[frame_index];
    if (!view_command_buffer)
    {
      view_command_buffer = vk::make_framed_shared<vk::layer1::command_buffer>(
        _device->select_command_pool(vk::layer1::queue_type::graphics, true),
        vk::command_buffer_level::secondary);
    }
    else
      view_command_buffer->reset(vk::command_buffer_reset_flag::none);
    view_command_buffer->begin(vk::command_buffer_begin_info(
      /* next */ nullptr,
      /* flags */
      vk::command_buffer_usage_flag::one_time_submit_bit |
        vk::command_buffer_usage_flag::render_pass_continue_bit,
      /* inheritance_info */
      core::rvalue_address(vk::command_buffer_inheritance_info(
        /* next */ nullptr,
        /* render_pass */ _scene_render_pass->handle(),
        /* subpass */ 0,
        /* framebuffer */ multiview->scene_framebuffer->handle(),
        /* occlusion_query_enable */ VK_FALSE,
        /* query_flags */ vk::query_control_flag::none,
        /* pipeline_statistics */ vk::query_pipeline_statistic_flag::none))));

    const auto viewports = core::make_array(vk::viewport(
      /* x */ 0.0f,
      /* y */ 0.0f,
      /* width */ static_cast<float>(extent.width()),
      /* height */ static_cast<float>(extent.height()),
      /* min_depth */ 0.0f,
      /* max_depth */ 1.0f));
    view_command_buffer->set_viewports(0, viewports);

    const auto scissors = core::make_array(vk::rect_2d(
      /* offset */ vk::offset_2d{0, 0},
      /* extent */ vk::extent_2d{extent.width(), extent.height()}));
    view_command_buffer->set_scissors(0, scissors);

    view_command_buffer->bind_pipeline(vk::pipeline_bind_point::graphics,
                                       *_scene_pipeline_textured);
    for (const auto* model : _scene_models_textured)
    {
      const auto* mesh = model->mesh;
      view_command_buffer->bind_descriptor_sets(
        vk::pipeline_bind_point::graphics, *_scene_pipeline_layout, 0,
        core::make_array(model->descriptor_sets[frame_index]));

      view_command_buffer->bind_vertex_buffers(
        0,
        core::make_array(mesh->position_buffer.get(), mesh->normal_buffer.get(),
                         mesh->texcoord_buffer.get(),
                         model->instance_buffer.get()),
        core::make_array(VkDeviceSize{mesh->position_offset},
                         VkDeviceSize{mesh->normal_offset},
                         VkDeviceSize{mesh->texcoord_offset}, VkDeviceSize{0}));
      view_command_buffer->bind_index_buffer(*mesh->index_buffer, 0,
                                             vk::index_type::uint32);

      for (const auto& sub_mesh : mesh->resource_mesh->sub_meshes)
      {
        view_command_buffer->draw_indexed(
          sub_mesh.index_count,
          static_cast<std::uint32_t>(model->instances.size()),
          sub_mesh.first_index,
          static_cast<std::int32_t>(sub_mesh.vertex_offset), 0);
      }
    }

    view_command_buffer->bind_pipeline(vk::pipeline_bind_point::graphics,
                                       *_scene_pipeline_untextured);
    for (const auto* model : _scene_models_untextured)
    {
      const auto* mesh = model->mesh;
      view_command_buffer->bind_descriptor_sets(
        vk::pipeline_bind_point::graphics, *_scene_pipeline_layout, 0,
        core::make_array(model->descriptor_sets[frame_index]));

      view_command_buffer->bind_vertex_buffers(
        0,
        core::make_array(mesh->position_buffer.get(), mesh->normal_buffer.get(),
                         model->instance_buffer.get()),
        core::make_array(VkDeviceSize{mesh->position_offset},
                         VkDeviceSize{mesh->normal_offset}, VkDeviceSize{0}));
      view_command_buffer->bind_index_buffer(*mesh->index_buffer, 0,
                                             vk::index_type::uint32);

      for (const auto& sub_mesh : mesh->resource_mesh->sub_meshes)
      {
        view_command_buffer->draw_indexed(
          sub_mesh.index_count,
          static_cast<std::uint32_t>(model->instances.size()),
          sub_mesh.first_index,
          static_cast<std::int32_t>(sub_mesh.vertex_offset), 0);
      }
    }

    if (_show_bounding_boxes)
    {
      view_command_buffer->bind_pipeline(vk::pipeline_bind_point::graphics,
                                         *_scene_pipeline_bounding_box);
      view_command_buffer->bind_descriptor_sets(
        vk::pipeline_bind_point::graphics, *_scene_pipeline_layout, 0,
        core::make_array(_scene_bounding_boxes.descriptor_sets[frame_index]));

      view_command_buffer->bind_vertex_buffers(
        0,
        core::make_array(_scene_bounding_boxes.vertex_buffer.get(),
                         _scene_bounding_boxes.instance_buffer.get()),
        core::make_array(VkDeviceSize{0}, VkDeviceSize{0}));
      view_command_buffer->bind_index_buffer(
        *_scene_bounding_boxes.index_buffer, 0, vk::index_type::uint32);

      view_command_buffer->draw_indexed(
        8 * 3 * 2,
        static_cast<std::uint32_t>(_scene_bounding_boxes.instances.size()), 0,
        0, 0);
    }

    view_command_buffer->end();
#pragma endregion
    if (_enable_text)
      _pass_text->update(*multiview, frame_index);
    // if (_enable_warping)
    //  _pass_warp->update(_swapchain_index);

    // Scene rendering:
    const auto render_area = vk::rect_2d(
      /* offset */ vk::offset_2d{0, 0},
      /* extent */ vk::extent_2d{frame_size.width(), frame_size.height()});
    const auto clear_values = core::make_array(
      vk::clear_value(
        vk::clear_color_value(core::make_array(0.2f, 0.2f, 0.2f, 0.0f))),
      vk::clear_value(vk::clear_depth_stencil_value(
        /* depth */ 1.0f,
        /* stencil */ 0)));
    primary_command_buffer.begin_render_pass(
      *_scene_render_pass, *multiview->scene_framebuffer, render_area,
      clear_values.size(), clear_values.data(),
      vk::subpass_contents::secondary_command_buffers);
    primary_command_buffer.execute_commands(*view_command_buffer);
    primary_command_buffer.end_render_pass();
  }

#pragma region Blit each view image to the swapchain image
  const auto swapchain_image_size =
    _swapchain_images[_swapchain_index]->extent();
  for (auto& multiview : _multiviews)
  {
    auto frame_size = multiview->scene_image->extent();

    primary_command_buffer.pipeline_barrier(
      vk::pipeline_stage_flag::color_attachment_output_bit,
      vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
      vk::image_memory_barrier(
        /* next */ nullptr,
        /* src_access_mask */ vk::access_flag::color_attachment_read_bit |
          vk::access_flag::color_attachment_write_bit,
        /* dst_access_mask */ vk::access_flag::transfer_read_bit,
        /* old_layout */ vk::image_layout::color_attachment_optimal,
        /* new_layout */ vk::image_layout::transfer_src_optimal,
        /* src_queue_family_index */
        _device->graphics_queue_family_index(),
        /* dst_queue_family_index */
        _device->graphics_queue_family_index(),
        /* image */ multiview->scene_image->handle(),
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::color_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1)));
    for (const auto& view : multiview->views)
    {
      auto frame_position = math::make_vector_from(
        static_cast<std::int32_t>(swapchain_image_size.width() *
                                  view->relative_position.x),
        static_cast<std::int32_t>(swapchain_image_size.height() *
                                  view->relative_position.y));
      primary_command_buffer.blit_image(
        *multiview->scene_image, vk::image_layout::transfer_src_optimal,
        *_swapchain_images[_swapchain_index],
        vk::image_layout::transfer_dst_optimal,
        vk::image_blit(
          /* src_subresource */
          vk::image_subresource_layers(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* mip_level */ 0u,
            /* base_array_layer */ 0u,
            /* layer_count */ 1u),
          /* src_offsets */
          core::make_array(
            vk::offset_3d{0, 0, 0},
            vk::offset_3d{static_cast<std::int32_t>(frame_size.width()),
                          static_cast<std::int32_t>(frame_size.height()), 1}),
          /* dst_subresource */
          vk::image_subresource_layers(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* mip_level */ 0u,
            /* base_array_layer */ 0u,
            /* layer_count */ 1u),
          /* dst_offsets */
          core::make_array(
            vk::offset_3d{frame_position.x, frame_position.y, 0},
            vk::offset_3d{
              frame_position.x + static_cast<std::int32_t>(frame_size.width()),
              frame_position.y + static_cast<std::int32_t>(frame_size.height()),
              1})),
        vk::filter::nearest);
    }
    primary_command_buffer.pipeline_barrier(
      vk::pipeline_stage_flag::transfer_bit,
      vk::pipeline_stage_flag::color_attachment_output_bit,
      vk::dependency_flag::none,
      vk::image_memory_barrier(
        /* next */ nullptr,
        /* src_access_mask */ vk::access_flag::transfer_read_bit,
        /* dst_access_mask */ vk::access_flag::color_attachment_read_bit |
          vk::access_flag::color_attachment_write_bit,
        /* old_layout */ vk::image_layout::transfer_src_optimal,
        /* new_layout */ vk::image_layout::color_attachment_optimal,
        /* src_queue_family_index */
        _device->graphics_queue_family_index(),
        /* dst_queue_family_index */
        _device->graphics_queue_family_index(),
        /* image */ multiview->scene_image->handle(),
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::color_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1u)));

    // if constexpr(config::debug_reproject_depth)
    //{
    //  primary_command_buffer.pipeline_barrier(
    //    vk::pipeline_stage_flag::compute_shader_bit,
    //    vk::pipeline_stage_flag::transfer_bit, vk::dependency_flag::none,
    //    vk::image_memory_barrier(
    //      /* src_access_mask */ vk::access_flag::shader_write_bit,
    //      /* dst_access_mask */ vk::access_flag::transfer_read_bit,
    //      /* old_layout */ vk::image_layout::general,
    //      /* new_layout */ vk::image_layout::transfer_src_optimal,
    //      /* src_queue_family_index */ _device->compute_queue_family_index(),
    //      /* dst_queue_family_index */ _device->graphics_queue_family_index(),
    //      /* image */ multiview->reproject_depth.depth_image->handle(),
    //      /* subresource_range */
    //      vk::image_subresource_range(
    //        /* aspect_mask */ vk::image_aspect_flag::depth_bit,
    //        /* base_mip_level */ 0u,
    //        /* level_count */ 1u,
    //        /* base_array_layer */ 0u,
    //        /* layer_count */ 1u)));
    //  primary_command_buffer.blit_image(
    //    *multiview->reproject_depth.depth_image,
    //    vk::image_layout::transfer_src_optimal,
    //    *_swapchain_images[_swapchain_index],
    //    vk::image_layout::transfer_dst_optimal,
    //    vk::image_blit(
    //      /* src_subresource */
    //      vk::image_subresource_layers(
    //        /* aspect_mask */ vk::image_aspect_flag::depth_bit,
    //        /* mip_level */ 0,
    //        /* base_array_layer */ 0,
    //        /* layer_count */ 1),
    //      /* src_offsets */
    //      core::make_array(
    //        vk::offset_3d{0, 0, 0},
    //        vk::offset_3d{static_cast<std::int32_t>(
    //                        multiview->reproject_depth.depth_image->extent().width()),
    //                      static_cast<std::int32_t>(
    //                        multiview->reproject_depth.depth_image->extent().height()),
    //                      1}),
    //      /* dst_subresource */
    //      vk::image_subresource_layers(
    //        /* aspect_mask */ vk::image_aspect_flag::color_bit,
    //        /* mip_level */ 0,
    //        /* base_array_layer */ 0,
    //        /* layer_count */ 1),
    //      /* dst_offsets */
    //      core::make_array(
    //        vk::offset_3d{0, 0, 0},
    //        vk::offset_3d{static_cast<std::int32_t>(window_size.width / 4),
    //                      static_cast<std::int32_t>(window_size.height / 4),
    //                      1})),
    //    vk::filter::nearest);
    //  primary_command_buffer.pipeline_barrier(
    //    vk::pipeline_stage_flag::transfer_bit,
    //    vk::pipeline_stage_flag::compute_shader_bit,
    //    vk::dependency_flag::none, vk::image_memory_barrier(
    //      /* src_access_mask */ vk::access_flag::transfer_read_bit,
    //      /* dst_access_mask */ vk::access_flag::shader_write_bit,
    //      /* old_layout */ vk::image_layout::transfer_src_optimal,
    //      /* new_layout */ vk::image_layout::general,
    //      /* src_queue_family_index */ _device->graphics_queue_family_index(),
    //      /* dst_queue_family_index */ _device->compute_queue_family_index(),
    //      /* image */ multiview->reproject_depth.depth_image->handle(),
    //      /* subresource_range */
    //      vk::image_subresource_range(
    //        /* aspect_mask */ vk::image_aspect_flag::depth_bit,
    //        /* base_mip_level */ 0u,
    //        /* level_count */ 1u,
    //        /* base_array_layer */ 0u,
    //        /* layer_count */ 1u)));
    //}
  }
  primary_command_buffer.pipeline_barrier(
    vk::pipeline_stage_flag::transfer_bit, _window->present_stage_flags(),
    vk::dependency_flag::none,
    vk::image_memory_barrier(
      /* next */ nullptr,
      /* src_access_mask */ vk::access_flag::transfer_write_bit,
      /* dst_access_mask */ _window->present_access_flags(),
      /* old_layout */ vk::image_layout::transfer_dst_optimal,
      /* new_layout */ _window->present_image_layout(),
      /* src_queue_family_index */
      _device->graphics_queue_family_index(),
      /* dst_queue_family_index */
      _device->graphics_queue_family_index(),
      /* image */ _swapchain_images[_swapchain_index]->handle(),
      /* subresource_range */
      vk::image_subresource_range(
        /* aspect_mask */ vk::image_aspect_flag::color_bit,
        /* base_mip_level */ 0u,
        /* level_count */ 1u,
        /* base_array_layer */ 0u,
        /* layer_count */ 1u)));
#pragma endregion
  primary_command_buffer.end();

  // Queue primary command buffer to graphics queue.
  _device->graphics_queue()->submit(
    vk::submit_info(
      /* next */ nullptr,
      /* wait_semaphore_count */
      static_cast<std::uint32_t>(view_end_semaphores.size()),
      /* wait_semaphores */ view_end_semaphores.data(),
      /* wait_dst_stage_mask */ wait_dst_stage_masks.data(),
      /* command_buffer_count */ 1u,
      /* command_buffers */ &primary_command_buffer.handle(),
      /* signal_semaphore_count */ 1u,
      /* signal_semaphores */ &_frame_end_semaphore.handle()),
    _submit_fences[_swapchain_index]->handle());
  _submitted[_swapchain_index] = true;

  if (_window->present(
        1u, reinterpret_cast<std::uint64_t*>(&_frame_end_semaphore.handle()),
        _swapchain_index))
  {
    _swapchain_index = _window->acquire_next_image();
    BOOST_ASSERT(_swapchain_index < _swapchain_images.size());
    ++_frame_counter;
  }
  // api_object_pool::collect_garbage(_swapchain_index);
}

boost::intrusive_ptr<vk::layer2::buffer> renderer_impl::create_buffer(
  vk::buffer_usage_flags usage,
  std::shared_ptr<resource::buffer>& source_buffer,
  const std::lock_guard<std::mutex>& /*render_graph_lock*/)
{
  if (auto existing_buffer_iter = _buffers.find(source_buffer.get());
      existing_buffer_iter != _buffers.end())
  {
    return existing_buffer_iter->second;
  }

  boost::intrusive_ptr<vk::layer2::buffer> new_buffer(
    new vk::layer2::buffer(*_device, source_buffer->storage.size(), usage,
                           source_buffer),
    false);
  _buffers.insert_or_assign(source_buffer.get(), new_buffer);
  async_load(*new_buffer);
  return new_buffer;
}

boost::intrusive_ptr<vk::layer2::mesh> renderer_impl::create_mesh(
  const std::shared_ptr<resource::mesh>& source_mesh,
  const std::lock_guard<std::mutex>& render_graph_lock)
{
  if (auto existing_mesh_iter = _meshes.find(source_mesh.get());
      existing_mesh_iter != _meshes.end())
  {
    return existing_mesh_iter->second;
  }

  if (source_mesh->vertex_attributes.empty())
  {
    BOOST_THROW_EXCEPTION(core::runtime_error() << core::context_info(
                            "A mesh must have at least one vertex attribute."));
  }
  for (auto& attribute : source_mesh->vertex_attributes)
  {
    if (!attribute.vertex_buffer_view.buffer)
    {
      BOOST_THROW_EXCEPTION(core::runtime_error() << core::context_info(
                              "Each vertex attributes' buffer view must be "
                              "associated with a buffer."));
    }
  }

  // Translate resource mesh vertex description to Vulkan format.
  vk::layer2::mesh::vertex_attributes_t vertex_attributes;
  // std::vector<vk::vertex_input_attribute_description> vertex_attributes;
  for (auto& source_attribute : source_mesh->vertex_attributes)
  {
    auto& attribute = vertex_attributes[static_cast<std::underlying_type_t<
      vk::layer2::mesh::vertex_attribute_usage>>(source_attribute.usage)];
    if (attribute.buffer)
    {
      BOOST_THROW_EXCEPTION(
        shift::core::runtime_error() << shift::core::context_info(
          "Encountered two vertex attributes with the same usage type."));
    }
    auto vertex_buffer =
      source_attribute.vertex_buffer_view.buffer.get_shared();
    attribute.buffer = create_buffer(vk::buffer_usage_flag::vertex_buffer_bit,
                                     vertex_buffer, render_graph_lock);
    attribute.buffer_offset = source_attribute.vertex_buffer_view.offset;
    attribute.buffer_size = source_attribute.vertex_buffer_view.size;
    attribute.attribute_offset = source_attribute.offset;
    attribute.attribute_stride = source_attribute.stride;
    attribute.format = vk::format_from_resource(
      source_attribute.data_type, source_attribute.component_type);
    attribute.attribute_size = source_attribute.size;

    if (attribute.format == vk::format::undefined)
    {
      BOOST_THROW_EXCEPTION(
        shift::core::runtime_error()
        << shift::core::context_info("Unsupported vertex attribute format."));
    }

    switch (source_attribute.usage)
    {
    case resource::vertex_attribute_usage::position:
      if (attribute.format != vk::format::r32_g32_b32_sfloat)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex position format."));
      }
      if (source_attribute.size != sizeof(math::vector3<float>))
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex position size."));
      }
      if (source_attribute.offset != 0)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex position offset."));
      }
      break;

    case resource::vertex_attribute_usage::normal:
      if (attribute.format != vk::format::r32_g32_b32_sfloat)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex normal format."));
      }
      if (source_attribute.size != sizeof(math::vector3<float>))
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex normal size."));
      }
      if (source_attribute.offset != 0)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex normal offset."));
      }
      break;

    case resource::vertex_attribute_usage::texcoord:
      if (attribute.format != vk::format::r32_g32_sfloat)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex texcoord format."));
      }
      if (source_attribute.size != sizeof(math::vector2<float>))
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex texcoord size."));
      }
      if (source_attribute.offset != 0)
      {
        BOOST_THROW_EXCEPTION(
          shift::core::runtime_error()
          << shift::core::context_info("Unsupported vertex texcoord offset."));
      }
      break;

    default:
      // Ignore other cases for now.
      break;
    }
  }

  // Create vertex, index, and uniform buffers.
  vk::layer2::mesh::index_attribute_t index_attribute;
  auto source_index_buffer = source_mesh->index_buffer_view.buffer.get_shared();
  index_attribute.buffer =
    create_buffer(vk::buffer_usage_flag::index_buffer_bit, source_index_buffer,
                  render_graph_lock);
  index_attribute.buffer_offset = 0;
  index_attribute.buffer_size = 0;
  switch (source_mesh->index_data_type)
  {
  case resource::vertex_index_data_type::uint16:
    index_attribute.index_type = vk::index_type::uint16;
    break;

  case resource::vertex_index_data_type::uint32:
    index_attribute.index_type = vk::index_type::uint32;
    break;

  case resource::vertex_index_data_type::uint8:
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Unsupported vertex index type."));
  }

  boost::intrusive_ptr<vk::layer2::mesh> new_mesh(
    new vk::layer2::mesh(*_device, vertex_attributes, index_attribute), false);
  _meshes.insert_or_assign(source_mesh.get(), new_mesh);
  return new_mesh;
}

boost::intrusive_ptr<vk::layer2::texture> renderer_impl::create_texture(
  std::shared_ptr<resource::image>& source_texture,
  const std::lock_guard<std::mutex>& /*render_graph_lock*/)
{
  if (auto existing_texture_iter = _textures.find(source_texture.get());
      existing_texture_iter != _textures.end())
  {
    return existing_texture_iter->second;
  }

  vk::format image_format = vk::format_from_resource(source_texture->format);
  // Some textures have r8_g8_b8 format, which is not supported by Nvidia
  // hardware.
  /// ToDo: Solve this by checking actual device capabilities.
  /// Also cache result in a lookups table for following textures.
  if (image_format == vk::format::r8_g8_b8_unorm)
    image_format = vk::format::r8_g8_b8_a8_unorm;
  else if (image_format == vk::format::r8_g8_b8_srgb)
    image_format = vk::format::r8_g8_b8_a8_srgb;

  const auto& primary_mip_level = source_texture->mipmaps.front();
  vk::extent_3d extent(primary_mip_level.width, primary_mip_level.height,
                       primary_mip_level.depth);

  boost::intrusive_ptr<vk::layer2::texture> new_texture(
    new vk::layer2::texture(
      *_device, image_format, extent,
      static_cast<std::uint32_t>(source_texture->mipmaps.size()),
      source_texture->array_element_count, source_texture),
    false);
  _textures.insert_or_assign(source_texture.get(), new_texture);
  async_load(*new_texture);
  return new_texture;
}

boost::intrusive_ptr<vk::layer2::sampler> renderer_impl::create_sampler(
  const resource::sampler& source_sampler,
  const std::lock_guard<std::mutex>& /*render_graph_lock*/)
{
  auto sampler_id = std::hash<resource::sampler>{}(source_sampler);
  if (auto existing_sampler_iter = _samplers.find(sampler_id);
      existing_sampler_iter != _samplers.end())
  {
    return existing_sampler_iter->second;
  }

  boost::intrusive_ptr<vk::layer2::sampler> new_sampler(
    new vk::layer2::sampler(
      *_device,
      static_cast<vk::sampler_address_mode>(source_sampler.address_mode_u),
      static_cast<vk::sampler_address_mode>(source_sampler.address_mode_v),
      static_cast<vk::sampler_address_mode>(source_sampler.address_mode_w),
      source_sampler.max_anisotropy, source_sampler.min_lod,
      source_sampler.max_lod),
    false);
  _samplers.insert_or_assign(sampler_id, new_sampler);
  return new_sampler;
}

boost::intrusive_ptr<vk::layer2::material> renderer_impl::create_material(
  const std::shared_ptr<resource::material>& source_material,
  const std::lock_guard<std::mutex>& render_graph_lock)
{
  if (auto existing_material_iter = _materials.find(source_material.get());
      existing_material_iter != _materials.end())
  {
    return existing_material_iter->second;
  }

  auto translate_map =
    [&](std::pair<resource::image_reference, resource::sampler>&
          image_and_sampler) -> vk::layer2::material_map {
    auto& [image, sampler] = image_and_sampler;
    auto image_source = image.image.get_shared();

    return {create_texture(image_source, render_graph_lock),
            create_sampler(sampler, render_graph_lock)};
  };

  boost::intrusive_ptr<vk::layer2::material> new_material(
    new vk::layer2::material(
      *_device, {translate_map(source_material->albedo_map),
                 translate_map(source_material->ambient_occlusion_map),
                 translate_map(source_material->normal_map),
                 translate_map(source_material->height_map),
                 translate_map(source_material->roughness_map),
                 translate_map(source_material->metalness_map)}),
    false);
  _materials.insert_or_assign(source_material.get(), new_material);
  return new_material;
}

boost::intrusive_ptr<vk::layer2::model> renderer_impl::create_model(
  const std::shared_ptr<resource::mesh>& source_mesh,
  const std::shared_ptr<resource::material>& source_material,
  const std::lock_guard<std::mutex>& render_graph_lock)
{
  if (!source_mesh || !source_material)
    return nullptr;

  if (auto existing_model_iter =
        _models.find(std::make_pair(source_mesh.get(), source_material.get()));
      existing_model_iter != _models.end())
  {
    return existing_model_iter->second;
  }

  auto mesh = create_mesh(source_mesh, render_graph_lock);
  auto material = create_material(source_material, render_graph_lock);
  BOOST_ASSERT(mesh != nullptr && material != nullptr);
  boost::intrusive_ptr<vk::layer2::model> new_model(
    new vk::layer2::model(*_device, std::move(mesh), std::move(material)),
    false);
  _models.insert_or_assign(
    std::make_pair(source_mesh.get(), source_material.get()), new_model);
  return new_model;
}

void renderer_impl::create_swapchain()
{
  _swapchain_index = _window->create_swapchain(*_device);
  _swapchain_length = _window->image_count();
  for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
       ++swapchain_index)
  {
    _swapchain_images[swapchain_index] = _window->image(swapchain_index);

    // Create swapchain image and image view.
    _default_context.command_buffer().pipeline_barrier(
      vk::pipeline_stage_flag::top_of_pipe_bit, _window->present_stage_flags(),
      vk::dependency_flag::none,
      vk::image_memory_barrier(
        /* next */ nullptr,
        /* src_access_mask */ vk::access_flag::none,
        /* dst_access_mask */ _window->present_access_flags(),
        /* old_layout */ vk::image_layout::undefined,
        /* new_layout */ _window->present_image_layout(),
        /* src_queue_family_index */
        _device->graphics_queue_family_index(),
        /* dst_queue_family_index */
        _device->graphics_queue_family_index(),
        /* image */ _swapchain_images[swapchain_index]->handle(),
        /* subresource_range */
        vk::image_subresource_range(
          /* aspect_mask */ vk::image_aspect_flag::color_bit,
          /* base_mip_level */ 0u,
          /* level_count */ 1u,
          /* base_array_layer */ 0u,
          /* layer_count */ 1u)));

    _swapchain_image_views[swapchain_index] =
      vk::make_framed_shared<vk::layer1::image_view>(
        *_device,
        vk::image_view_create_info(
          /* next */ nullptr,
          /* flags */ 0,
          /* image */ _swapchain_images[swapchain_index]->handle(),
          /* view_type */ vk::image_view_type::_2d,
          /* format */ _swapchain_images[swapchain_index]->format(),
          /* components */
          vk::component_mapping{
            vk::component_swizzle::identity, vk::component_swizzle::identity,
            vk::component_swizzle::identity, vk::component_swizzle::identity},
          /* subresource_range */
          vk::image_subresource_range(
            /* aspect_mask */ vk::image_aspect_flag::color_bit,
            /* base_mip_level */ 0u,
            /* level_count */ 1u,
            /* base_array_layer */ 0u,
            /* layer_count */ 1u)),
        *_swapchain_images[swapchain_index]);
  }

  _allow_view_changes = false;
  for (auto& multiview : _multiviews)
  {
    multiview->create_frame_data(math::static_cast_vector<std::uint32_t>(
                                   _window->size() * multiview->relative_size),
                                 _default_context);
  }
}

void renderer_impl::destroy_swapchain()
{
  for (auto& multiview : _multiviews)
    multiview->destroy_frame_data();
  _allow_view_changes = true;

  for (std::uint32_t swapchain_index = 0; swapchain_index < _swapchain_length;
       ++swapchain_index)
  {
    _swapchain_image_views[swapchain_index].reset();
    _swapchain_images[swapchain_index].reset();
  }
  _window->destroy_swapchain();
}

void renderer_impl::create_descriptor_pool()
{
  if (_enable_text)
    _pass_text->create_resource_descriptions();
  // if (_enable_warping)
  //  _pass_warp->create_resource_descriptions();

  // Create descriptor pool
  const auto descriptor_pool_sizes =
    core::make_array(vk::descriptor_pool_size(
                       /* type */ vk::descriptor_type::uniform_buffer,
                       /* descriptor_count */ 8192),
                     vk::descriptor_pool_size(
                       /* type */ vk::descriptor_type::sampler,
                       /* descriptor_count */ 8192),
                     vk::descriptor_pool_size(
                       /* type */ vk::descriptor_type::sampled_image,
                       /* descriptor_count */ 8192),
                     vk::descriptor_pool_size(
                       /* type */ vk::descriptor_type::combined_image_sampler,
                       /* descriptor_count */ 8192),
                     vk::descriptor_pool_size(
                       /* type */ vk::descriptor_type::storage_image,
                       /* descriptor_count */ 32));

  _descriptor_pool = vk::make_framed_shared<vk::layer1::descriptor_pool>(
    *_device,
    vk::descriptor_pool_create_info(
      /* next */ nullptr,
      /* flags */ vk::descriptor_pool_create_flag::free_descriptor_set_bit,
      /* max_sets */ 8192,
      /* pool_size_count */ descriptor_pool_sizes.size(),
      /* pool_sizes */ descriptor_pool_sizes.data()));
}

void renderer_impl::destroy_descriptor_pool()
{
  _descriptor_pool.reset();

  // if (_enable_warping)
  //  _pass_warp->destroy_resource_descriptions();
  if (_enable_text)
    _pass_text->destroy_resource_descriptions();
}
}
