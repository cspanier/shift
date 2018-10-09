#ifndef SHIFT_RENDER_VK_APPLICATION_H
#define SHIFT_RENDER_VK_APPLICATION_H

#include <shift/core/boost_disable_warnings.h>
#include <boost/intrusive_ptr.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/singleton.h>
#include <shift/core/object_pool.h>
#include <shift/math/vector.h>
#include <shift/math/matrix.h>
#include <shift/resource/scene.h>
#include "shift/render/vk/types.h"
#include "shift/render/vk/window.h"

namespace shift::resource
{
class repository;
class scene;
}

namespace shift::render::vk
{
class renderer;
class window;

/// Base class for simple applications that use the Vulkan renderer.
class application : public core::singleton<application, core::create::on_stack>
{
public:
  ///
  application() noexcept;

  ///
  virtual ~application();

  application(const application&) = delete;
  application(application&&) = delete;
  application& operator=(const application&) = delete;
  application& operator=(application&&) = delete;

  ///
  void application_name(const char* new_name);

  ///
  const char* application_name() const;

  ///
  void application_version(std::uint32_t new_version);

  ///
  std::uint32_t application_version() const;

  /// This is purely a convenient function that doesn't do much more than
  /// calling initialize, looping update, and calling finalize.
  void run();

  /// Grants access to the renderer object.
  vk::renderer& renderer()
  {
    return *_renderer;
  }

protected:
  ///
  virtual void initialize();

  ///
  virtual void finalize();

  /// This method is called once per main loop.
  virtual void update();

  /// This method will be called once for each physical device available until
  /// one is accepted. If none of the passed physical devices are acceptable the
  /// application will quit.
  /// @remarks
  ///   An application should test for specific feature requirements and filter
  ///   unsupported devices here.
  /// @return
  ///   Return whether the passed physical device is acceptable.
  virtual bool test_physical_device(
    const vk::layer1::physical_device* physical_device);

  ///
  virtual void create_views();

  ///
  virtual void destroy_views();

  ///
  virtual void create_resources(resource::scene& scene);

  ///
  virtual void destroy_resources();

  ///
  virtual void on_close();

  ///
  virtual void on_key(int key, int scancode, int action, int mods);

  ///
  virtual void on_mouse_button(int button, int action, int mods);

  ///
  virtual void on_move_cursor(math::vector<2, float> position);

protected:
  ///
  void collect_resources(resource::scene_node& node,
                         const math::matrix44<float>& transform);

  const char* _application_name = "";
  std::uint32_t _application_version = 1;

  bool _quit = false;
  bool _initialized = false;
  std::unique_ptr<resource::repository> _repository;
  vk::renderer* _renderer = nullptr;
  vk::window* _window = nullptr;

  std::shared_ptr<resource::scene> _resource_scene;

  math::vector3<float> _view_position = {0.0f, 0.0f, 0.0f};
  math::vector3<float> _view_angles = {0.0f, 0.0f, 0.0f};
  math::vector<2, float> _cursor_position = {0.5f, 0.5f};
  bool _key_left = false;
  bool _key_right = false;
  bool _key_down = false;
  bool _key_up = false;
  bool _key_backward = false;
  bool _key_forward = false;
  bool _key_boost = false;
  bool _mouse_left = false;
  bool _mouse_right = false;

protected:
  struct mesh_instance_data_t
  {
    math::matrix44<float> model;
  };

  struct model_data_t
  {
    boost::intrusive_ptr<vk::model> model;
    std::vector<mesh_instance_data_t> instances;
  };

  core::object_pool<model_data_t> _scene_model_pool;

  std::vector<model_data_t*> _scene_models;
  std::vector<model_data_t*> _scene_models_textured;
  std::vector<model_data_t*> _scene_models_untextured;
};
}

#endif
