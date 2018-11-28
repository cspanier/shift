#ifndef SHIFT_RENDER_VK_MATERIAL_HPP
#define SHIFT_RENDER_VK_MATERIAL_HPP

#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/shared_object.hpp"
#include "shift/render/vk/texture.hpp"
#include "shift/render/vk/sampler.hpp"

namespace shift::resource_db
{
class material;
}

namespace shift::render::vk
{
enum class material_map_usage
{
  albedo,
  ambient_occlusion,
  normal,
  height,
  roughness,
  metalness
};

///
class material : public shared_object
{
public:
  material() = default;
  material(const material&) = delete;
  material(material&&) = delete;
  ~material() noexcept override = 0;
  material& operator=(const material&) = delete;
  material& operator=(material&&) = delete;

  /// @return
  ///   The texture of the specified map.
  virtual boost::intrusive_ptr<vk::texture> map_texture(
    material_map_usage usage) = 0;

  /// @return
  ///   The sampler of the specified map.
  virtual boost::intrusive_ptr<vk::sampler> map_sampler(
    material_map_usage usage) = 0;
};
}

#endif
