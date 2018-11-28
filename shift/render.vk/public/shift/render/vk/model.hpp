#ifndef SHIFT_RENDER_VK_MODEL_HPP
#define SHIFT_RENDER_VK_MODEL_HPP

#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared_object.hpp"
#include "shift/render/vk/mesh.hpp"
#include "shift/render/vk/material.hpp"

namespace shift::resource_db
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
