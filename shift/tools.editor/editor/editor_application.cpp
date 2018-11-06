#define BOOST_BIND_NO_PLACEHOLDERS
#include "editor/editor_application.hpp"
#include <shift/render/vk/renderer.hpp>
#include <shift/resource/repository.hpp>
#include <shift/resource/resource_group.hpp>
#include <shift/resource/scene.hpp>
#include <shift/log/log.hpp>
#include <shift/core/exception.hpp>
#include <shift/core/at_exit_scope.hpp>
#include <boost/filesystem/operations.hpp>
#include <QQmlComponent>
#include <QQuickStyle>
#include <QQmlContext>
#include <QQmlEngine>
#include <string>
#include <cstdlib>
#include <GLFW/glfw3.h>

namespace shift
{
using namespace std::chrono_literals;
using namespace std::placeholders;
using namespace render;

editor_application::editor_application(int& argc, char** argv)
: QGuiApplication(argc, argv)
{
}

editor_application::~editor_application() = default;

QPoint editor_application::window_position() const
{
  return _left_pane_position;
}

void editor_application::window_position(QPoint new_position)
{
  if (_left_pane_position != new_position)
  {
    _left_pane_position = new_position;
    emit window_position_changed();
  }
}

QSize editor_application::window_size() const
{
  return _window_size;
}

void editor_application::window_size(QSize new_size)
{
  if (_window_size != new_size)
  {
    _window_size = new_size;
    emit window_size_changed();
  }
}

int editor_application::run()
{
  // qmlRegisterType<world_view>("Editor", 1, 0, "WorldView");

  // Set UI style.
  QQuickStyle::setStyle("Universal");
  QQuickWindow::setDefaultAlphaBuffer(true);

  QObject::connect(this, SIGNAL(lastWindowClosed()), this, SLOT(quit()));
  QObject::connect(this, SIGNAL(aboutToQuit()), this, SLOT(prepare_quit()));

  _qml_engine.addImportPath(QString::fromStdWString(
    (boost::filesystem::current_path() / L"qml").generic_wstring()));
  refresh_qml();

  // tbb::task_scheduler_init tbb_task_scheduler_init;

  _idle_timer = std::make_unique<QTimer>();
  connect(_idle_timer.get(), SIGNAL(timeout()), this, SLOT(idle()));
  _idle_timer->start(16ms);

  initialize();
  auto result = exec();
  finalize();
  return result;
}

void editor_application::refresh_qml()
{
  static constexpr auto* qml_source = "qml/Editor.qml";

  auto* root_context = _qml_engine.rootContext();
  root_context->setContextProperty("editor", this);
  root_context->setContextProperty("schematicsController",
                                   &_schematics_controller);
  _qml_engine.clearComponentCache();
  _qml_engine.load(QUrl::fromLocalFile(QString::fromStdWString(
    boost::filesystem::absolute(qml_source).wstring())));

  auto root_objects = _qml_engine.rootObjects();
  if (root_objects.empty())
    BOOST_THROW_EXCEPTION(core::runtime_error());
  _window_editor = dynamic_cast<QQuickWindow*>(root_objects.first());
  if (_window_editor == nullptr)
    BOOST_THROW_EXCEPTION(core::runtime_error());
}

bool editor_application::select_center_view(const QString& /*name*/)
{
  return true;
}

bool editor_application::select_edit_tool(const QString& name)
{
  log::debug() << name.toStdString();
  if (name == "select")
  {
  }
  else if (name == "move")
  {
  }
  else if (name == "rotate")
  {
  }
  else if (name == "scale")
  {
  }
  else
    return false;
  return true;
}

void editor_application::idle()
{
  update();
}

void editor_application::prepare_quit()
{
  if (_idle_timer)
  {
    _idle_timer->stop();
    _idle_timer.reset();
  }
  finalize();
}

void editor_application::initialize()
{
  using namespace std::placeholders;
  using namespace std::chrono_literals;
  using namespace math::literals;
  using namespace render;

  // Initialize resource repository and mount public folder.
  _repository = std::make_unique<resource::repository>();
  _repository->mount("./");
  auto global_cache =
    _repository->load<resource::resource_group>("global.cache");

// Initialize renderer instance.
#if defined(BUILD_CONFIG_DEBUG) || 1
  vk::debug_layers debug_layers =
    vk::debug_layer::standard_validation | vk::debug_layer::monitor;
#else
  vk::debug_layers debug_layers = vk::debug_layer::monitor;
#endif
#if defined(BUILD_CONFIG_DEBUG)
  vk::debug_logs debug_logs = vk::debug_log::information |
                              vk::debug_log::warning |
                              vk::debug_log::performance_warning |
                              vk::debug_log::error | vk::debug_log::debug;
#else
  vk::debug_logs debug_logs = vk::debug_log::warning |
                              vk::debug_log::performance_warning |
                              vk::debug_log::error;
#endif
  _renderer = &vk::renderer::create_renderer(1, "shift.tools.editor", 1,
                                             debug_layers, debug_logs);

  // Select physical devices to use.
  vk::layer1::physical_device* selected_physical_device = nullptr;
  if (auto& physical_devices = _renderer->available_physical_devices();
      !physical_devices.empty())
  {
    selected_physical_device = physical_devices.front();
  }
  if (selected_physical_device == nullptr)
  {
    /// ToDo: Report error.
    return;
  }

  _render_window = _renderer->create_glfw_window(
    {400, 100}, {1280u, 1024u} /*, _view*/, vk::window_flag::decorated);

  // Add window callback handlers.
  _render_window->on_close.connect(
    std::bind(&editor_application::on_close, this));
  _render_window->on_move.connect(
    std::bind(&editor_application::on_move_window, this, _1));
  _render_window->on_key.connect(
    std::bind(&editor_application::on_key, this, _1, _2, _3, _4));
  _render_window->on_mouse_button.connect(
    std::bind(&editor_application::on_mouse_button, this, _1, _2, _3));
  _render_window->on_move_cursor.connect(
    std::bind(&editor_application::on_move_cursor, this, _1));
  _render_window->visible(true);

  _renderer->initialize(*selected_physical_device);

  create_views();

  _resource_scene =
    //  _repository->load<resource::scene>("scenes/empty.scene.json").get_shared();
    // _repository->load<resource::scene>("pbrt/sanmiguel/sanmiguel.scene").get_shared();
    _repository->load<resource::scene>("pbrt/white-room/whiteroom-night.scene")
      .get_shared();
  // _repository->load<resource::scene>("pbrt/landscape/view-0.scene").get_shared();
  if (!_resource_scene || _resource_scene->root == nullptr)
  {
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Missing scene resource"));
  }

  _renderer->create_resources();

  _view_position = math::make_vector_from(0.0f, -10.0f, 1.8f);
  _view_angles = math::make_vector_from(90.0_fdeg, 0.0_fdeg, 0.0_fdeg);
}

void editor_application::finalize()
{
  if (_renderer != nullptr)
  {
    _renderer->destroy_resources();
    destroy_views();
    _renderer->finalize();

    if (_render_window != nullptr)
    {
      _renderer->destroy_window(_render_window);
      _render_window = nullptr;
    }

    _renderer->release();
    _renderer = nullptr;
  }
  _repository.reset();
}

void editor_application::update()
{
  using namespace std::chrono;
  using namespace std::chrono_literals;

  /// ToDo: Get rid of statics.
  static auto start = high_resolution_clock::now();
  static auto now = start;
  static auto last_frame_time = now;
  static auto last_fps_update = start;
  static std::uint32_t fps_counter = 0;

  last_frame_time = now;
  now = high_resolution_clock::now();

  auto frame_duration = duration_cast<milliseconds>(now - last_frame_time);

  auto view_rotation = math::make_rotation_matrix_z<4, 4>(_view_angles.z) *
                       math::make_rotation_matrix_x<4, 4>(_view_angles.x);

  auto view_velocity = math::make_vector_from(0.0f, 0.0f, 0.0f);
  auto speed = 0.005f * frame_duration.count();
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

  _renderer->begin_frame();
  _renderer->end_frame();
  glfwPollEvents();

  ++fps_counter;
  if (now - last_fps_update >= 1s /*&& _last_buffer != ~0*/)
  {
    /// ToDo: Remove log dependency and add call to on_update_fps(new_fps).
    log::status()
      << (fps_counter * 1000.0f /
          duration_cast<milliseconds>(now - last_fps_update).count())
      << " fps";

    fps_counter = 0;
    last_fps_update = now;
  }
}

void editor_application::on_close()
{
  quit();
}

void editor_application::on_move_window(math::vector<2, std::int32_t> position)
{
  window_position(QPoint(position.x, position.y));
}

void editor_application::create_views()
{
  using namespace math::literals;

  auto window_size = _render_window->size();
  _render_view = _renderer->create_view(vk::view_create_param{
    /*.relative_position = */ math::make_vector_from(0.0f, 0.0f),
    /*.relative_size = */ math::make_vector_from(1.0f, 1.0f),
    /*.camera = */ nullptr});

  static const auto flip_y_axis = math::make_scale_matrix<4, 4>(
    math::make_vector_from(1.0f, -1.0f, 1.0f, 1.0f));

  _renderer->projection_matrix(
    math::make_perspective_projection_matrix<float>(
      60_fdeg, static_cast<float>(window_size.x) / window_size.y, 0.1f,
      1000.0f) *
    flip_y_axis);
}

void editor_application::destroy_views()
{
  if (_render_view != nullptr)
  {
    _renderer->destroy_view(_render_view);
    _render_view = nullptr;
  }
}

void editor_application::on_key(int key, int /*scancode*/, int action,
                                int /*mods*/)
{
  if (key == GLFW_KEY_A)
    _key_left = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_D)
    _key_right = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_F)
    _key_down = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_R)
    _key_up = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_S)
    _key_backward = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_W)
    _key_forward = (action != GLFW_RELEASE);
  else if (key == GLFW_KEY_F1)
  {
    if (action == GLFW_PRESS)
      _renderer->show_bounding_boxes(!_renderer->show_bounding_boxes());
  }
}

void editor_application::on_mouse_button(int button, int action, int /*mods*/)
{
  if (button == GLFW_MOUSE_BUTTON_1)
    _mouse_left = (action != GLFW_RELEASE);
  if (button == GLFW_MOUSE_BUTTON_2)
    _mouse_right = (action != GLFW_RELEASE);
}

void editor_application::on_move_cursor(math::vector<2, float> position)
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
}
