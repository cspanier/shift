#ifndef SHIFT_RENDER_VK_WINDOW_H
#define SHIFT_RENDER_VK_WINDOW_H

#include <cstdint>
#include <memory>
#include <string>
#include <shift/core/boost_disable_warnings.h>
#include <boost/signals2.hpp>
#include <shift/core/boost_restore_warnings.h>
#include <shift/core/types.h>
#include <shift/core/bit_field.h>
#include <shift/math/vector.h>
#include "shift/render/vk/render_target.h"

namespace shift::render::vk::layer1
{
class device;
}

namespace shift::render::vk
{
enum class window_flag
{
  none = 0x00,
  visible = 0x01,
  decorated = 0x02,
  resizeable = 0x04,
  transparent = 0x08
};

using window_flags = core::bit_field<window_flag>;

inline window_flags operator|(window_flag lhs, window_flag rhs)
{
  return window_flags{lhs} | rhs;
}

///
class window : public render_target
{
public:
  /// Default constructor.
  window() = default;

  /// Constructor.
  window(math::vector2<std::int32_t> initial_position,
         math::vector2<std::uint32_t> initial_size, vk::window_flags flags);

  window(const window&) = delete;
  window(window&&) = delete;

  /// Destructor.
  virtual ~window();

  window& operator=(const window&) = delete;
  window& operator=(window&&) = delete;

  /// Returns the platform specific window handle.
  void* handle() const
  {
    return _handle;
  }

  /// Returns whether the window is resizeable.
  bool resizeable() const
  {
    return _resizeable;
  }

  /// Returns whether the window is decorated.
  bool decorated() const
  {
    return _decorated;
  }

  /// Returns whether the window is transparent.
  bool transparent() const
  {
    return _transparent;
  }

  /// Returns whether the window is visible.
  bool visible() const
  {
    return _visible;
  }

  /// Shows or hides the window.
  /// @remarks
  ///   By default a newly created window is hidden.
  /// @remarks
  ///   This has to be called from the thread which initialized the Renderer
  ///   instance.
  virtual void visible(bool visible);

  /// Returns the window's current position.
  virtual math::vector2<std::int32_t> position() const;

  /// Moves the window to the new position.
  /// @remarks
  ///   This has to be called from the thread which initialized the Renderer
  ///   instance.
  virtual void position(math::vector2<std::int32_t> position);

  /// Returns the window's current size.
  virtual math::vector2<std::uint32_t> size() const;

  /// Resizes the window to the new size.
  /// @remarks
  ///   This has to be called from the thread which initialized the Renderer
  ///   instance.
  virtual void size(math::vector2<std::uint32_t> size);

  /// Returns the window's current title.
  virtual std::string title() const;

  /// Sets a new window title.
  /// @remarks
  ///   This has to be called from the thread which initialized the Renderer
  ///   instance.
  virtual void title(std::string title);

public:
  ///
  boost::signals2::signal<void()> on_close;

  ///
  boost::signals2::signal<void(math::vector<2, std::int32_t> position)> on_move;

  /// Event handler which gets called each time the window is resized.
  /// @remarks
  ///   When a window gets minimized/hidden the size will be {0, 0}.
  boost::signals2::signal<void(math::vector<2, std::uint32_t> size)> on_resize;

  ///
  boost::signals2::signal<void(int key, int scancode, int action, int mods)>
    on_key;

  ///
  boost::signals2::signal<void(math::vector<2, float> position)> on_move_cursor;

  ///
  boost::signals2::signal<void(math::vector<2, float> offset)> on_scroll;

  ///
  boost::signals2::signal<void(int button, int action, int mods)>
    on_mouse_button;

protected:
  void* _handle = nullptr;
  math::vector2<std::int32_t> _initial_position;
  math::vector2<std::uint32_t> _initial_size;
  bool _visible = false;
  bool _decorated = false;
  bool _resizeable = false;
  bool _transparent = false;
  std::string _title;
};
}

#endif
