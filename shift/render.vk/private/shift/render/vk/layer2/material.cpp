#include "shift/render/vk/layer2/material.h"
#include "shift/render/vk/layer2/texture.h"
#include "shift/render/vk/layer2/sampler.h"
#include "shift/render/vk/layer1/sampler.h"
#include "shift/render/vk/layer1/device.h"

#include <shift/log/log.h>

namespace shift::render::vk
{
material::~material() noexcept
{
}
}

namespace shift::render::vk::layer2
{
material::material(vk::layer1::device& device, material_maps maps)
: _device(&device), _maps(std::move(maps))
{
  for (auto& map : _maps)
  {
    if (map.texture != nullptr && map.texture->add_parent(*this))
      ++_unloaded_children;
  }
}

material::~material() noexcept
{
  for (auto& map : _maps)
  {
    if (map.texture != nullptr)
      map.texture->remove_parent(*this);
  }
}

boost::intrusive_ptr<vk::texture> material::map_texture(
  material_map_usage usage)
{
  return core::at(_maps, usage).texture;
}

boost::intrusive_ptr<vk::sampler> material::map_sampler(
  material_map_usage usage)
{
  return core::at(_maps, usage).sampler;
}

void material::signal(availability_state state)
{
  if (state == availability_state::unloaded)
  {
    if (++_unloaded_children == 1)
      availability(availability_state::unloaded);
  }
  else if (state == availability_state::loaded)
  {
    BOOST_ASSERT(_unloaded_children > 0);
    if (--_unloaded_children == 0)
      availability(availability_state::loaded);
  }
  else if (state == availability_state::updated)
  {
    if (_unloaded_children == 0)
      availability(availability_state::updated);
  }
  log::debug() << "Material #" << std::hex
               << reinterpret_cast<std::size_t>(this) << " now has " << std::dec
               << _unloaded_children << " unavailable dependencies left.";
}
}
