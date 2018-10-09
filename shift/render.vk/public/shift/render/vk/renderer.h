#ifndef SHIFT_RENDER_VK_RENDERER_H
#define SHIFT_RENDER_VK_RENDERER_H

#include <functional>
#include <gsl/span>
#include <shift/core/boost_disable_warnings.h>
#include <boost/intrusive_ptr.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/bit_field.h>
#include <shift/math/vector.h>
#include <shift/resource/image.h>
#include "shift/render/vk/types.h"
#include "shift/render/vk/shared.h"
#include "shift/render/vk/window.h"
#include "shift/render/vk/buffer.h"
#include "shift/render/vk/texture.h"
#include "shift/render/vk/material.h"
#include "shift/render/vk/mesh.h"
#include "shift/render/vk/model.h"

namespace shift::resource
{
class buffer;
class image;
class material;
class mesh;
class model;
class scene;
}

namespace shift::render::vk
{
///
enum class debug_layer : std::uint32_t
{
  none = 0u,
  standard_validation = 1u << 0,
  monitor = 1u << 1,
  screenshot = 1u << 2,
  api_dump = 1u << 3
};
using debug_layers = core::bit_field<debug_layer>;

///
inline debug_layers operator|(debug_layer lhs, debug_layer rhs)
{
  return debug_layers{lhs} | rhs;
}

///
enum class debug_log : std::uint32_t
{
  none = 0u,
  information = 1u << 0,
  warning = 1u << 1,
  performance_warning = 1u << 2,
  error = 1u << 3,
  debug = 1u << 4
};
using debug_logs = core::bit_field<debug_log>;

///
inline debug_logs operator|(debug_log lhs, debug_log rhs)
{
  return debug_logs{lhs} | rhs;
}

///
struct view_create_param
{
  /// Top left corner of the view in normalized window space coordinates (0-1).
  math::vector2<float> relative_position;
  /// Size of the view in normalized window space coordinates (0-1).
  math::vector2<float> relative_size;
  /// The initial camera assigned to this view.
  vk::camera* camera;
};

///
struct multiview_create_param
{
  /// A pointer to an array of view_create_param structures.
  gsl::span<const view_create_param> view_create_params;
  /// An optional pointer to an array of view handles which gets filled on
  /// success. If not null the number of elements in the view must match the
  /// number of elements of view_create_params.
  gsl::span<vk::view*> views;
};

///
class renderer
{
public:
  ///
  static vk::renderer& create_renderer(std::uint32_t render_thread_count,
                                       const char* application_name,
                                       std::uint32_t application_version,
                                       vk::debug_layers debug_layers,
                                       vk::debug_logs debug_logs);

public:
  renderer() = default;
  renderer(const renderer&) = delete;
  renderer(renderer&&) = delete;

  /// Destructor.
  virtual ~renderer() noexcept;

  renderer& operator=(const renderer&) = delete;
  renderer& operator=(renderer&&) = delete;

  /// Destroys the renderer instance.
  virtual void release() = 0;

  ///
  virtual bool show_bounding_boxes() const = 0;

  ///
  virtual void show_bounding_boxes(bool new_state) = 0;

  ///
  virtual const std::vector<vk::physical_device*>&
  available_physical_devices() = 0;

  ///
  virtual const vk::device* device() const = 0;

  ///
  virtual const math::matrix44<float>& projection_matrix() const = 0;

  ///
  virtual void projection_matrix(
    const math::matrix44<float>& new_projection) = 0;

  ///
  virtual const math::matrix44<float>& view_matrix() const = 0;

  ///
  virtual void view_matrix(const math::matrix44<float>& new_view) = 0;

public:
  /// Creates a window used for presentation.
  /// @pre
  ///   Must not be called between initialize and finalize.
  virtual vk::window* create_glfw_window(
    math::vector2<std::int32_t> initial_position,
    math::vector2<std::uint32_t> initial_size, vk::window_flags flags) = 0;

  /// Destroys a window.
  /// @pre
  ///   Must not be called between initialize and finalize.
  virtual void destroy_window(vk::window* window) = 0;

  /// @param graphics_physical_device
  ///   Select a physical device from instance().available_physical_devices().
  virtual void initialize(vk::physical_device& physical_device) = 0;

  ///
  virtual void finalize() = 0;

