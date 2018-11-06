#ifndef SHIFT_RENDER_VK_LAYER2_MODEL_HPP
#define SHIFT_RENDER_VK_LAYER2_MODEL_HPP

#include <memory>
#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/model.hpp"
#include "shift/render/vk/layer1/buffer.hpp"
#include "shift/render/vk/layer1/descriptor_set.hpp"
#include "shift/render/vk/layer2/object.hpp"
#include "shift/render/vk/layer2/mesh.hpp"
#include "shift/render/vk/layer2/material.hpp"

namespace shift::resource
{
class model;
}

namespace shift::render::vk::layer2
{
///
class model final : public vk::model, public child_object, public parent_object
{
public:
  ///
  model(vk::layer1::device& device, boost::intrusive_ptr<vk::layer2::mesh> mesh,
        boost::intrusive_ptr<vk::layer2::material> material);

  model(const model&) = delete;
  model(model&&) = delete;
  ~model() noexcept final;
  model& operator=(const model&) = delete;
  model& operator=(model&&) = delete;

  ///
  boost::intrusive_ptr<vk::mesh> mesh() final
  {
    return static_cast<vk::mesh*>(_mesh.get());
  }

  ///
  boost::intrusive_ptr<vk::material> material() final
  {
    return static_cast<vk::material*>(_material.get());
  }

  /// @see parent_object::signal.
  void signal(availability_state signal) final;

private:
  ///
  void update_resources();

  vk::layer1::device* _device;
  boost::intrusive_ptr<vk::layer2::mesh> _mesh;
  boost::intrusive_ptr<vk::layer2::material> _material;

  vk::shared_ptr<vk::layer1::buffer> _instance_buffer;
  std::array<vk::shared_ptr<vk::layer1::descriptor_set>, config::frame_count>
    _descriptor_sets;
};
}

#endif
