#ifndef SHIFT_EDITOR_APPLICATION_HPP
#define SHIFT_EDITOR_APPLICATION_HPP

#include <memory>
#include <vector>
#include <QTimer>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QWindow>
#include <QQuickWindow>
#include <boost/intrusive_ptr.hpp>
#include <shift/core/singleton.hpp>
#include <shift/core/object_pool.hpp>
#include <shift/math/vector.hpp>
#include <shift/math/matrix.hpp>
#include <shift/resource_db/types.hpp>
#include <shift/resource_db/repository.hpp>
#include <shift/resource_db/material.hpp>
#include "editor/schematics_controller.hpp"

namespace shift::resource_db
{
class repository;
class scene;
}

namespace shift::render::vk
{
class renderer;
class window;
}

namespace shift::render::vk::layer2
{
class view;
}

namespace shift
{
///
class editor_application
: public QGuiApplication,
  public core::singleton<editor_application, core::create::on_stack>
{
  Q_OBJECT

  Q_PROPERTY(QPoint windowPosition MEMBER _window_position READ window_position
               WRITE window_position NOTIFY window_position_changed)
  Q_PROPERTY(QSize windowSize MEMBER _window_size READ window_size WRITE
               window_size NOTIFY window_size_changed)

public:
  /// Constructor.
  editor_application(int& argc, char** argv);

  /// Destructor.
  virtual ~editor_application() override;

  /// Deleted constructor to enforce lvalue reference for argc.
  /// @remarks
  ///   The constructor of QGuiApplication requires a reference to a permanently
  ///   valid int for the argc parameter. This deleted overload prevents
  ///   accidental passing of a temporary value. storing argc in some member
  ///   variable does not work because the base class constructor is called
  ///   before any member is being initialized.
  editor_application(int&& argc, char** argv) = delete;

  ///
  QPoint window_position() const;

  ///
  void window_position(QPoint new_position);

  ///
  QSize window_size() const;

  ///
  void window_size(QSize new_size);

  ///
  int run();

public:
  ///
  Q_INVOKABLE void refresh_qml();

  ///
  Q_INVOKABLE bool select_center_view(const QString& name);

  ///
  Q_INVOKABLE bool select_edit_tool(const QString& name);

signals:
  ///
  void window_position_changed();

  ///
  void window_size_changed();

public slots:
  ///
  void idle();

  ///
  void prepare_quit();

private:
  ///
  void initialize();

  ///
  void finalize();

  ///
  void update();

  ///
  void on_close();

  ///
  void on_move_window(math::vector<2, std::int32_t> position);

  ///
  void create_views();

  ///
  void destroy_views();

  ///
  void on_key(int key, int scancode, int action, int mods);

  ///
  void on_mouse_button(int button, int action, int mods);

  ///
  void on_move_cursor(math::vector<2, float> position);

  QQmlApplicationEngine _qml_engine;
  QPoint _left_pane_position = {0, 0};
  QSize _window_size = {1280, 1024};
  QQuickWindow* _window_editor = nullptr;
  std::unique_ptr<QTimer> _idle_timer;
  schematics_controller _schematics_controller;

  bool _quit = false;
  bool _initialized = false;

  std::unique_ptr<resource_db::repository> _repository;
  render::vk::renderer* _renderer = nullptr;
  std::shared_ptr<resource_db::scene> _resource_scene;

  render::vk::window* _render_window = nullptr;
  render::vk::layer2::view* _render_view = nullptr;

  math::vector3<float> _view_position;
  math::vector3<float> _view_angles;
  math::vector<2, float> _cursor_position;
  bool _key_left = false;
  bool _key_right = false;
  bool _key_down = false;
  bool _key_up = false;
  bool _key_backward = false;
  bool _key_forward = false;
  bool _mouse_left = false;
  bool _mouse_right = false;
};
}

#endif
