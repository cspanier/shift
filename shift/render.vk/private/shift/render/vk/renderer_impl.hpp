#ifndef SHIFT_RENDER_VK_RENDERER_IMPL_HPP
#define SHIFT_RENDER_VK_RENDERER_IMPL_HPP

#include <vector>
#include <unordered_set>
#include <atomic>
#include <condition_variable>
#include <thread>
#include <gsl/gsl>
#include <shift/core/singleton.hpp>
#include <shift/core/stack_ptr.hpp>
#include <shift/resource/scene.hpp>
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/memory_manager.hpp"
#include "shift/render/vk/window.hpp"
#include "shift/render/vk/layer1/instance.hpp"
#include "shift/render/vk/layer1/physical_device.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/layer1/fence.hpp"
#include "shift/render/vk/layer1/semaphore.hpp"
#include "shift/render/vk/layer1/command_pool.hpp"
#include "shift/render/vk/layer1/command_buffer.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer1/image.hpp"
#include "shift/render/vk/layer1/image_view.hpp"
#include "shift/render/vk/layer1/framebuffer.hpp"
#include "shift/render/vk/layer1/sampler.hpp"
#include "shift/render/vk/layer1/shader_module.hpp"
#include "shift/render/vk/layer1/pipeline_cache.hpp"
#include "shift/render/vk/layer1/pipeline.hpp"
#include "shift/render/vk/layer1/pipeline_layout.hpp"
#include "shift/render/vk/layer1/render_pass.hpp"
#include "shift/render/vk/layer1/descriptor_pool.hpp"
#include "shift/render/vk/layer1/descriptor_set.hpp"
#include "shift/render/vk/layer1/descriptor_set_layout.hpp"
#include "shift/render/vk/layer1/query_pool.hpp"
#include "shift/render/vk/layer1/timestamp_query_pool.hpp"
#include "shift/render/vk/layer2/buffer.hpp"
#include "shift/render/vk/layer2/texture.hpp"
#include "shift/render/vk/layer2/sampler.hpp"
#include "shift/render/vk/layer2/material.hpp"
#include "shift/render/vk/resource_streamer.hpp"
#include "shift/render/vk/renderer.hpp"
#include "shift/render/vk/context_impl.hpp"
#include "shift/render/vk/application.hpp"
#include "shift/render/vk/utility.hpp"

namespace std
{
template <>
struct hash<std::pair<shift::resource::mesh*, shift::resource::material*>>
{
  std::size_t operator()(const std::pair<shift::resource::mesh*,
                                         shift::resource::material*>& key) const
  {
    using std::hash;

    return hash<shift::resource::mesh*>()(key.first) ^
           rotate(hash<shift::resource::material*>()(key.second));
  }

private:
  static std::size_t rotate(std::size_t value)
  {
    return (value << (sizeof(value) / 2)) | (value >> (sizeof(value) / 2));
  }
};
}

namespace shift::render::vk
{
class window_impl;
class context_impl;
class pass_terrain;
class pass_text;
class pass_warp;

///
struct buffer_load_job
{
  boost::intrusive_ptr<vk::layer2::buffer> buffer;
};

///
struct texture_load_job
{
  boost::intrusive_ptr<vk::layer2::texture> texture;
  std::uint32_t mip_level_begin;
  std::uint32_t mip_level_end;
};

///
struct buffer_transfer_job
{
  buffer_transfer_job(
    boost::intrusive_ptr<vk::layer2::buffer> buffer,
    vk::shared_ptr<vk::layer1::command_buffer>&& command_buffer,
    vk::layer1::fence&& submit_fence)
  : buffer(std::move(buffer)),
    command_buffer(std::move(command_buffer)),
    submit_fence(std::move(submit_fence))
  {
  }

