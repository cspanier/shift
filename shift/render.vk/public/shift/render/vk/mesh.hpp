#ifndef SHIFT_RENDER_VK_MESH_HPP
#define SHIFT_RENDER_VK_MESH_HPP

#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared_object.hpp"
#include "shift/render/vk/buffer.hpp"

namespace shift::resource
{
class mesh;
}

namespace shift::render::vk
{
///
class mesh : public shared_object
{
public:
  mesh() = default;
  mesh(const mesh&) = delete;
  mesh(mesh&&) = delete;
  ~mesh() noexcept override = 0;
  mesh& operator=(const mesh&) = delete;
  mesh& operator=(mesh&&) = delete;
};
}

#endif
