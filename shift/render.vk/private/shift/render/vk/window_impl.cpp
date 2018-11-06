#include "shift/render/vk/window_impl.hpp"

namespace shift::render::vk
{
window_impl::window_impl(math::vector2<std::int32_t> initial_position,
                         math::vector2<std::uint32_t> initial_size,
                         vk::window_flags flags)
: vk::window(initial_position, initial_size, std::move(flags))
{
}

window_impl::~window_impl() = default;
}