  boost::intrusive_ptr<vk::layer2::buffer> buffer;
  vk::shared_ptr<vk::layer1::command_buffer> command_buffer;
  vk::layer1::fence submit_fence;
};

///
struct texture_transfer_job
{
  texture_transfer_job(
    boost::intrusive_ptr<vk::layer2::texture> texture,
    vk::shared_ptr<vk::layer1::command_buffer>&& command_buffer,
    vk::layer1::fence&& submit_fence, std::uint32_t mip_level_begin,
    std::uint32_t mip_level_end)
  : texture(std::move(texture)),
    command_buffer(std::move(command_buffer)),
    submit_fence(std::move(submit_fence)),
    mip_level_begin(mip_level_begin),
    mip_level_end(mip_level_end)
  {
  }

  boost::intrusive_ptr<vk::layer2::texture> texture;
  vk::shared_ptr<vk::layer1::command_buffer> command_buffer;
  vk::layer1::fence submit_fence;
  std::uint32_t mip_level_begin;
  std::uint32_t mip_level_end;
};

///
struct texture_transition_job
{
  vk::shared_ptr<vk::layer1::command_buffer> command_buffer;
  vk::layer1::fence submit_fence;
  std::vector<boost::intrusive_ptr<vk::layer2::texture>> textures;
};

///
struct global_uniform_data_t
{
  math::matrix44<float> last_inv_view_proj;
  math::matrix44<float> view_proj;
  math::vector2<float> source_size;
  math::vector2<float> target_size;
};

constexpr std::uint32_t max_multiview_view_count = 6;

/// ToDo: merge with global_uniform_data_t.
struct mesh_uniform_data
{
  math::matrix44<float> view[max_multiview_view_count];
  math::matrix44<float> projection[max_multiview_view_count];
};

struct mesh_instance_data
{
  math::matrix44<float> model;
};

struct image_data
{
  resource::image* resource_image = nullptr;

  vk::shared_ptr<vk::layer1::image> image;
  vk::shared_ptr<vk::layer1::image_view> image_view;
};

struct sampler_data
{
  resource::sampler resource_sampler;
  vk::shared_ptr<vk::layer1::sampler> sampler;
};

struct material_data
{
  resource::material* resource_material = nullptr;
  std::pair<image_data*, sampler_data*> albedo =
    std::make_pair(nullptr, nullptr);
  std::pair<image_data*, sampler_data*> normal =
    std::make_pair(nullptr, nullptr);
};

struct mesh_data
{
  resource::mesh* resource_mesh = nullptr;

