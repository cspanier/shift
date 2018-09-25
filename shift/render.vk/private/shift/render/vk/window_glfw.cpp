#include "shift/render/vk/window_glfw.h"
#include "shift/render/vk/renderer_impl.h"
#include "shift/render/vk/shared.h"
#include "shift/render/vk/layer1/device.h"
#include "shift/render/vk/layer1/fence.h"
#include <shift/platform/fpexceptions.h>
#include <GLFW/glfw3.h>
#if defined(SHIFT_PLATFORM_WINDOWS)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <GLFW/glfw3native.h>
#include <dwmapi.h>
#elif defined(SHIFT_PLATFORM_LINUX)
#if defined(USE_WAYLAND)
#define GLFW_EXPOSE_NATIVE_WAYLAND
#else
#define GLFW_EXPOSE_NATIVE_X11
#endif
#include <GLFW/glfw3native.h>
#endif

#include <iostream>

namespace shift::render::vk
{
struct glfw_instance
{
  glfw_instance()
  {
    glfwSetErrorCallback(&error_callback);
    /// ToDo: glfwInit() throws an inexact floating-point exception, which
    /// we simply ignore here. Check a later version of the library if this
    /// still holds true.
    platform::floating_point_exceptions fpexceptions(0, _EM_INEXACT);
    if (!glfwInit())
    {
      BOOST_THROW_EXCEPTION(
        shift::core::runtime_error()
        << shift::core::context_info("Failed to initialize GLFW.")
        << shift::core::context_info(std::to_string(error_code))
        << shift::core::context_info(error_description));
    }
  }

  ~glfw_instance()
  {
    glfwSetErrorCallback(nullptr);
    glfwTerminate();
  }

  static void error_callback(int error, const char* description)
  {
    error_code = error;
    error_description = description;
  }

  static thread_local int error_code;
  static thread_local std::string error_description;
};

thread_local int glfw_instance::error_code = 0;
thread_local std::string glfw_instance::error_description;

void close_callback(GLFWwindow* handle)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_close();
}

void move_window_callback(GLFWwindow* handle, int x, int y)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_move({x, y});
}

void resize_window_callback(GLFWwindow* handle, int width, int height)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;

  auto size =
    math::make_vector_from(static_cast<unsigned int>(width >= 0 ? width : 0),
                           static_cast<unsigned int>(height >= 0 ? height : 0));
  if (size.x == 0 && size.y == 0)
  {
    /// ToDo: Happens in KDE, possibly all X11 desktops. Neither do I have a
    /// reason why, yet, nor do I know if it is safe to simply skip the
    /// event in this case.
    return;
  }
  if (size == vk_window->last_size())
    return;
  vk_window->_last_size = size;
  vk_window->on_resize(size);
}

/// ToDo: On Windows 10 the window size seems to be equal to the framebuffer
/// size, even when setting a custom window scaling factor. Check how other
/// operating systems handle this.
// void resize_framebuffer_callback(GLFWwindow* handle, int width,
//                                        int height)
//{
//  auto* vk_window =
//  static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
//  BOOST_ASSERT(vk_window);
//  if (!vk_window)
//    return;
//  if (width < 0)
//    width = 0;
//  if (height < 0)
//    height = 0;
//  auto size = math::vector2<std::uint32_t>{
//    static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
//  auto last_size = vk_window->last_size();
//  if (last_size.width == width && last_size.height == height)
//    return;
//  vk_window->last_size(size);
//  vk_window->on_resize(size);
//}

void key_callback(GLFWwindow* handle, int key, int scancode, int action,
                  int mods)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_key(key, scancode, action, mods);
}

void move_cursor_callback(GLFWwindow* handle, double x, double y)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_move_cursor({static_cast<float>(x), static_cast<float>(y)});
}

void scroll_callback(GLFWwindow* handle, double x, double y)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_scroll({static_cast<float>(x), static_cast<float>(y)});
}

void mouse_button_callback(GLFWwindow* handle, int button, int action, int mods)
{
  auto* vk_window = static_cast<window_glfw*>(glfwGetWindowUserPointer(handle));
  BOOST_ASSERT(vk_window);
  if (!vk_window)
    return;
  vk_window->on_mouse_button(button, action, mods);
}

