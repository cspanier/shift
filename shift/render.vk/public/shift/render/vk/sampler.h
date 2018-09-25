#ifndef SHIFT_RENDER_VK_SAMPLER_H
#define SHIFT_RENDER_VK_SAMPLER_H

#include "shift/render/vk/types.h"
#include "shift/render/vk/shared.h"
#include "shift/render/vk/shared_object.h"
#include "shift/render/vk/texture.h"

namespace shift::resource
{
struct sampler;
}

namespace shift::render::vk
{
enum class sampler_map
{
  albedo,
  ambient_occlusion,
  normal,
  height,
  rouchess,
  metalness
};

struct sampler_address_modes_uvw
{
  vk::sampler_address_mode u = vk::sampler_address_mode::repeat;
  vk::sampler_address_mode v = vk::sampler_address_mode::repeat;
  vk::sampler_address_mode w = vk::sampler_address_mode::repeat;
};

///
class sampler : public shared_object
{
public:
  sampler() = default;
  sampler(const sampler&) = delete;
  sampler(sampler&&) = delete;
  ~sampler() noexcept override = 0;
  sampler& operator=(const sampler&) = delete;
  sampler& operator=(sampler&&) = delete;

  virtual sampler_address_modes_uvw address_modes() const = 0;
  virtual float max_anisotropy() = 0;
  virtual float min_lod() = 0;
  virtual float max_lod() = 0;
};
}

#endif
