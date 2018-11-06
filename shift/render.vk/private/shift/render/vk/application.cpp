#define BOOST_BIND_NO_PLACEHOLDERS
#include "shift/render/vk/application.hpp"
#include "shift/render/vk/renderer_impl.hpp"
#include "shift/render/vk/renderer.hpp"
#include "shift/render/vk/context.hpp"
#include "shift/render/vk/texture.hpp"
#include "shift/render/vk/window.hpp"
#include "shift/render/vk/layer1/device.hpp"
#include "shift/render/vk/layer1/physical_device.hpp"
#include <shift/resource/repository.hpp>
#include <shift/resource/scene.hpp>
#include <shift/resource/resource_group.hpp>
#include <shift/log/log.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <GLFW/glfw3.h>

namespace shift::render::vk
{
application::application() noexcept
{
}

application::~application() = default;

void application::application_name(const char* new_name)
{
  if (_initialized)
  {
    BOOST_THROW_EXCEPTION(core::logic_error() << core::context_info(
                            "You may not change the application name once "
                            "initialize has been called."));
  }
  _application_name = new_name;
}

const char* application::application_name() const
{
  return _application_name;
}

void application::application_version(std::uint32_t new_version)
{
  if (_initialized)
  {
    BOOST_THROW_EXCEPTION(core::logic_error() << core::context_info(
                            "You may not change the application version once "
                            "initialize has been called."));
  }
  _application_version = new_version;
}

std::uint32_t application::application_version() const
{
  return _application_version;
}

void application::run()
{
  // tbb::task_scheduler_init tbb_task_scheduler_init;

  auto at_exit_scope = core::make_at_exit_scope([&]() { finalize(); });

  initialize();
  while (!_quit)
    update();
}

void application::initialize()
{
  using namespace std::placeholders;
  using namespace std::chrono_literals;
  using namespace math::literals;

  if (_initialized)
  {
    BOOST_THROW_EXCEPTION(
      core::logic_error() << core::context_info(
        "You cannot call application::initialize() twice."));
  }
  _initialized = true;

  // Initialize resource repository and mount public folder.
  _repository = std::make_unique<resource::repository>();
  _repository->mount("public/");
  auto global_cache =
    _repository->load<resource::resource_group>("public/global.cache");

  // Initialize renderer instance.
  vk::debug_layers debug_layers =
#if defined(BUILD_CONFIG_DEBUG) || 1
    vk::debug_layer::standard_validation;
#else
    vk::debug_layer::monitor;
#endif
  vk::debug_logs debug_logs =
#if defined(BUILD_CONFIG_DEBUG)
    vk::debug_log::information | vk::debug_log::warning |
    vk::debug_log::performance_warning | vk::debug_log::error |
    vk::debug_log::debug;
#else
    vk::debug_log::warning | vk::debug_log::performance_warning |
    vk::debug_log::error;
#endif
  _renderer = &vk::renderer_impl::singleton_create(
    4, _application_name, _application_version, debug_layers, debug_logs);

  // Select physical devices to use.
  vk::layer1::physical_device* selected_physical_device = nullptr;
  for (auto* physical_device : _renderer->available_physical_devices())
  {
    if (test_physical_device(physical_device))
    {
      selected_physical_device = physical_device;
      break;
    }
  }
  if (selected_physical_device == nullptr)
    return;

  // Create default window.
  _window = _renderer->create_glfw_window({100, 100}, {2560u, 1024u},
                                          vk::window_flag::transparent |
                                            vk::window_flag::decorated |
                                            vk::window_flag::resizeable);

  // Add window callback handlers.
  _window->on_close.connect(std::bind(&application::on_close, this));
  _window->on_resize.connect([&](math::vector<2, std::uint32_t> /*size*/) {
    using namespace math::literals;

    // If the initial window size is larger than the desktop we immediately get
    // a resize event. We won't have to recreate anything so we can silently
    // ignore this case.
    if (_renderer->device() == nullptr)
      return;

    _renderer->wait_idle();
    _renderer->begin_resize();
    destroy_views();
    create_views();
    _renderer->end_resize();
  });
  _window->on_key.connect(
    std::bind(&application::on_key, this, _1, _2, _3, _4));
  _window->on_mouse_button.connect(
    std::bind(&application::on_mouse_button, this, _1, _2, _3));
  _window->on_move_cursor.connect(
    std::bind(&application::on_move_cursor, this, _1));
  _window->visible(true);

  create_views();
  _renderer->initialize(*selected_physical_device);

  _resource_scene =
    _repository->load<resource::scene>("public/pbrt/sanmiguel/sanmiguel.scene")
      .get_shared();
  //    _repository
  //      ->load<resource::scene>("public/pbrt/white-room/whiteroom-night.scene")
  //      .get_shared();
  // _repository->load<resource::scene>("public/pbrt/landscape/view-0.scene").get_shared();
  //  if (!_resource_scene || !_resource_scene->root)
  //  {
  //    BOOST_THROW_EXCEPTION(
  //      shift::core::runtime_error()
  //      << shift::core::context_info("Missing scene resource"));
  //  }
  _renderer->create_resources();
  create_resources(*_resource_scene);

  _view_position = math::make_vector_from(0.0f, -10.0f, 1.8f);
  _view_angles = math::make_vector_from(90.0_fdeg, 0.0_fdeg, 0.0_fdeg);
}

void application::finalize()
{
  if (_renderer != nullptr)
  {
    destroy_resources();
    _renderer->destroy_resources();
    _renderer->finalize();
    destroy_views();

    if (_window != nullptr)
    {
      _renderer->destroy_window(_window);
      _window = nullptr;
    }

    _renderer->release();
    _renderer = nullptr;
  }
  _repository.reset();
  _initialized = false;
}

void application::update()
{
  using namespace std::chrono;
  using namespace std::chrono_literals;
  using namespace math::literals;

  /// ToDo: Get rid of statics.
  static auto start = high_resolution_clock::now();
  static auto now = start;
  static auto last_frame_time = now;
  static auto last_fps_update = start;
  static std::uint32_t fps_counter = 0;

  last_frame_time = now;
  now = high_resolution_clock::now();

  auto frame_duration = duration_cast<microseconds>(now - last_frame_time);

  auto view_rotation = math::make_rotation_matrix_z<4, 4>(_view_angles.z) *
                       math::make_rotation_matrix_x<4, 4>(_view_angles.x);

  auto view_velocity = math::make_vector_from(0.0f, 0.0f, 0.0f);
  auto speed = 0.000001f * frame_duration.count();
  if (_key_boost)
    speed *= 5.0f;
  if (_key_right)
    view_velocity.x += speed;
  if (_key_left)
    view_velocity.x -= speed;

  if (_key_up)
    view_velocity.y += speed;
  if (_key_down)
    view_velocity.y -= speed;

  if (_key_backward)
    view_velocity.z += speed;
  if (_key_forward)
    view_velocity.z -= speed;

  _view_position += math::sub_matrix(view_rotation, 3, 3) * view_velocity;
  _renderer->view_matrix(math::transpose(view_rotation) *
                         math::make_translation_matrix<4, 4>(-_view_position));

  static const auto flip_y_axis = math::make_scale_matrix<4, 4>(
    math::make_vector_from(1.0f, -1.0f, 1.0f, 1.0f));
  auto window_size = _window->size();
  _renderer->projection_matrix(
    math::make_perspective_projection_matrix<float>(
      60_fdeg, static_cast<float>(window_size.x) / window_size.y, 0.1f,
      1000.0f) *
    flip_y_axis);

  _renderer->begin_frame();
  _renderer->end_frame();
  glfwPollEvents();

  ++fps_counter;
  if (now - last_fps_update >= 1s /*&& _last_buffer != ~0*/)
  {
    /// ToDo: Remove log dependency and add call to on_update_fps(new_fps).
    std::stringstream title;
    title << (fps_counter * 1000.0f /
              duration_cast<milliseconds>(now - last_fps_update).count())
          << " fps";
    _window->title(title.str());

    fps_counter = 0;
    last_fps_update = now;
  }
}

bool application::test_physical_device(
  const vk::layer1::physical_device* /*physical_device*/)
{
  /// ToDo: Check minimum capabilities we depend on.
  return true;
}

void application::create_views()
{
  //  _renderer->create_view(vk::view_create_param{
  //    /*.relative_position = */ math::make_vector_from(0.0f, 0.0f),
  //    /*.relative_size = */ math::make_vector_from(1.0f, 1.0f),
  //    /*.camera = */ nullptr});
  std::array<vk::view_create_param, 2> view_create_params{
    {{/*.relative_position = */ math::make_vector_from(0.0f, 0.0f),
      /*.relative_size = */ math::make_vector_from(0.5f, 1.0f),
      /*.camera = */ nullptr},
     {/*.relative_position = */ math::make_vector_from(0.5f, 0.0f),
      /*.relative_size = */ math::make_vector_from(0.5f, 1.0f),
      /*.camera = */ nullptr}}};
  [[maybe_unused]] auto* view = _renderer->create_multiview(
    vk::multiview_create_param{/*.view_create_params = */ view_create_params,
                               /*.views = */ {}});
}

void application::destroy_views()
{
  _renderer->destroy_all_views();
}

void application::create_resources(resource::scene& scene)
{
  using namespace std::chrono_literals;

  collect_resources(*scene.root, math::make_identity_matrix<4, 4, float>());

  // for (auto& texture : _scene_textures)
  //  texture->texture = _renderer->create_texture(texture->resource_image);

  // for (auto& mesh : _scene_meshes)
  //{
  //  BOOST_ASSERT(!mesh->resource_mesh->vertex_attributes.empty());
  //  auto& first_buffer =
  //    mesh->resource_mesh->vertex_attributes.front().vertex_buffer_view.buffer;
  //  for (const auto& vertex_attribute :
  //  mesh->resource_mesh->vertex_attributes)
  //  {
  //    BOOST_ASSERT(vertex_attribute.vertex_buffer_view.buffer.id() ==
  //                 first_buffer.id());
  //  }
  //  mesh->vertex_buffer = _renderer->create_buffer(
  //    vk::buffer_usage_flag::vertex_buffer_bit, first_buffer.get());
  //}
}

void application::destroy_resources()
{
  _scene_models_untextured.clear();
  _scene_models_textured.clear();
  _scene_models.clear();

  _scene_model_pool.clear();
}

void application::on_close()
{
  _quit = true;
}

void application::on_key(int key, int /*scancode*/, int action, int /*mods*/)
{
  if (action == GLFW_REPEAT)
    return;

  switch (key)
  {
  case GLFW_KEY_A:
    _key_left = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_D:
    _key_right = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_LEFT_SHIFT:
    _key_down = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_SPACE:
    _key_up = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_S:
    _key_backward = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_W:
    _key_forward = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_LEFT_CONTROL:
    _key_boost = (action != GLFW_RELEASE);
    break;

  case GLFW_KEY_F1:
    if (action == GLFW_PRESS)
      _renderer->show_bounding_boxes(!_renderer->show_bounding_boxes());
    break;

  case GLFW_KEY_ESCAPE:
    _quit = true;
    break;
  }
}

void application::on_mouse_button(int button, int action, int /*mods*/)
{
  switch (button)
  {
  case GLFW_MOUSE_BUTTON_1:
    _mouse_left = (action != GLFW_RELEASE);
    break;

  case GLFW_MOUSE_BUTTON_2:
    _mouse_right = (action != GLFW_RELEASE);
    break;
  }
}

void application::on_move_cursor(math::vector<2, float> position)
{
  using namespace math::literals;

  if (_mouse_left)
  {
    auto delta = position - _cursor_position;
    _view_angles +=
      math::make_vector_from(delta.y * -0.1_fdeg, 0.0f, delta.x * -0.1_fdeg);
  }
  _cursor_position = position;
}

void application::collect_resources(resource::scene_node& node,
                                    const math::matrix44<float>& transform)
{
  auto world_transform = transform * node.transform;

  if (node.mesh && node.material)
  {
    if (auto model = _renderer->create_model(node.mesh.get_shared(),
                                             node.material.get_shared()))
    {
      model_data_t* model_data = nullptr;
      if (model->reference_count() > 1)
      {
        for (auto* existing_model_data : _scene_models)
        {
          if (existing_model_data->model == model)
          {
            model_data = existing_model_data;
            break;
          }
        }
      }
      if (model_data == nullptr)
      {
        model_data = _scene_model_pool.create();
        model_data->model = std::move(model);
        _scene_models.push_back(model_data);
      }
      /// ToDo: Add material parameters to mesh_instance_data_t
      model_data->instances.emplace_back(mesh_instance_data_t{world_transform});
    }
  }

  for (auto* child : node.children)
    collect_resources(*child, world_transform);
}
}
