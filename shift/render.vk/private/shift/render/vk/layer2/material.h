#ifndef SHIFT_RENDER_VK_LAYER2_MATERIAL_H
#define SHIFT_RENDER_VK_LAYER2_MATERIAL_H

#include <memory>
#include "shift/render/vk/types.h"
#include "shift/render/vk/shared.h"
#include "shift/render/vk/smart_ptr.h"
#include "shift/render/vk/material.h"
#include "shift/render/vk/layer2/object.h"

namespace shift::resource
{
class material;
}

namespace shift::render::vk::layer2
{
struct material_map
{
  boost::intrusive_ptr<vk::layer2::texture> texture;
  boost::intrusive_ptr<vk::layer2::sampler> sampler;
};

using material_maps = std::array<material_map, 6>;

///
class material final : public vk::material,
                       public child_object,
                       public parent_object
{
public:
  ///
  material(vk::layer1::device& device, material_maps maps);

  material(const material&) = delete;
  material(material&&) = delete;
  ~material() noexcept final;
  material& operator=(const material&) = delete;
  material& operator=(material&&) = delete;

  /// @see vk::material::map_texture.
  boost::intrusive_ptr<vk::texture> map_texture(material_map_usage usage) final;

  /// @see vk::material::map_sampler.
  boost::intrusive_ptr<vk::sampler> map_sampler(material_map_usage usage) final;

  ///
  void signal(availability_state state) final;

private:
  vk::layer1::device* _device = nullptr;
  material_maps _maps;
};
}

#endif
