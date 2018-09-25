#ifndef SHIFT_RENDER_VK_MODEL_H
#define SHIFT_RENDER_VK_MODEL_H

#include "shift/render/vk/types.h"
#include "shift/render/vk/shared_object.h"
#include "shift/render/vk/mesh.h"
#include "shift/render/vk/material.h"

namespace shift::resource
{
class model;
}

namespace shift::render::vk
{
///
class model : public shared_object
{
public:
  model() = default;
  model(const model&) = delete;
  model(model&&) = delete;
  ~model() noexcept override = 0;
  model& operator=(const model&) = delete;
  model& operator=(model&&) = delete;

  ///
  virtual boost::intrusive_ptr<vk::mesh> mesh() = 0;

  ///
  virtual boost::intrusive_ptr<vk::material> material() = 0;
};
}

#endif