  /// Wait until all render work has finished.
  virtual void wait_idle() = 0;

  /// Creates a new world.
  /// @remarks
  ///   You may create multiple independent worlds.
  [[nodiscard]] virtual vk::world* create_world() = 0;

  /// Destroys a world.
  /// @post
  ///   The destroyed world handle is invalidated.
  virtual void destroy_world(vk::world* world) = 0;

  /// Destroys all currently existing worlds.
  /// @post
  ///   All handles previously returned by create_world are invalidated.
  virtual void destroy_all_worlds() = 0;

  /// Creates a new view.
  /// @pre
  ///   The swapchain must not be created.
  /// @return
  ///   A handle used to reference the view.
  /// @remarks
  ///   You can only call this function before initialize or between
  ///   begin_resize and end_resize.
  [[nodiscard]] virtual vk::view* create_view(
    const view_create_param& create_param) = 0;

  /// Destroys a specific view.
  /// @param view
  ///   The passed view must be created using create_view. Destroying individual
  ///   views from a multiview is not allowed.
  /// @pre
  ///   The swapchain must not be created.
  /// @post
  ///   The destroyed view handle is invalidated.
  /// @remarks
  ///   You can only call this function before initialize or between
  ///   begin_resize and end_resize.
  virtual void destroy_view(vk::view* view) = 0;

  /// Creates a group of views rendered in a single pass.
  /// @param create_param
  ///   The view_count field must be greater than one. All view_create_param
  ///   instances must have the same relative_size. The view field may be null.
  /// @return
  ///   A handle used to reference the multiview.
  /// @remarks
  ///   This is useful for VR, multi-monitor setups or rendering of cube maps.
  [[nodiscard]] virtual vk::multiview* create_multiview(
    const multiview_create_param& create_param) = 0;

  /// Destroys a set of multiview views.
  /// @param view_count
  ///   The number of view handles passed in view.
  /// @param multiview
  ///   A valid multiview handle previously created using create_multiview.
  /// @post
  ///   The destroyed multiview handle is invalidated.
  virtual void destroy_multiview(vk::multiview* multiview) = 0;

  /// Destroys all currently existing views and multiviews.
  /// @pre
  ///   The swapchain must not be created.
  /// @post
  ///   All handles previously returned by create_view and create_multiview are
  ///   invalidated.
  /// @remarks
  ///   You can only call this function before initialize or between
  ///   begin_resize and end_resize.
  virtual void destroy_all_views() = 0;

  /// Creates a context used for several stateful operations.
  /// @return
  ///   A valid context handle, or nullptr if some error occured.
  [[nodiscard]] virtual boost::intrusive_ptr<vk::context> create_context() = 0;

  ///
  [[nodiscard]] virtual boost::intrusive_ptr<vk::buffer> create_buffer(
    vk::buffer_usage_flags usage,
    std::shared_ptr<resource::buffer>& source_buffer) = 0;

  ///
  [[nodiscard]] virtual boost::intrusive_ptr<vk::mesh> create_mesh(
    const std::shared_ptr<resource::mesh>& source_mesh) = 0;

  ///
  [[nodiscard]] virtual boost::intrusive_ptr<vk::texture> create_texture(
    std::shared_ptr<resource::image>& source_texture) = 0;

  ///
  [[nodiscard]] virtual boost::intrusive_ptr<vk::material> create_material(
    const std::shared_ptr<resource::material>& source_material) = 0;

  ///
  [[nodiscard]] virtual boost::intrusive_ptr<vk::model> create_model(
    const std::shared_ptr<resource::mesh>& source_mesh,
    const std::shared_ptr<resource::material>& source_material) = 0;

  /// Call this method to recreate all window size dependent resources.
  virtual void begin_resize() = 0;

  /// Call this method to recreate all window size dependent resources.
  virtual void end_resize() = 0;

  ///
  virtual void begin_frame() = 0;

  //  ///
  //  virtual void begin_view(vk::view* view) = 0;

  //  ///
  //  virtual void end_view(vk::view* view) = 0;

  ///
  virtual void end_frame() = 0;

  ///
  virtual void create_resources() = 0;

  ///
  virtual void destroy_resources() = 0;

  ///
  virtual void async_load(vk::buffer& buffer) = 0;

  ///
  virtual void async_load(vk::texture& texture) = 0;
};
}

#endif
