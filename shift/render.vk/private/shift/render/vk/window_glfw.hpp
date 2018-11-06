#ifndef SHIFT_RENDER_VK_WINDOW_GLFW_HPP
#define SHIFT_RENDER_VK_WINDOW_GLFW_HPP

#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/window_impl.hpp"
#include "shift/render/vk/layer1/physical_device.hpp"
#include "shift/render/vk/layer1/fence.hpp"
#include "shift/render/vk/layer1/surface.hpp"
#include "shift/render/vk/layer1/swapchain.hpp"

using GLFWwindow = struct GLFWwindow;

namespace shift::render::vk
{
///
class window_glfw : public window_impl
{
public:
  /// Constructor.
  window_glfw(math::vector2<std::int32_t> initial_position,
              math::vector2<std::uint32_t> initial_size,
              vk::window_flags flags);

  /// Destructor.
  ~window_glfw() final;

  /// @see shift::render::vk::window_impl::create_surface.
  /// @pre
  ///   A physical device has been assigned using physical_device.
  void create_surface() final;

  /// @see shift::render::vk::window_impl::destroy_surface.
  void destroy_surface() final;

  /// @return
  ///   Returns the current swapchain buffer index.
  std::uint32_t create_swapchain(vk::layer1::device& device) final;

  ///
  void destroy_swapchain() final;

  /// @see shift::render::vk::window_impl::image_count.
  std::uint32_t image_count() const final;

  /// @see shift::render::vk::window_impl::image.
  vk::shared_ptr<vk::layer1::image> image(
    std::uint32_t image_index) const final;

  /// @see shift::render::vk::window_impl::acquire_next_image.
  std::uint32_t acquire_next_image() final;

  /// @see shift::render::vk::window_impl::present.
  bool present(uint32_t wait_semaphore_count, std::uint64_t* wait_semaphores,
               std::uint32_t current_buffer) final;

  /// Prevent hiding of getter method in base class.
  using vk::window::visible;

  /// @see shift::render::vk::window::visible.
  void visible(bool visible) final;

  /// @see shift::render::vk::window::position.
  math::vector2<std::int32_t> position() const final;

  /// @see shift::render::vk::window::position.
  void position(math::vector2<std::int32_t> position) final;

  /// @see shift::render::vk::window::size.
  math::vector2<std::uint32_t> size() const final;

  /// @see shift::render::vk::window::size.
  void size(math::vector2<std::uint32_t> size) final;

  /// @see shift::render::vk::window::title.
  std::string title() const final;

  /// @see shift::render::vk::window::title.
  void title(std::string title) final;

  /// Returns the native window handle.
  void* native_handle() const;

public:
  /// Return the last known size.
  /// @remarks
  ///   Some window systems tend to send unnecessary resize events, so the
  ///   last window size gets cached here.
  math::vector2<std::uint32_t> last_size() const;

  ///
  vk::layer1::surface& surface()
  {
    return *_surface;
  }

  ///
  const vk::layer1::surface& surface() const
  {
    return *_surface;
  }

  ///
  vk::layer1::swapchain& swapchain()
  {
    return *_swapchain;
  }

  ///
  const vk::layer1::swapchain& swapchain() const
  {
    return *_swapchain;
  }

protected:
  friend void close_callback(GLFWwindow* handle);

  friend void move_window_callback(GLFWwindow* handle, int x, int y);

  friend void resize_window_callback(GLFWwindow* handle, int width, int height);

  friend void key_callback(GLFWwindow* handle, int key, int scancode,
                           int action, int mods);

  friend void move_cursor_callback(GLFWwindow* handle, double x, double y);

  friend void scroll_callback(GLFWwindow* handle, double x, double y);

  friend void mouse_button_callback(GLFWwindow* handle, int button, int action,
                                    int mods);

  math::vector2<std::uint32_t> _last_size = {0u, 0u};
  core::stack_ptr<vk::layer1::surface> _surface;
  core::stack_ptr<vk::layer1::swapchain> _swapchain;
};
}

#endif
