#include "shift/render/vk/window.h"
#include "shift/render/vk/renderer.h"

namespace shift::render::vk
{
window::window(math::vector2<std::int32_t> initial_position,
               math::vector2<std::uint32_t> initial_size,
               vk::window_flags flags)
: _initial_position(initial_position),
  _initial_size(initial_size),
  _visible(flags & vk::window_flag::visible),
  _decorated(flags & vk::window_flag::decorated),
  _resizeable((flags & vk::window_flag::resizeable) &&
              (flags & vk::window_flag::decorated)),
  _transparent((flags & vk::window_flag::transparent) &&
               !(flags & vk::window_flag::decorated))
{
}

window::~window() = default;

void window::visible(bool visible)
{
  _visible = visible;
}

math::vector2<std::int32_t> window::position() const
{
  return {0, 0};
}

void window::position(math::vector2<std::int32_t> /*position*/)
{
  // NOP.
}

math::vector2<std::uint32_t> window::size() const
{
  return {0u, 0u};
}

void window::size(math::vector2<std::uint32_t> /*size*/)
{
  // NOP.
}

std::string window::title() const
{
  return {};
}

void window::title(std::string /*title*/)
{
  // NOP.
}
}
