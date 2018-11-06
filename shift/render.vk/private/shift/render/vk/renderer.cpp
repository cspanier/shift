#include "shift/render/vk/renderer.hpp"
#include "shift/render/vk/renderer_impl.hpp"

namespace shift::render::vk
{
vk::renderer& renderer::create_renderer(std::uint32_t render_thread_count,
                                        const char* application_name,
                                        uint32_t application_version,
                                        vk::debug_layers debug_layers,
                                        vk::debug_logs debug_logs)
{
  return vk::renderer_impl::singleton_create(
    render_thread_count, application_name, application_version, debug_layers,
    debug_logs);
}

renderer::~renderer() noexcept = default;
}