window_glfw::window_glfw(math::vector2<std::int32_t> initial_position,
                         math::vector2<std::uint32_t> initial_size,
                         vk::window_flags flags)
: vk::window_impl(initial_position, initial_size, flags),
  _last_size(initial_size)
{
  static const glfw_instance _glfw_instance;

  // glfwGetWindowSize(_handle, &_size.width, &_size.height);

  glfwWindowHint(GLFW_VISIBLE, _visible ? 1 : 0);
  glfwWindowHint(GLFW_RESIZABLE, _resizeable ? 1 : 0);
  glfwWindowHint(GLFW_DECORATED, _decorated ? 1 : 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  _handle = glfwCreateWindow(static_cast<int>(_initial_size.x),
                             static_cast<int>(_initial_size.y), _title.c_str(),
                             nullptr, nullptr);
  if (!_handle)
  {
    SHIFT_THROW_EXCEPTION(
      shift::core::runtime_error()
      << shift::core::context_info("Failed to create GLFW window.")
      << shift::core::context_info("glfwCreateWindow"));
  }

#if defined(SHIFT_PLATFORM_WINDOWS)
  if (_transparent)
  {
    auto win32Handle = glfwGetWin32Window(static_cast<GLFWwindow*>(_handle));
    DWM_BLURBEHIND blurBehind;
    blurBehind.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
    blurBehind.fEnable = true;
    blurBehind.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
    DwmEnableBlurBehindWindow(win32Handle, &blurBehind);
    DeleteObject(blurBehind.hRgnBlur);

    HBRUSH brush = CreateSolidBrush(0x00000000);
    SetClassLongPtr(win32Handle, GCLP_HBRBACKGROUND,
                    reinterpret_cast<LONG_PTR>(brush));

    // SetWindowPos(win32Handle, HWND_TOPMOST, 0, 0, 0, 0,
    //             SWP_NOMOVE | SWP_NOSIZE);
  }
#endif

  glfwSetWindowPos(static_cast<GLFWwindow*>(_handle), _initial_position.x,
                   _initial_position.y);

  glfwSetWindowUserPointer(static_cast<GLFWwindow*>(_handle), this);

  glfwSetWindowCloseCallback(
    static_cast<GLFWwindow*>(_handle),
    reinterpret_cast<GLFWwindowclosefun>(close_callback));
  glfwSetWindowPosCallback(static_cast<GLFWwindow*>(_handle),
                           move_window_callback);
  glfwSetWindowSizeCallback(static_cast<GLFWwindow*>(_handle),
                            resize_window_callback);
  // glfwSetFramebufferSizeCallback(static_cast<GLFWwindow*>(_handle),
  //                               resize_framebuffer_callback);
  glfwSetKeyCallback(static_cast<GLFWwindow*>(_handle), key_callback);
  glfwSetCursorPosCallback(static_cast<GLFWwindow*>(_handle),
                           move_cursor_callback);
  glfwSetScrollCallback(static_cast<GLFWwindow*>(_handle), scroll_callback);
  glfwSetMouseButtonCallback(static_cast<GLFWwindow*>(_handle),
                             mouse_button_callback);
}

window_glfw::~window_glfw()
{
  if (_handle)
  {
    glfwDestroyWindow(static_cast<GLFWwindow*>(_handle));
    _handle = nullptr;
  }
}

void window_glfw::create_surface()
{
  BOOST_ASSERT(_physical_device);
  BOOST_ASSERT(!_surface);

  if (!_physical_device)
    return;

  auto& instance = renderer_impl::singleton_instance().instance();
  VkSurfaceKHR surface_handle = nullptr;
  vk_check(static_cast<vk::result>(glfwCreateWindowSurface(
    instance.handle(), static_cast<GLFWwindow*>(_handle),
    reinterpret_cast<const VkAllocationCallbacks*>(
      renderer_impl::singleton_instance().default_allocator()),
    &surface_handle)));
  BOOST_ASSERT(surface_handle);
  if (!surface_handle)
  {
    SHIFT_THROW_EXCEPTION(
      core::runtime_error() << core::context_info(
        "glfwCreateWindowSurface failed to create a surface."));
  }
  try
  {
    _surface.create(instance, *_physical_device, surface_handle);
  }
  catch (...)
  {
    vk::destroy_surface_khr(
      instance.handle(), surface_handle,
      renderer_impl::singleton_instance().default_allocator());
    throw;
  }
}

void window_glfw::destroy_surface()
{
  _surface.reset();
}

std::uint32_t window_glfw::create_swapchain(vk::layer1::device& device)
{
  _device = &device;

  _present_stage_flags = vk::pipeline_stage_flag::color_attachment_output_bit;
  _present_access_flags = vk::access_flag::memory_read_bit;
  _present_image_layout = vk::image_layout::present_src_khr;

  auto old_swapchain = std::move(_swapchain);
  auto window_size = size();
  _swapchain.create(device, *_surface,
                    vk::extent_2d{window_size.x, window_size.y}, 2,
                    old_swapchain.get());

  // Get the index of the next available swapchain image:
  auto [result, swapchain_index, present_complete_semaphore] =
    _swapchain->acquire_next_image(std::numeric_limits<std::uint64_t>::max(),
                                   vk::layer1::fence::null_handle);
  switch (result)
  {
  case vk::result::error_out_of_date_khr:
    BOOST_THROW_EXCEPTION(
      shift::core::runtime_error() << shift::core::context_info(
        "The swapchain was out of date already after creation."));

  case vk::result::suboptimal_khr:
    // demo->swapchain is not as optimal as it could be, but the platform's
    // presentation engine will still present the image correctly.
    break;

  case vk::result::success:
    break;

  default:
    BOOST_ASSERT(false);
  }
  _present_complete_semaphore = present_complete_semaphore;
  return swapchain_index;
}

void window_glfw::destroy_swapchain()
{
  _present_complete_semaphore = nullptr;
  _swapchain.reset();
  _device = nullptr;
}

std::uint32_t window_glfw::image_count() const
{
  BOOST_ASSERT(_swapchain);
  return _swapchain->image_count();
}

vk::shared_ptr<vk::layer1::image> window_glfw::image(
  std::uint32_t image_index) const
{
  BOOST_ASSERT(_swapchain);
  return _swapchain->image(image_index);
}

std::uint32_t window_glfw::acquire_next_image()
{
  using namespace std::chrono_literals;

  auto [result, swapchain_index, present_complete_semaphore] =
    _swapchain->acquire_next_image(
      std::chrono::duration_cast<std::chrono::nanoseconds>(1s).count(),
      vk::layer1::fence::null_handle);
  switch (result)
  {
  case vk::result::success:
    _present_complete_semaphore = present_complete_semaphore;
    break;

  case vk::result::timeout:
    // Cannot happen as we wait for an infinite amount of time.
    std::cout << "acquire_next_image: timeout" << std::endl;
    BOOST_ASSERT(false);
    break;

  case vk::result::not_ready:
    std::cout << "acquire_next_image: not_ready" << std::endl;
    break;

  case vk::result::suboptimal_khr:
    std::cout << "acquire_next_image: suboptimal_khr" << std::endl;
    // swapchain is not as optimal as it could be, but the platform's
    // presentation engine will still present the image correctly.
    break;

  case vk::result::error_out_of_date_khr:
    std::cout << "acquire_next_image: error_out_of_date_khr" << std::endl;
    // swapchain is out of date (possibly because the window was resized) and
    // must be recreated.
    on_resize(size());
    break;

  case vk::result::error_device_lost:
    std::cout << "acquire_next_image: error_device_lost" << std::endl;
    /// ToDo: What do do now?
    BOOST_ASSERT(false);
    break;

  case vk::result::error_out_of_host_memory:
    std::cout << "acquire_next_image: error_out_of_host_memory" << std::endl;
    /// ToDo: What do do now?
    BOOST_ASSERT(false);
    break;

  case vk::result::error_out_of_device_memory:
    std::cout << "acquire_next_image: error_out_of_device_memory" << std::endl;
    /// ToDo: What do do now?
    BOOST_ASSERT(false);
    break;

  default:
    std::cout << "acquire_next_image: unknown error" << std::endl;
    // Unexpected error.
    BOOST_ASSERT(false);
  }
  return swapchain_index;
}

bool window_glfw::present(uint32_t wait_semaphore_count,
                          std::uint64_t* wait_semaphores,
                          std::uint32_t current_buffer)
{
  BOOST_ASSERT(_device);

  const auto swapchains = core::make_array(_swapchain->handle());

  switch (_device->graphics_queue()->present(vk::present_info_khr(
    /* next */ nullptr,
    /* wait_semaphore_count */ wait_semaphore_count,
    /* wait_semaphores */ reinterpret_cast<const VkSemaphore*>(wait_semaphores),
    /* swapchain_count */ swapchains.size(),
    /* swapchains */ swapchains.data(),
    /* image_indices */ &current_buffer,
    /* results */ nullptr)))
  {
  case vk::result::success:
    return true;

  case vk::result::suboptimal_khr:
    std::cout << "present: suboptimal" << std::endl;
    // demo->swapchain is not as optimal as it could be, but the platform's
    // presentation engine will still present the image correctly.
    return true;

  case vk::result::error_out_of_date_khr:
    std::cout << "present: error_out_of_date_khr" << std::endl;
    // swapchain is out of date (possibly because the window was resized)
    // and must be recreated:
    on_resize(size());
    return false;

  default:
    BOOST_ASSERT(false);
    return false;
  }
}

void window_glfw::visible(bool visible)
{
  if (_visible == visible)
    return;
  _visible = visible;
  if (!_handle)
    return;
  if (_visible)
    glfwShowWindow(static_cast<GLFWwindow*>(_handle));
  else
    glfwHideWindow(static_cast<GLFWwindow*>(_handle));
}

math::vector2<std::int32_t> window_glfw::position() const
{
  if (_handle)
  {
    math::vector2<std::int32_t> result;
    glfwGetWindowPos(static_cast<GLFWwindow*>(_handle), &result.x, &result.y);
    return result;
  }
  else
    return _initial_position;
}

void window_glfw::position(math::vector2<std::int32_t> position)
{
  /// ToDo: Check whether this function is called from the main thread.
  if (_handle)
  {
    glfwSetWindowPos(static_cast<GLFWwindow*>(_handle), position.x, position.y);
  }
  else
    _initial_position = position;
}

math::vector2<std::uint32_t> window_glfw::size() const
{
  if (_handle)
  {
    int width;
    int height;
    glfwGetWindowSize(static_cast<GLFWwindow*>(_handle), &width, &height);
    if (width < 0)
      width = 0;
    if (height < 0)
      height = 0;
    return {static_cast<std::uint32_t>(width),
            static_cast<std::uint32_t>(height)};
  }
  else
    return _initial_size;
}

void window_glfw::size(math::vector2<std::uint32_t> size)
{
  /// ToDo: Check whether this function is called from the main thread.
  if (_handle)
    glfwSetWindowSize(static_cast<GLFWwindow*>(_handle),
                      static_cast<int>(size.x), static_cast<int>(size.y));
}

std::string window_glfw::title() const
{
  return _title;
}

void window_glfw::title(std::string title)
{
  _title = title;
  if (_handle)
    glfwSetWindowTitle(static_cast<GLFWwindow*>(_handle), _title.c_str());
}

void* window_glfw::native_handle() const
{
#if defined(SHIFT_PLATFORM_WINDOWS)
  return glfwGetWin32Window(static_cast<GLFWwindow*>(_handle));
#elif defined(SHIFT_PLATFORM_LINUX)
#if defined(USE_WAYLAND)
  return glfwGetWaylandWindow(static_cast<GLFWwindow*>(_handle));
#else
  return reinterpret_cast<void*>(
    glfwGetX11Window(static_cast<GLFWwindow*>(_handle)));
#endif
#endif
}

math::vector2<std::uint32_t> window_glfw::last_size() const
{
  return _last_size;
}
}
