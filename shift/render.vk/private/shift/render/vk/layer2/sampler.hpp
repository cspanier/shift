#ifndef SHIFT_RENDER_VK_LAYER2_SAMPLER_HPP
#define SHIFT_RENDER_VK_LAYER2_SAMPLER_HPP

#include <memory>
#include "shift/render/vk/types.hpp"
#include "shift/render/vk/shared.hpp"
#include "shift/render/vk/smart_ptr.hpp"
#include "shift/render/vk/sampler.hpp"
#include "shift/render/vk/layer2/object.hpp"

namespace shift::resource
{
struct sampler;
}

namespace shift::render::vk::layer2
{
///
class sampler final : public vk::sampler, public child_object
{
public:
  sampler(vk::layer1::device& device, sampler_address_mode address_mode_u,
          sampler_address_mode address_mode_v,
          sampler_address_mode address_mode_w, float max_anisotropy,
          float min_lod, float max_lod);

  sampler(const sampler&) = delete;
  sampler(sampler&&) = delete;
  ~sampler() noexcept final;
  sampler& operator=(const sampler&) = delete;
  sampler& operator=(sampler&&) = delete;

  sampler_address_modes_uvw address_modes() const final;
  float max_anisotropy() final;
  float min_lod() final;
  float max_lod() final;

private:
  vk::shared_ptr<vk::layer1::sampler> _sampler;
  vk::sampler_address_modes_uvw _address_modes = {
    vk::sampler_address_mode::repeat, vk::sampler_address_mode::repeat,
    vk::sampler_address_mode::repeat};
  float _max_anisotropy = 1.0f;
  float _min_lod = 0.0f;
  float _max_lod = 14.0f;
};
}

#endif
