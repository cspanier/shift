#ifndef SHIFT_RENDER_VK_MESH_H
#define SHIFT_RENDER_VK_MESH_H

#include "shift/render/vk/types.h"
#include "shift/render/vk/shared_object.h"
#include "shift/render/vk/buffer.h"

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