  vk::shared_ptr<vk::layer1::buffer> position_buffer;
  vk::shared_ptr<vk::layer1::buffer> normal_buffer;
  vk::shared_ptr<vk::layer1::buffer> texcoord_buffer;
  vk::shared_ptr<vk::layer1::buffer> index_buffer;
  std::uint64_t position_offset = 0;
  std::uint64_t normal_offset = 0;
  std::uint64_t texcoord_offset = 0;
  bool is_textured = false;
};

struct model_data
{
  material_data* material = nullptr;
  mesh_data* mesh = nullptr;
  std::vector<mesh_instance_data> instances;
  vk::shared_ptr<vk::layer1::buffer> instance_buffer;
  multi_buffered<vk::shared_ptr<vk::layer1::descriptor_set>> descriptor_sets;
};

struct bounding_box_data
{
  std::vector<mesh_instance_data> instances;
  vk::shared_ptr<vk::layer1::buffer> vertex_buffer;
  vk::shared_ptr<vk::layer1::buffer> index_buffer;
  vk::shared_ptr<vk::layer1::buffer> instance_buffer;
  multi_buffered<vk::shared_ptr<vk::layer1::descriptor_set>> descriptor_sets;
};

///
class renderer_impl final
: public renderer,
  public core::singleton<renderer_impl, core::create::using_new>
{
public:
  /// Constructor.
  renderer_impl(std::uint32_t worker_count, const char* application_name,
                std::uint32_t application_version,
                vk::debug_layers debug_layers, vk::debug_logs debug_logs);

  /// Destructor.
  ~renderer_impl() noexcept final;

  renderer_impl(const renderer_impl&) = delete;
  renderer_impl(renderer_impl&&) = delete;
  renderer_impl& operator=(const renderer_impl&) = delete;
  renderer_impl& operator=(renderer_impl&&) = delete;

  /// @see shift::render::vk::renderer::release.
  void release() final;

  ///
  bool show_bounding_boxes() const final
  {
    return _show_bounding_boxes;
  }

  ///
  void show_bounding_boxes(bool new_state) final
  {
    _show_bounding_boxes = new_state;
  }

  const std::vector<vk::layer1::physical_device*>& available_physical_devices()
    final;

  ///
  vk::layer1::device* device() const final
  {
    return _device.get();
  }

  ///
  vk::resource_streamer& resource_streamer()
  {
    BOOST_ASSERT(_resource_streamer != nullptr);
    return *_resource_streamer;
  }

  ///
  const math::matrix44<float>& projection_matrix() const final
  {
    return _projection_matrix;
  }

  ///
  void projection_matrix(const math::matrix44<float>& new_projection) final
  {
    _projection_matrix = new_projection;
  }

  ///
  const math::matrix44<float>& view_matrix() const final
  {
    return _view_matrix;
  }

  ///
  void view_matrix(const math::matrix44<float>& new_view) final
  {
    _view_matrix = new_view;
  }

  /// @see shift::render::vk::renderer::create_window.
  vk::window* create_glfw_window(math::vector2<std::int32_t> initial_position,
                                 math::vector2<std::uint32_t> initial_size,
                                 vk::window_flags flags) final;

  /// @see shift::render::vk::renderer::destroy_window.
  void destroy_window(vk::window* window) final;

  /// @param physical_device
  ///   Select a physical device from instance().available_physical_devices().
  void initialize(vk::layer1::physical_device& physical_device) final;

  ///
  void finalize() final;

  ///
  void wait_idle() final;

  ///
  [[nodiscard]] vk::layer2::world* create_world() final;

  ///
  void destroy_world(vk::layer2::world* world) final;

  ///
  void destroy_all_worlds() final;

  ///
  [[nodiscard]] vk::layer2::view* create_view(
    const view_create_param& create_param) final;

  ///
  void destroy_view(vk::layer2::view* view) final;

  ///
  [[nodiscard]] vk::layer2::multiview* create_multiview(
    const multiview_create_param& create_param) final;

  ///
  void destroy_multiview(vk::layer2::multiview* multiview) final;

  ///
  void destroy_all_views() final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::context> create_context() final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::buffer> create_buffer(
    vk::buffer_usage_flags usage,
    std::shared_ptr<resource::buffer>& source_buffer) final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::mesh> create_mesh(
    const std::shared_ptr<resource::mesh>& source_mesh) final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::texture> create_texture(
    std::shared_ptr<resource::image>& source_texture) final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::material> create_material(
    const std::shared_ptr<resource::material>& source_material) final;

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::model> create_model(
    const std::shared_ptr<resource::mesh>& source_mesh,
    const std::shared_ptr<resource::material>& source_material) final;

  /// Destroys a context.
  /// @param context
  ///   A valid context previously created using create_context.
  /// @post
  ///   The context is invalidated.
  /// @remarks
  ///   This method gets automatically called by vk::context_impl::release.
  void destroy_context(gsl::owner<vk::context*> context);

  ///
  void begin_resize() final;

  ///
  void end_resize() final;

  ///
  void begin_frame() final;

  ///
  void end_frame() final;

  ///
  void create_resources() final;

  ///
  void destroy_resources() final;

  ///
  void async_load(vk::buffer& buffer) final;

  ///
  void async_load(vk::texture& texture) final;

public:
  /// Returns the number of worker threads.
  std::size_t worker_count() const
  {
    return _worker_count;
  }

  /// Returns the worker id of the calling thread.
  /// @pre
  ///   This call is only valid from one of the worker threads.
  std::uint32_t worker_id() const;

  ///
  vk::layer1::instance& instance()
  {
    return _instance;
  }

  ///
  const vk::allocation_callbacks* default_allocator() const
  {
    return _default_allocator;
  }

  ///
  std::uint32_t swapchain_length()
  {
    return _swapchain_length;
  }

  ///
  std::uint32_t swapchain_index()
  {
    return _swapchain_index;
  }

  ///
  vk::shared_ptr<vk::layer1::descriptor_pool> descriptor_pool()
  {
    return _descriptor_pool;
  }

  ///
  vk::layer1::descriptor_set_layout& scene_descriptor_set_layout()
  {
    return *_scene_descriptor_set_layout;
  }

  ///
  void create_swapchain();

  ///
  void destroy_swapchain();

  ///
  void create_descriptor_pool();

  ///
  void destroy_descriptor_pool();

  ///
  void create_pipeline();

  ///
  void destroy_pipeline();

private:
  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::buffer> create_buffer(
    vk::buffer_usage_flags usage,
    std::shared_ptr<resource::buffer>& source_buffer,
    const std::lock_guard<std::mutex>& /*render_graph_lock*/);

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::mesh> create_mesh(
    const std::shared_ptr<resource::mesh>& source_mesh,
    const std::lock_guard<std::mutex>& render_graph_lock);

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::texture> create_texture(
    std::shared_ptr<resource::image>& source_texture,
    const std::lock_guard<std::mutex>& /*render_graph_lock*/);

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::sampler> create_sampler(
    const resource::sampler& source_sampler,
    const std::lock_guard<std::mutex>& /*render_graph_lock*/);

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::material> create_material(
    const std::shared_ptr<resource::material>& source_material,
    const std::lock_guard<std::mutex>& render_graph_lock);

  ///
  [[nodiscard]] boost::intrusive_ptr<vk::layer2::model> create_model(
    const std::shared_ptr<resource::mesh>& source_mesh,
    const std::shared_ptr<resource::material>& source_material,
    const std::lock_guard<std::mutex>& render_graph_lock);

private:
  ///
  void process_worker(std::uint32_t worker_id);

  ///
  void update_transfer_jobs();

private:
  //  vk::debug_flags _debug_flags;

  std::size_t _worker_count = 0;
  std::atomic<bool> _quit = ATOMIC_VAR_INIT(false);
  std::vector<std::thread> _workers;
  static thread_local std::uint32_t _worker_id;

  std::mutex _worker_mutex;
  std::condition_variable _worker_condition;
  std::size_t _active_job_count = 0;
  std::queue<std::unique_ptr<buffer_load_job>> _buffer_load_jobs;
  std::deque<std::unique_ptr<texture_load_job>> _texture_load_jobs;

  std::mutex _transfer_mutex;
  std::queue<buffer_transfer_job> _buffer_transfers;
  std::queue<texture_transfer_job> _texture_transfers;
  std::queue<texture_transition_job> _texture_transitions;

  bool _enable_warping = false;
  bool _enable_text = false;
  bool _show_bounding_boxes = false;

  vk::layer1::instance _instance;
  const vk::allocation_callbacks* _default_allocator = nullptr;
  const vk::layer1::physical_device* _physical_device = nullptr;
  std::unique_ptr<vk::window_impl> _window;
  std::unique_ptr<vk::layer1::device> _device;
  std::unique_ptr<vk::resource_streamer> _resource_streamer;

  vk::context_impl _default_context;
  vk::layer1::semaphore _reproject_depth_complete_semaphore;
  vk::layer1::semaphore _frame_end_semaphore;
  /// The number of images in the active swapchain.
  std::uint32_t _swapchain_length = 0;
  /// The swapchain index is the value acquired by vkAcquireNextImageKHR. It may
  /// or may not be equal to _frame_index.
  std::uint32_t _swapchain_index = 0;
  /// A simple counter for the number of frames rendered.
  std::uint32_t _frame_counter = 0;
  /// The frame index is basically _frame_counter % _swapchain_length, with
  /// additional logic to avoid overflow errors.
  std::uint32_t _frame_index = 0;

  swap_data<vk::shared_ptr<vk::layer1::command_buffer>>
    _prepare_command_buffers;
  swap_data<vk::shared_ptr<vk::layer1::command_buffer>>
    _primary_command_buffers;
  swap_data<core::stack_ptr<vk::layer1::fence>> _submit_fences;
  swap_data<bool> _submitted = {false, false, false, false};

  swap_data<vk::shared_ptr<vk::layer1::image>> _swapchain_images;
  swap_data<vk::shared_ptr<vk::layer1::image_view>> _swapchain_image_views;

  core::stack_ptr<vk::layer1::pipeline_cache> _pipeline_cache;
  vk::shared_ptr<vk::layer1::descriptor_pool> _descriptor_pool;

  /// Global data used in reproject depth pass shared across all views.
  core::stack_ptr<vk::layer1::descriptor_set_layout>
    _reproject_depth_descriptor_set_layout;
  core::stack_ptr<vk::layer1::pipeline_layout> _reproject_depth_pipeline_layout;
  vk::shared_ptr<vk::layer1::pipeline> _reproject_depth_pipeline;
  vk::shared_ptr<vk::layer1::sampler> _reproject_depth_sampler;

  // core::stack_ptr<vk::layer1::descriptor_set_layout>
  // _convert_depth_descriptor_set_layout;
  // core::stack_ptr<vk::layer1::pipeline_layout>
  // _convert_depth_pipeline_layout; vk::shared_ptr<vk::layer1::pipeline>
  // _convert_depth_pipeline; vk::shared_ptr<vk::layer1::sampler>
  // _convert_depth_sampler;

  core::stack_ptr<vk::layer1::descriptor_set_layout>
    _scene_descriptor_set_layout;
  core::stack_ptr<vk::layer1::pipeline_layout> _scene_pipeline_layout;
  vk::shared_ptr<vk::layer1::render_pass> _scene_render_pass;
  vk::shared_ptr<vk::layer1::pipeline> _scene_pipeline_textured;
  vk::shared_ptr<vk::layer1::pipeline> _scene_pipeline_untextured;
  vk::shared_ptr<vk::layer1::pipeline> _scene_pipeline_bounding_box;

  core::object_pool<image_data> _scene_image_pool;
  core::object_pool<sampler_data> _scene_sampler_pool;
  core::object_pool<material_data> _scene_material_pool;
  core::object_pool<mesh_data> _scene_mesh_pool;
  core::object_pool<model_data> _scene_model_pool;

  std::vector<image_data*> _scene_images;
  std::vector<sampler_data*> _scene_samplers;
  std::vector<material_data*> _scene_materials;
  std::vector<mesh_data*> _scene_meshes;
  std::vector<model_data*> _scene_models;
  std::vector<model_data*> _scene_models_textured;
  std::vector<model_data*> _scene_models_untextured;
  bounding_box_data _scene_bounding_boxes;

  multi_buffered<vk::shared_ptr<vk::layer1::buffer>> _scene_uniform_buffers;

  std::unique_ptr<pass_text> _pass_text;
  std::unique_ptr<pass_warp> _pass_warp;

  std::mutex _render_graph_mutex;
  std::unordered_map<resource::buffer*,
                     boost::intrusive_ptr<vk::layer2::buffer>>
    _buffers;
  std::unordered_map<resource::mesh*, boost::intrusive_ptr<vk::layer2::mesh>>
    _meshes;
  std::unordered_map<resource::image*,
                     boost::intrusive_ptr<vk::layer2::texture>>
    _textures;
  std::unordered_map<std::size_t, boost::intrusive_ptr<vk::layer2::sampler>>
    _samplers;
  std::unordered_map<resource::material*,
                     boost::intrusive_ptr<vk::layer2::material>>
    _materials;
  std::unordered_map<std::pair<resource::mesh*, resource::material*>,
                     boost::intrusive_ptr<vk::layer2::model>>
    _models;

  std::vector<std::unique_ptr<vk::layer2::world>> _worlds;
  std::vector<std::unique_ptr<vk::layer2::multiview>> _multiviews;
  bool _allow_view_changes = true;

  std::mutex _context_lock;
  std::vector<context_impl*> _contexts;

  /// ToDo: Move these into some sort of scene graph.
  math::matrix44<float> _projection_matrix =
    math::make_identity_matrix<4, 4, float>();
  math::matrix44<float> _view_matrix =
    math::make_identity_matrix<4, 4, float>();
};
}

#endif
